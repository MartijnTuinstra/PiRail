#include "LocoNet/LocoNet.h"
#include "EEPROM.h"
#include "SPI.h" //Fast shiftOut and shiftIn


lnMsg *RxPacket;
lnMsg TxPacket;

uint8_t DevID;
uint8_t InputRegisters,OutputRegisters;
char * InputRegs, *NInputRegs, * OutputRegs, * BlinkMask, *PulseMask;
char activatePulse;
char blink_EN=0;

int latchPinOut = 9;
int dataLoad = A1, latchPinIn = A0;

#define PULSE_WIDTH_USEC   5

//Timers
unsigned long prev_step = 0;
unsigned long prev_blink = 0;
unsigned long activatePulse_prev = 0;
unsigned long blink_interval, pulse_interval, step_interval;

void setup(){
  Serial.begin(115200);

  /*First Boot*/
  //if(EEPROM.read(0) == 1){
    EEPROM.write(0,0);

    //EEPROM.write(1,32); //Address 32

    EEPROM.write(2,1); //1 Input  Registers
    EEPROM.write(3,1); //1 Output Register

    EEPROM.write(4,70); //Blink interval scaler
    EEPROM.write(5,20); //Pulse_interval
    EEPROM.write(6,1000); //Step_interval
  //}
  /********/

  SPI.begin();

  DevID = EEPROM.read(1);

  InputRegisters  = EEPROM.read(2);
  OutputRegisters = EEPROM.read(3);

  blink_interval = EEPROM.read(4) * 10;
  pulse_interval  = EEPROM.read(5) * 10;
  step_interval  = EEPROM.read(6) * 10;

  InputRegs  = (uint8_t *)malloc(InputRegisters);
  NInputRegs = (uint8_t *)malloc(InputRegisters);
  OutputRegs = (uint8_t *)malloc(OutputRegisters);
  BlinkMask  = (uint8_t *)malloc(OutputRegisters);
  PulseMask  = (uint8_t *)malloc(OutputRegisters);

  pinMode(5,OUTPUT);
  digitalWrite(5,LOW);

  LocoNet.init(7);

  memset(RxPacket->data,16,0);
  memset(TxPacket.data,16,0);


  for(int i = 0;i<InputRegisters;i++){
    InputRegs[i]  = 0;
    NInputRegs[i] = 0;
  }
  for(int i = 0;i<OutputRegisters;i++){
    OutputRegs[i]  = 0;
    BlinkMask[i] = 0;
    PulseMask[i] = 0;
  }

  pinMode(10,OUTPUT );
  pinMode(13,OUTPUT );

  pinMode(latchPinOut,OUTPUT ); // Latch pin
  pinMode(latchPinIn,OUTPUT ); // Latch pin
  pinMode(dataLoad  ,OUTPUT ); // Data load pin

  //Reset inputs and outputs
  SPI.transfer(OutputRegs,OutputRegisters);

  for(int i = 0;i<InputRegisters;i++){
    InputRegs[i] = SPI.transfer(0);
  }


  //wait and Report Own ID
  delay(2*DevID);
  TxPacket.data[0] = 0x80;
  TxPacket.data[1] = DevID;
  LocoNet.send(&TxPacket);
  Serial.print("My address is: 0x");
  Serial.println(DevID,HEX);
  delay(200-2*DevID);

  //Reset Outputs and inputs to zero
  //for(int i = 0;i<InputRegisters;i++){InputRegs[i] = 0;}
  //for(int i = 0;i<OutputRegisters;i++){OutputRegs[i] = 0;}
  unsigned long t = millis();
  while(t < blink_interval || t < pulse_interval || t < step_interval){
    delay(10);
    t = millis();
  }
}

LN_STATUS RN_status;

