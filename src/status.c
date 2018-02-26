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
  uint16_t type;
  char data[16];
  char data_length;
};

struct WS_Message MessageList[0x1FFF];
char MessageCounter = 0;

void WS_init_Message_List(){
  memset(MessageList,0,64);
  MessageCounter = 0;
}

char WS_init_Message(char type){
  if(MessageCounter >= 0x1FFF){
    MessageCounter = 0;
  }
  while((MessageList[MessageCounter].type & 0x8000) != 0){
    MessageCounter++;
    printf("Busy Message %i, Skip index %02X\n",MessageCounter,MessageList[MessageCounter].type);
    if(MessageCounter >= 0x1FFF){
      MessageCounter = 0;
    }
  }
  MessageList[MessageCounter].type = type + 0x8000;
  return MessageCounter++;
}

void WS_add_Message(uint16_t ID, char length,char data[16]){
  memcpy(MessageList[ID].data,data,length);
  MessageList[ID].data_length = length;
}

void WS_send_open_Messages(int Client_fd){
  for(int i = 0;i<=0x1FFF;i++){
    if(MessageList[i].type & 0x8000){
      send_packet(Client_fd,MessageList[i].data,MessageList[i].data_length,0xFF);
    }
  }
}

void WS_clear_message(uint16_t ID){
  MessageList[ID].type = 0;

  printf("Send clear message (ID: %i)\n",ID);

  send_all((char [3]){WSopc_ClearMessage,(ID >> 8),ID&0xFF},3,0xFF);
}







void WS_EmergencyStop(){
  send_all((char []){WSopc_EmergencyStop},1,0xFF); //Everyone
}

void WS_ShortCircuit(){
  send_all((char []){WSopc_ShortCircuitStop},1,0xFF); //Everyone
}

void WS_ClearEmergency(){
  send_all((char []){WSopc_ClearEmergency},1,0xFF); //Everyone
}





void WS_trackUpdate(int Client_fd){
  pthread_mutex_lock(&mutex_lockB);
  char data[4096];

  data[0] = WSopc_BroadTrack; //Opcode

  int data_len;
  _Bool content = 0;

  int q = 1;

  for(int i = 0;i<MAX_Modules;i++){
    if(Units[i]){
      for(int j = 0;j<=Units[i]->B_L;j++){
        struct Seg * B = Units[i]->B[j];
        if(B && B->change){
          content = 1;

          data[(q-1)*4+1] = B->Module;
          data[(q-1)*4+2] = B->id;
          data[(q-1)*4+3] = (B->dir << 7) + (B->blocked << 4) + B->state;
          data[(q-1)*4+4] = B->train;
          q++;

          B->change = 0;
        }
      }
    }
  }

  data_len = (q-1)*4+1;

  if(content == 1){
    if(Client_fd){
      send_packet(Client_fd,data,data_len,WS_Flag_Track);
    }else{
      send_all(data,data_len,WS_Flag_Track);
    }
  }
  pthread_mutex_unlock(&mutex_lockB);
}

void WS_SwitchesUpdate(int Client_fd){
  pthread_mutex_lock(&mutex_lockB);
  char buf[4096];
  /*Switches*/

    buf[0] = WSopc_BroadSwitch;
    int buf_l  = 0;
    _Bool content   = 0;

    int q = 1;
    //printf("\n\n3");

    for(int i = 0;i<MAX_Modules;i++){
      if(Units[i]){
        for(int j = 0;j<=Units[i]->S_L;j++){
          struct Swi * S = Units[i]->S[j];
          if(S){
            if(Client_fd == 0 && (S->state & 0x80) != 0x80){
              continue;
            }
            content = 1;
            buf[(q-1)*3+1] = S->Module;
            buf[(q-1)*3+2] = S->id & 0x7F;
            buf[(q-1)*3+3] = S->state & 0x7F;
           // printf(",%i,%i,%i",S->Module,S->id,S->state);
            q++;
          }
        }
      }
    }

    buf_l = (q-1)*3+1;

  /*MSSwitches*/

    //buf[0] = 5;
    //buf_l = 0;

    q = 1;

    for(int i = 0;i<MAX_Modules;i++){
      if(Units[i]){
        content = 1;
        for(int j = 0;j<=Units[i]->Mod_nr;j++){
          struct Mod * M = Units[i]->M[j];
          if(M){
            buf[(q-1)*4+1+buf_l] = M->Module;
            buf[(q-1)*4+2+buf_l] = (M->id & 0x7F) + 0x80;
            buf[(q-1)*4+3+buf_l] = M->state;
            buf[(q-1)*4+4+buf_l] = M->length;
            q++;
          }
        }
      }
    }

  buf_l += (q-1)*4+1;

  if(content == 1){
    if(Client_fd){
      printf("WS_SwitchesUpdate Custom Client");
      send_packet(Client_fd,buf,buf_l,WS_Flag_Switches);
    }else{
      printf("WS_SwitchesUpdate ALL");
      send_all(buf,buf_l,WS_Flag_Switches);
    }
  }

  pthread_mutex_unlock(&mutex_lockB);
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
*/
void WS_NewTrain(char nr,char M,char B){
  //Nr:   follow id of train
  //M,B:  module nr and block nr
  uint16_t msg_ID = WS_init_Message(0);

  char data[6];
  data[0] = WSopc_NewMessage;
  data[1] = ((msg_ID >> 8) & 0x1F) + 0; //type = 0
  data[2] = (msg_ID & 0xFF);
  data[3] = nr;
  data[4] = M;
  data[5] = B;
  send_all(data,6,WS_Flag_Messages);
  WS_add_Message(msg_ID,6,data);
}

void WS_TrainSplit(char nr,char M1,char B1,char M2,char B2){
  //Nr:   follow id of train
  //M,B:  module nr and block nr
  uint16_t msg_ID = WS_init_Message(1);

  char data[8];
  data[0] = WSopc_NewMessage;
  data[1] = ((msg_ID >> 8) & 0x1F) + 0x20; //type = 1
  data[2] = (msg_ID & 0xFF);
  data[3] = nr;
  data[4] = M1;
  data[5] = B1;
  data[6] = M2;
  data[7] = B2;
  send_all(data,8,WS_Flag_Messages);
  WS_add_Message(msg_ID,8,data);
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

void WS_reset_switches(int client_fd){
  //Check if client has admin rights
  char admin = 1;
  if(admin){
    //Go through all switches
    for(int i = 0;i<MAX_Modules;i++){
      for(int j = 0;j<Units[i]->S_L;j++){
        if(Units[i]->S[j]){
          Units[i]->S[j]->state = Units[i]->S[j]->default_state + 0x80;
        }
      }
    }

    //Send all switch updates
    WS_SwitchesUpdate(0);
  }
}

void WS_LinkTrain(uint8_t fID, uint8_t tID){
  send_all((char []){WSopc_LinkTrain,fID,tID},3,0xFF);
}

void WS_TrainData(char data[14]){
  printf("\n\nWeb_Train_Data\n\n");
  char s_data[20];
  s_data[0] = WSopc_Z21TrainData;

  for(int i = 0;i<7;i++){
    s_data[i+2] = data[i];
  }

  s_data[1] = DCC_train[((s_data[2] << 8) + s_data[3])]->ID;

  send_all(s_data,9,WS_Flag_Trains);
}
/*
void Web_Train_Data(char data[14]){
  printf("\n\nWeb_Train_Data;\n\n");
  char s_data[20];
  s_data[0] = 7;

  for(int i = 0;i<7;i++){
    s_data[i+1] = data[i];
  }

  send_all(s_data,8,1);
}
*/