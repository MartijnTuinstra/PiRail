//Websocket opcodes
#include "Web.h"

/**//*

0 = EMPTY
1 = EMERGENY STOP
2 =               release
3 = Electrical Short
4 =                  release
5 = Train has split
6 =                 release
2-10 =  or HIGH PRIO commands
11 = NEW TRAIN [Couple id]
12 =                       release

/**/

#define ACTIVATE 0
#define RELEASE  1

struct WS_Message {
  char type;
};

struct WS_Message MessageList[32];
char MessageCounter = 0;

void WS_init_Message_List(){
  memset(MessageList,0,64);
  MessageCounter = 0;
}

char WS_add_Message(char type){
  if(MessageCounter >= 32){
    MessageCounter = 0;
  }
  while((MessageList[MessageCounter].type & 0x80) != 0){
    MessageCounter++;
    printf("Busy Message %i, Skip index %02X\n",MessageCounter,MessageList[MessageCounter].type);
    if(MessageCounter >= 32){
      MessageCounter = 0;
    }
  }
  MessageList[MessageCounter].type = type + 0x80;
  return MessageCounter++;
}

void WS_clear_message(char ID){
  MessageList[ID].type = 0;

  send_all((char []){WSopc_ClearMessage,ID},2,1);
}

void WS_EmergencyStop(){
  send_all((char []){WSopc_EmergencyStop},1,0xFF); //Everyone
}

void WS_ShortCircuit(){
  send_all((char []){WSopc_ShortCircuitStop},1,0xFF); //Everyone
}

void WS_EmergencyRelease(){
  send_all((char []){WSopc_ClearEmergency},1,0xFF); //Everyone
}
/*
void Web_Emergency_Stop(int i){
  char data[5];
  data[0] = 1;
  if(i == ACTIVATE){
    data[1] = 1;
  }else if(i == RELEASE){
    data[1] = 2;
  }else{
    return;
  }
  printf("Emergency_Stop (%i):[%i][%i]",i,data[0],data[1]);

  send_all(data,2,1);
}

void Web_Electrical_Stop(int i){
  char data[5];
  data[0] = 1;
  if(i == ACTIVATE){
    data[1] = 3;
  }else if(i == RELEASE){
    data[1] = 4;
  }else{
    return;
  }
  send_all(data,2,1);
}
*/
void Web_Train_Split(int i,char tID,char B[]){
  printf("\n\nWeb_Train_Split(%i,%i,{%i,%i});\n\n",i,tID,B[0],B[1]);
  char data[8];
  data[0] = 1;
  if(i == ACTIVATE){
    data[1] = 5;
    data[2] = tID;
    data[3] = B[0];
    data[4] = B[1];
    send_all(data,5,1);
  }else if(i == RELEASE){
    data[1] = 6;
    data[2] = tID;
    send_all(data,3,1);
  }else{
    return;
  }
}

void WS_NewTrain(char nr,char M,char B){
  //Nr:   follow id of train
  //M,B:  module nr and block nr

  char data[5];
  data[0] = WSopc_NewMessage;
  data[1] = (WS_add_Message(0) & 0x1F) + 0; //type = 0
  data[2] = M;
  data[3] = B;
  data[4] = nr;
  send_all(data,5,1);
}
/*
void Web_Link_Train(int type,char nr,char B[]){
  //Type: type of message ACTIVATE or RELEASE
  //Nr:   follow id of train
  //B:    a two byte array containing the module nr and block nr
  char data[8];
  if(type == ACTIVATE){
    data[0] = WSopc_NewMessage;
    data[1] = (WS_add_Message(0) & 0x1F) + 0;
    data[2] = B[0];
    data[3] = B[1];
    data[4] = nr;
    send_all(data,5,1);
  }else if(type == RELEASE){
    data[0] = WSopc_ClearMessage;
    data[1] = (WS_add_Message(0) & 0x1F) + 0;
    data[1] = 12;
    data[2] = nr;
    data[3] = B[0];
    data[4] = B[1];
    data[5] = B[2];
    send_all(data,6,1);
  }else{
    return;
  }
}*/

void Web_Train_Data(char data[14]){
  printf("\n\nWeb_Train_Data;\n\n");
  char s_data[20];
  s_data[0] = 7;

  for(int i = 0;i<7;i++){
    s_data[i+1] = data[i];
  }

  send_all(s_data,8,1);
}