void loop(){
  // Check for any received LocoNet packets
  if( LocoNet.available() && (RxPacket = LocoNet.receive()) != 0)
  {
    if(proccess_packet(RxPacket)){
      digitalWrite(latchPinOut, LOW);
      delayMicroseconds(5);
      SPI.transfer(OutputRegs,OutputRegisters); //Shift out data
      delayMicroseconds(5);
      digitalWrite(latchPinOut, HIGH);
    }
  }
  
  //Executes on a interval of 'blink_interval'
  if(step()){
    digitalWrite(dataLoad,LOW);           //Pulse Read Line
    delayMicroseconds(PULSE_WIDTH_USEC);
    digitalWrite(dataLoad,HIGH);
    delayMicroseconds(PULSE_WIDTH_USEC);
    digitalWrite(latchPinIn, LOW);        //Start Shift in
    delay(1);
    uint8_t diff = 0;
    uint16_t INaddresses[10];
    char addres_counter;

    for(int i = 0;i<InputRegisters;i++){
      NInputRegs[i] = SPI.transfer(0);
      if((diff = NInputRegs[i] ^ InputRegs[i])){
        debug("difference",1);

        //Determine which bits are set
        if(diff & 1)  {INaddresses[addres_counter++] = i*8+0;}
        if(diff & 2)  {INaddresses[addres_counter++] = i*8+1;}
        if(diff & 4)  {INaddresses[addres_counter++] = i*8+2;}
        if(diff & 8)  {INaddresses[addres_counter++] = i*8+3;}
        if(diff & 16) {INaddresses[addres_counter++] = i*8+4;}
        if(diff & 32) {INaddresses[addres_counter++] = i*8+5;}
        if(diff & 64) {INaddresses[addres_counter++] = i*8+6;}
        if(diff & 128){INaddresses[addres_counter++] = i*8+7;}
      }
    }
    digitalWrite(latchPinIn, HIGH);
    Serial.println(addres_counter);

    //Create Message for master
    if(addres_counter != 0){
      if(addres_counter == 1){
        //POST SINGLE ADDRESS
        debug("POST SINGLE ADDRESS",1);

        Serial.println(INaddresses[0],HEX);
        Serial.println();
      }else if((addres_counter*2) <= InputRegisters){
        //POST MULTIPLE
        debug("POST MULTIPLE ADDRESSES",1);

        for(int i = 0;i<addres_counter;i++){
          Serial.println(INaddresses[i],HEX);
        }
        Serial.println();
      }else{
        //POST ALL
        debug("POST ALL ADDRESSES",1);
      }
      for(int i = 0;i<InputRegisters;i++){
        InputRegs[i] = NInputRegs[i];
      }
    }
    
    //printRegs();
    Serial.println("Step");
    if(DevID != 32){
      TxPacket.data[0] = 0x91;
      TxPacket.data[1] = 0x20;
      TxPacket.data[2] = 0x00;
      TxPacket.data[3] = 0x00;
      LocoNet.send(&TxPacket);
    }
  }
  //Executes on a interval of 'blink_interval'
  
  if(blink()){
    Serial.println("Blink!");
    for(int i = 0;i<OutputRegisters;i++){
      //XOR each output bit with Blink mask
      *(OutputRegs+i) ^= *(BlinkMask+i);
    }
    digitalWrite(latchPinOut, LOW);
    SPI.transfer(OutputRegs,OutputRegisters);
    digitalWrite(latchPinOut, HIGH);
  }
  //Executes on a interval of 'pulse_interval'
  if(pulse()){
    Serial.println("Pulse!");
    for(int i = 0;i<OutputRegisters;i++){
      //XOR each output bit with Blink mask
      *(OutputRegs+i) ^= *(PulseMask+i);
      
      //Reset Pulse mask
      *(PulseMask+i) = 0;
    }
    digitalWrite(latchPinOut, LOW);
    SPI.transfer(OutputRegs,OutputRegisters);
    digitalWrite(latchPinOut, HIGH);
  }
}

int step(){
  if(prev_step - step_interval < 0){
    return 0;
  }else if(prev_step < millis()-step_interval){
    prev_step = millis();
    return 1;
  }else{
    return 0;
  }
}

int blink(){
  if(blink_EN && prev_blink - blink_interval < 0){
    return 0;
  }else if(prev_blink < millis()-blink_interval){
    prev_blink = millis();
    return 1;
  }else{
    return 0;
  }
}

int pulse(){
  if(activatePulse == 1 && activatePulse_prev < millis()-pulse_interval){
    activatePulse = 0;
    return 1;
  }else{
    return 0;
  }
}

