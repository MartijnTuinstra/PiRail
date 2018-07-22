#include "RNet.h"
#include "EEPROM.h"
#include "SPI.h" //Fast shiftOut and shiftIn

lnMsg *RxPacket;
lnMsg TxPacket;

void setup(){
  Serial.begin(9600);

  RailNet.init(7,5);

  memset(RxPacket->data,16,0);
  memset(TxPacket.data,16,0);

  Serial.println("Booted");
}

RN_STATUS RN_status;

void loop(){
  // Check for any received LocoNet packets
  if( RailNet.available()){
    RxPacket = RailNet.receive();

    Serial.println("Available");
    
    if(RxPacket != 0){
      uint8_t size = getRnMsgSize(RxPacket);
      for(int i = 0;i<size;i++){
        Serial.print(RxPacket->data[i]);
      }
    }
  }

  // Check for any received packets through Serial interface
  if(Serial.available() > 1){
    memset(TxPacket.data,16,0);
    TxPacket.data[0] = Serial.read();
    TxPacket.data[1] = Serial.read();
    uint8_t size = getRnMsgSize(&TxPacket) - 1; //No checksum included
    while(Serial.available() != (size - 2));
    for(int i = 2;i<size;i++){
      TxPacket.data[i] = Serial.read();
    }
    RailNet.send(&TxPacket);
  }
  
}
