#include "LocoNet/LocoNet.h"

void setup(){
  Serial.begin(115200);

  LocoNet.init(7);

  pinMode(5,OUTPUT);
  pinMode(10,OUTPUT );
  pinMode(11,OUTPUT );
  pinMode(13,OUTPUT );
}

lnMsg *RxPacket;
LN_STATUS LN_status;

void loop(){
  // Check for any received LocoNet packets
  if( LocoNet.available() )
  {
    Serial.println("Recieved A Packet");
    RxPacket = LocoNet.receive() ;
    Serial.print("RX: ");
    uint8_t msgLen = getLnMsgSize(RxPacket);
    for (uint8_t x = 0; x < msgLen; x++)
    {
      uint8_t val = RxPacket->data[x];
      // Print a leading 0 if less than 16 to make 2 HEX digits
      if (val < 16)
        Serial.print('0');

      Serial.print(val, HEX);
      Serial.print(' ');
    }
    Serial.println();
  }
  delay(1000);
  if(digitalRead(11) == HIGH){
    LN_status = LocoNet.send( OPC_LOCO_SPD, 0xF0, 0xC2) ;
    if(LN_status == LN_COLLISION){
      Serial.println("Collision");
    }else if(LN_status == LN_UNKNOWN_ERROR){
      Serial.println("Unknown Error");
    }else if(LN_status == LN_NETWORK_BUSY){
      Serial.println("Network Busy");
    }else if(LN_status == LN_CD_BACKOFF || LN_status == LN_PRIO_BACKOFF){
      Serial.println("Backoff");
    }else if(LN_status == LN_RETRY_ERROR){
      Serial.println("Retry Error");
    }else{
      Serial.println("\tSuccesfull Send??");
    }
    delay(1000);
    digitalWrite(13,LOW);
  }
}