void printBits(byte myByte){
 for(byte mask = 0x80; mask; mask >>= 1){
   if(mask  & myByte)
       Serial.print('1');
   else
       Serial.print('0');
 }
 Serial.print(" ");
}

void printRegs(){
  Serial.print("Out:   ");
  for(int i = 0;i<OutputRegisters;i++){
    printBits(OutputRegs[i]);
  }
  Serial.println();
  Serial.print("Blink: ");
  for(int i = 0;i<OutputRegisters;i++){
    printBits(BlinkMask[i]);
  }
  Serial.println();
  Serial.print("Pulse: ");
  for(int i = 0;i<OutputRegisters;i++){
    printBits(PulseMask[i]);
  }
  Serial.println();Serial.println();
  Serial.print("In:    ");
  for(int i = 0;i<InputRegisters;i++){
    printBits(InputRegs[i]);
  }
  Serial.println();
  Serial.print("NIn:   ");
  for(int i = 0;i<InputRegisters;i++){
    printBits(NInputRegs[i]);
  }
  Serial.println();Serial.println();Serial.println();
}

char proccess_packet(lnMsg * RxPacket){
  char Opcode = RxPacket->data[0] ^ 0x80;

  Serial.println("Proccess_packet");
  for(int i = 0;i<getRnMsgSize(RxPacket);i++){
    Serial.print(RxPacket->data[i],HEX);
    Serial.print(" ");
  }
  Serial.println();

  if(Opcode == 0x10){//Toggle Single address
    Serial.println("Toggle Single Address");
    //Check DevID
    if(RxPacket->data[1] == DevID){
      Serial.println("ME");
      uint16_t address = RxPacket->data[2] + (RxPacket->data[3] << 7);
      uint8_t byte = address / 8;
      uint8_t offset = address % 8;
      OutputRegs[byte] ^= (1 << offset);
      return 1;
    }
  }else if(Opcode == 0x11){//Pulse Single address
    Serial.println("Pulse Single Address");
    //Check DevID
    if(RxPacket->data[1] == DevID){
      Serial.println("ME");
      uint16_t address = RxPacket->data[2] + (RxPacket->data[3] << 7);
      uint8_t byte = address / 8;
      uint8_t offset = address % 8;
      OutputRegs[byte] ^= (1 << offset);
      PulseMask[byte] ^= (1 << offset);
      activatePulse = 1;
      activatePulse_prev = millis();
      return 1;
    }
  }else if(Opcode == 0x12){//Toggle Blink Single address
    Serial.println("Toggle Blink Single Address");
    //Check DevID
    if(RxPacket->data[1] == DevID){
      Serial.println("ME");
      uint16_t address = RxPacket->data[2] + (RxPacket->data[3] << 7);
      uint8_t byte = address / 8;
      uint8_t offset = address % 8;
      if((OutputRegs[byte] & (1 << offset)) != 0){ //Reset
        OutputRegs[byte] ^= (1 << offset);
      }
      BlinkMask[byte] ^= (1 << offset);
      uint8_t test = 0;
      for(int i = 0;i<OutputRegisters;i++){
        test ^= BlinkMask[i];
      }
      if(test == 0){
        blink_EN = 0;
      }else{
        blink_EN = 1;
      }
      return 1;
    }
  }else if(Opcode == 0x13){ // Toggle multiple addresses
    debug("Toggle Multiple Addresses",1);
    if(RxPacket->data[2] == DevID){
      Serial.println("ME");
      char nr = (RxPacket->data[1] - 1);
      for(int i = 3;i<nr;i+=2){
        uint16_t address = RxPacket->data[i] + (RxPacket->data[i+1] << 7);
        uint8_t byte = address / 8;
        uint8_t offset = address % 8;
        OutputRegs[byte] ^= (1 << offset);
      }
      return 1;
    }
  }/*else if(Opcode == 0x14){ // Set all addresses
    debug("Set all Addresses",1);
    if(RxPacket->data[2] == DevID){
      Serial.println("ME");
      char nr = (RxPacket->data[1] - 1);
      for(int i = 3;i<nr;i++){
        OutputRegs[byte] = RxPacket->data[i];
      }
      return 1;
    }
  }else if(Opcode == 0x14){ // Set blink mask
    debug("Set blink mask",1);
    if(RxPacket->data[2] == DevID){
      Serial.println("ME");
      char nr = (RxPacket->data[1] - 1);
      for(int i = 3;i<nr;i++){
        BlinkMask[byte] = RxPacket->data[i];
      }
      return 1;
    }
  }else if(Opcode == 0x47){ //Request Read output
    debug("Request Read All output");
    if(RxPacket->data[1] == DevID){
      lnMsg TxPacket;
      TxPacket.data[0] = 0x96;
      TxPacket.data[1] = OutputRegisters+4;
      for(int i = 0;i<OutputRegisters;i++){
        TxPacket.data[i+2] = OutputRegs[i];
      }
      LocoNet.send(&TxPacket);
    }
  }else if(Opcode == 0x4C){ //Request Read input
    debug("Request Read All output");
    if(RxPacket->data[1] == DevID){
      //shift In
      lnMsg TxPacket;
      TxPacket.data[0] = 0x96;
      TxPacket.data[1] = InputRegisters+4;
      for(int i = 0;i<InputRegisters;i++){
        TxPacket.data[i+2] = InputRegs[i];
      }
      LocoNet.send(&TxPacket);
    }
  }*/
  //SETUP
  else if(Opcode == 0x50){ //New DevID
    if(RxPacket->data[1] == DevID){
      DevID = RxPacket->data[2];
      EEPROM.write(1,DevID);
      lnMsg TxPacket;
      TxPacket.data[0] = 0x7F;
      TxPacket.data[1] = DevID;
      LocoNet.send(&TxPacket);
    }
  }else if(Opcode == 0x51){ //input and output regs
    if(RxPacket->data[1] == DevID){
      InputRegisters = RxPacket->data[2];
      OutputRegisters = RxPacket->data[3];
      EEPROM.write(2,InputRegisters);
      EEPROM.write(3,OutputRegisters);
      InputRegs  = (uint8_t *)realloc(InputRegs,InputRegisters);
      OutputRegs = (uint8_t *)realloc(InputRegs,OutputRegisters);
      BlinkMask  = (uint8_t *)realloc(BlinkMask,OutputRegisters);
      PulseMask  = (uint8_t *)realloc(PulseMask,OutputRegisters);
      
      lnMsg TxPacket;
      TxPacket.data[0] = 0x7F;
      TxPacket.data[1] = DevID;
      LocoNet.send(&TxPacket);
    }
  }else if(Opcode == 0x52){ //blink interval
    if(RxPacket->data[1] == DevID){
      blink_interval = RxPacket->data[2];
      EEPROM.write(4,blink_interval);
      
      lnMsg TxPacket;
      TxPacket.data[0] = 0x7F;
      TxPacket.data[1] = DevID;
      LocoNet.send(&TxPacket);
    }
  }else if(Opcode == 0x53){ //pulse interval
    if(RxPacket->data[1] == DevID){
      pulse_interval = RxPacket->data[2] * 10;
      EEPROM.write(5,RxPacket->data[2]);
      
      lnMsg TxPacket;
      TxPacket.data[0] = 0x7F;
      TxPacket.data[1] = DevID;
      LocoNet.send(&TxPacket);
    }
  }else if(Opcode == 0x54){ //pulse interval
    if(RxPacket->data[1] == DevID){
      pulse_interval = RxPacket->data[2] * 10;
      EEPROM.write(5,RxPacket->data[2]);
      
      lnMsg TxPacket;
      TxPacket.data[0] = 0x7F;
      TxPacket.data[1] = DevID;
      LocoNet.send(&TxPacket);
    }
  }else if(Opcode == 0x59){ //Request EEPROM
    if(RxPacket->data[1] == DevID){
      lnMsg Tx;
      Tx.data[0] = 0x54;
      Tx.data[1] = 14;
      Tx.data[2] = DevID;
      for(int i = 0;i<10;i++){
        Tx.data[i+3] = EEPROM.read(i);
      }
      LocoNet.send(&Tx);
    }
  }else{
    Serial.println("Unknown Opcode");
  }
  return 0;
}

void debug(char string[],char new_line){
  if(new_line == 0){
    Serial.print(string);
  }else{
    Serial.println(string);
  }
}