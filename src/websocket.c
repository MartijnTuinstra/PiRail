#define _BSD_SOURCE
// #define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/sha.h>
#include <errno.h>

#include "./../lib/system.h"

#include "./../lib/rail.h"
#include "./../lib/switch.h"
#include "./../lib/trains.h"

#include "./../lib/websocket.h"
#include "./../lib/status.h"
#include "./../lib/encryption.h"
#include "./../lib/modules.h"
#include "./../lib/Z21.h"

#define MAX_WEB_CLIENTS 10

char websocket_magic_string[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

pthread_t client_threads[MAX_WEB_CLIENTS];
pthread_mutex_t mutex_lock_web;

char * WS_password = 0;

struct client_thread_args clients_data[MAX_WEB_CLIENTS];
struct web_client_t * clients[MAX_WEB_CLIENTS];
int fd_client_list[MAX_WEB_CLIENTS] = {0};

char strcmp2(char arr1[],char arr2[], int length){
  for(int i = 0;i<length;i++){
    if(arr1[i] != arr2[i]){
      return 0;
    }
  }
  return 1;
}

void append_Array(char arr1[],int length1, char arr2[],int length2,char arrout[]){
  int index = 0;
  for(index;index<length1;index++){
    arrout[index] = arr1[index];
  }

  int pos = index;

  for(index;(index-pos)<length2;index++){
    arrout[index] = arr2[index-pos];
  }
}

void copy_Array(char arr1[],char arr2[],int length){
	for(int index = 0;index<length;index++){
		arr1[index] = arr2[index];
	}
}

int websocket_connect(struct web_client_t * C){
  int fd_client = C->fd_client;

  char buf[1024];

  memset(buf, 0, 1024);
  read(fd_client, buf, 1023);

  //printf("Got data in buffer:\n\n%s\n\n",buf);

  char Connection[20] = "Connection: Upgrade";
  char Connection2[35] = "Connection: keep-alive, Upgrade";
  char UpgradeType[20] = "Upgrade: websocket";
  char Key[25] = "Sec-WebSocket-Key: ";
  char Protocol[20] = "Protocol: ";

  char *start, *end, *S_prot, *E_prot, target[60], protocol[5];
  memset(target,0,60);
  memset(protocol,0,5);
  int prot;

  if((strstr(buf, Connection) || strstr(buf,Connection2)) &&
        strstr(buf, UpgradeType) && strstr(buf, Key)) {
    printf("\nIt is a HTML5 WebSocket!!\n");
    //Search for the Security Key
    start = strstr(buf, Key);
    start += strlen(Key);
    end = strstr(start,"\r\n");

    S_prot = strstr(buf, Protocol);
    if(S_prot){
      S_prot += strlen(Protocol);
      E_prot = strstr(S_prot,"\r\n");
    }

    if (end){
        strncat(target,start,end-start);
    }

    printf("Socket-key: '%s'\n",target);
    strcat(target,websocket_magic_string);

    printf("Searching protocol form 0x%x\n",buf);
    //S_prot = strstr(S_prot, " ") + 1;
    printf("S_prot: 0x%x\n",S_prot);
    if((S_prot-buf) != 0){
      printf("Drotocol: %s\n",S_prot);
      if (E_prot){
          strncat(protocol,S_prot,E_prot-S_prot);
      }
      prot = (int)strtol(protocol,NULL,10) & ~(0x10); //Deselect admin properties
    }else{
      printf("Default Protocol\n");
      prot = 0xEF;
      protocol[0] = '2';
      protocol[1] = '3';
      protocol[2] = '9';
      protocol[3] = 0;
    }
    printf("Protocol: %i\n",prot);

    //Create response Security key by concatenating a magic string and hash it with SHA1 + base64 encryption
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(target, sizeof(target), hash);
    char b64[40] = "";
    base64_encode(hash,sizeof(hash),b64,40);

    //Server response to the client
    char response[500] = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ";
    strcat(response,b64);
    if(prot != 0){
      strcat(response,"\r\nSec-WebSocket-Protocol: ");
      strcat(response,protocol);
    }
    strcat(response,"\r\n\r\n");
    printf("Sending Response\n\n%s\n\n\n",response);
    write(fd_client, response, strlen(response));

    printf("Done\n");
    C->client_type = prot;

    //Successfull
    return 1;
  }
  else{
    printf("It's not a HTML5-websocket\n");
    printf(strstr(buf,Connection));
    printf("\n");
    printf(strstr(buf,Connection2));
    printf("\n");
    printf(strstr(buf,UpgradeType));
    printf("\n");
    printf(strstr(buf,Key));
    printf("\n");
    //Unsuccessfull
    return 0;
  }
}

int recv_packet(int fd_client, char outbuf[], int * L){
  pthread_mutex_lock(&mutex_lock_web);
  char buf[1024];
  memset(buf,0,1024);
  memset(outbuf,0,sizeof(outbuf));
  usleep(10000);
  recv(fd_client,buf,1024,0);

  int byte = 0;
  /*
  while(1){
    if(buf[byte] == 0 && buf[byte+1] == 0 && buf[byte+2] == 0 && buf[byte+3] == 0){
      break;
    }
    printf("%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\n",buf[byte++],buf[byte++],buf[byte++],buf[byte++],buf[byte++],buf[byte++],buf[byte++],buf[byte++]);
  }

  byte = 0;*/
  //printf("FIN\topCode\tMask\tLength\tMask\n");
  //printf("%i\t",((buf[byte] & 0b10000000)>>7));
  int opcode = buf[byte++] & 0b00001111;
  //printf("%i\t",opcode);
  //printf("%i\t",((buf[byte] & 0b10000000)>>7));
  unsigned int mes_length = buf[byte++] & 0b01111111;
  if(mes_length == 126){
    mes_length = (buf[byte++] << 8) + buf[byte++];
    //printf("len %2x%2x\n",buf[byte++],buf[byte++]);
  }else if(mes_length == 127){
    //printf("len %2x%2x%2x%2x%2x%2x%2x%2x\n",buf[byte++],buf[byte++],buf[byte++],buf[byte++],buf[byte++],buf[byte++],buf[byte++],buf[byte++]);
  }
  //printf("%i\t",mes_length);
  int masking_index = byte;
  unsigned int masking_key = (buf[byte++] << 24) + (buf[byte++] << 16) + (buf[byte++] << 8) + (buf[byte++]);
  //printf("%u\n",masking_key);

  char output[mes_length+2];
  memset(output,0,mes_length+2);

  for(int i = 0;i<mes_length;0){
    unsigned int test;
    unsigned int text;
    test = (buf[byte++] << 24) + (buf[byte++] << 16) + (buf[byte++] << 8) + (buf[byte++]);
    text = test ^ masking_key;
      //printf("%c ",(text & 0xFF000000) >> 24);
      output[i++] = (text & 0xFF000000) >> 24;
      if(i<mes_length){
        //printf("%c ",(text & 0xFF0000) >> 16);
        output[i++] = (text & 0xFF0000) >> 16;
      }
      if(i<mes_length){
        //printf("%c ",(text & 0xFF00) >> 8);
        output[i++] = (text & 0xFF00) >> 8;
      }
      if(i<mes_length){
        //printf("%c ",(text & 0xFF));
        output[i++] = text & 0xFF;
      }
      //i+=4;
  }
  *L = mes_length;

  copy_Array(outbuf,output,mes_length);
  printf("WS recieved data: ");
  for(int q = 0;q<mes_length;q++){
    printf("%02x ",output[q]);
  }
  printf("\n",output);

  pthread_mutex_unlock(&mutex_lock_web);
  if(opcode == 8){
    printf("Connection closed by client\n\n");
    return -8;
  }
  return 1;
}

int send_packet(int fd_client, char data[],int length,int flag){
  //if(client_thread_args[])

  char m_length = 0;
  char outbuf[4096];
  char buf[10];
  memset(outbuf,0,4096);
  buf[0] = 0b10000000 + 0b00000010;
  if(length < 126){
    buf[1] = length;
    m_length = 2;
  }else if(length < 65535){
    buf[1] = 126;
    buf[2] = 0xFF;
    buf[3] = 0xFF;
    //printf("data length: %i\n",strlen(data));
    buf[2] = (length & 0xFF00) >> 8;
    buf[3] = length & 0xFF;
    m_length = 4;
  }else{
    printf("Too large message\n\n");
  }

  append_Array(buf,m_length,data,length,outbuf);

  //printf("Send: %s\n\n",data);

  write(fd_client,outbuf,m_length+length);
}

void send_all(char data[],int length,int flag){
  //Send the data packet with a length to everyone that has flag

  pthread_mutex_lock(&mutex_lock_web);
  int m_length = 0;
  char outbuf[4096];
  char buf[10];
  memset(outbuf,0,4096);
  buf[0] = 0b10000000 + 0b00000010;
  if(length < 126){
    buf[1] = length;
    m_length = 2;
  }else if(length < 65535){
    buf[1] = 126;
    buf[2] = 0xFF;
    buf[3] = 0xFF;
    //printf("data length: %i\n",strlen(data));
    buf[2] = (length & 0xFF00) >> 8;
    buf[3] = length & 0xFF;
    m_length = 4;
  }

  append_Array(buf,m_length,data,length,outbuf);

  for(int i = 0;i<MAX_WEB_CLIENTS;i++){
    if(fd_client_list[i] != 0 && (clients[i]->client_type & flag) != 0){
      printf("WS send (%i)\t",i);
      for(int zi = 0;zi<(length);zi++){printf("%02X ",data[zi]);};
      printf("\n");
      if(write(fd_client_list[i],outbuf,m_length+length) == -1){
        if(errno == EPIPE){
          printf("Broken Pipe!!!!!\n\n");
          close(fd_client_list[i]);
          _SYS->_Clients--;
          fd_client_list[i] = 0;
          clients[i]->state = 2;
        }
      }
    }
  }
  pthread_mutex_unlock(&mutex_lock_web);
}

int recv_packet_procces(char data[1024],struct client_thread_args * client_data){
  // Flag Admin Settings    0x80
  // Train stuff flag       0x40
  // Rail stuff flag        0x20
  // General Operation flag 0x10
  printf("recv_packet_procces\n");

  if(data[0] & 0x80){ //Admin settings
    printf("Admin settings: %02X\n",data[0]);
    if(data[0] == 0xFF){ //Admin login
      printf("\nAdmin Login\n\n");
      if(strcmp2(&data[1],WS_password,32) == 1){
        printf("\n\n\n\n\nSUCCESSFULL LOGIN\n\n");
        //0xc3,0xbf,0x35,0x66,0x34,0x64,0x63,0x63,0x33,0x62,0x35,0x61,0x61,0x37,0x36,0x35,0x64,0x36,0x31,0x64,0x38,0x33,0x32,0x37,0x64,0x65,0x62,0x38,0x38,0x32,0x63,0x66,0x39,0x39
        clients[client_data->thread_id]->client_type |= 0x10;

        send_packet(client_data->fd_client,(char [2]){WSopc_ChangeBroadcast,clients[client_data->thread_id]->client_type},2,255);
      }else{
        printf("\n\n\n\n\nFAILED LOGIN!!\n\n");
      }
    }
    if((clients[client_data->thread_id]->client_type & 0x10) != 0x10){
      //Client is not an admin
      printf("Not an Admin client");
      return 0;
    }
    

    if(data[0] == WSopc_Track_Scan){
      if(data[1] == 1){
        //Stop connecting
        _SYS_change(STATE_Modules_Coupled,1);
      }else if(data[1] == 2){
        //reload setup
        printf("\n\nReload setup not implemented\n\n");
      }
    }


    if(data[0] == 0x83){ //Reset switches to default

    }
    else if(data[0] == 0x84){ //Toggle Light Output

    }
    else if(data[0] == 0x88){ //All trains back to depot

    }

    else if(data[0] == 0xA0){ //Force switch

    }

    else if(data[0] == 0x90){ //Emergency stop, Admin authority / Disable track voltage

    }
    else if(data[0] == 0x91){ //Emergency stop, Admin authority / Enable track voltage

    }
  }
  else if(data[0] & 0x40){ //Train stuff
    if(data[0] == WSopc_AddNewTrain){ //New train

    }
    else if(data[0] == WSopc_LinkTrain){ //Link train
      uint8_t fID = data[1]; //follow TrainID
      uint8_t tID = data[2]; //TrainID

      printf("Linking train %i with dcc address #%i\n",fID,trains[tID]->DCC_ID);
      if(link_train(fID,tID)){
        WS_clear_message(((data[3] & 0x1F) >> 8)+data[4]);
        WS_LinkTrain(fID,tID);
        //Web_Link_Train(RELEASE,fID,(char []){tID, trains[tID]->DCC_ID >> 8,trains[tID]->DCC_ID & 0xFF});
        //WS_clear_message(msg_ID);
        Z21_GET_LOCO_INFO(trains[tID]->DCC_ID);
      }
    }
    else if(data[0] == WSopc_TrainSpeed){ //Train speed control
      char tID = data[1];
      char speed = data[2];
      trains[tID]->cur_speed = speed & 0x7F;
      trains[tID]->dir       = speed >> 7;

      Z21_GET_LOCO_INFO(trains[tID]->DCC_ID);
    }
    else if(data[0] == WSopc_TrainFunction){ //Train function control

    }
    else if(data[0] == WSopc_TrainOperation){ //Train operation change

    }
    else if(data[0] == WSopc_TrainAddRoute){ //Add route to train

    }
    else if(data[0] == WSopc_TrainClearRoute){ //Delete route

    }
  }
  else if(data[0] & 0x20){ //Track stuff
    if(data[0] == WSopc_SetSwitch){ //Toggle switch
      if(Units[data[1]] && Units[data[1]]->S[data[2]]){ //Check if switch exists
        printf("throw switch %i:%i to state: \t",data[1],data[2],data[3]);
        printf("%i->%i",Units[data[1]]->S[data[2]]->state, !Units[data[1]]->S[data[2]]->state);
        set_switch(Units[data[1]]->S[data[2]],data[3]);
      }
    }
    else if(data[0] == WSopc_SetSwitchReserved){ //Set switch reserved

    }
    else if(data[0] == WSopc_SetSwitchRoute){ //Set a route for switches

    }
  }
  else if(data[0] & 0x10){ // General Operation
    if(data[0] == WSopc_EmergencyStop){ //Enable Emergency Stop!!
      WS_EmergencyStop();
    }
    else if(data[0] == WSopc_ClearEmergency){ //Disable Emergency Stop!!
      WS_ClearEmergency();
    }
    else if(data[0] == WSopc_ClearMessage){

    }
    else if(data[0] == WSopc_ChangeBroadcast){
      clients[client_data->thread_id]->client_type; //current flags
      data[1]; //new flags
      if(data[1] & 0x10){ //Admin flag
        return 0; //Not allowed to set admin flag
        printf("Changing admin flag: NOT ALLOWED\n");
      }else if(data[1] != 0){
        clients[client_data->thread_id]->client_type = data[1];
        printf("Changing flags\n");
      }
      printf("Sending (new) flags\n");
      send_packet(client_data->fd_client,(char [2]){WSopc_ChangeBroadcast,clients[client_data->thread_id]->client_type},2,255);
    }
  }

  /*
  if(data[0] == 1){ //Re-calculate module

  }
  else if(data[0] == 2){ //New settings

  }
  else if(data[0] == 3){ //New train
    printf("Add a new train to the library\n");
    printf("%s\n",&data[1]);
    char data_out[40];
    char name[40];

    data_out[0] = 8; //Set opcode

    copy_Array(name,&data[1],strlen(&data[1]));

    int i = 2 + strlen(name);

    char type;

    printf("index %i = %i\n",i,data[i]);

    if(data[i] == 'P'){
      printf("Type\tPassenger train\n");
      type = 'P';
    }else if(data[i] == 'C'){
      printf("Type\tCargo train\n");
      type = 'C';
    }else{
      data_out[1] = 0; //Set status_code (0 == Failed)
      send_all(data_out,2,32);
      return;
    }
    i++;
    long DCC = (data[i++] << 8) + data[i++];
    long max_speed = (data[i++] << 8) + data[i++];

    printf("Name\t%s\nDCC\t%li\nSpeed\t%ld\n",name,DCC,max_speed);

    int value;
    if((value = create_train(DCC,max_speed,name,type)) >= 0){
      data_out[1] = 2; //Confirmed
      char l = 0;
      if(value < 255){//Train ID
        data_out[2] = value;
        l = 3;
      }else{
        data_out[2] = value & 0xFF;
        data_out[3] = value >> 8;
        l = 4;
      }
      /*
      data_out[3];//DCC HIGH byte
      data_out[4];//DCC LOW  byte

      sprintf(buf,"{\"RNTrain\" : [\"Confirmed\",[%i,\"%s\",%i,\"%c\",%i]]}",value,s_Name,atoi(s_DCC),data[3],atoi(s_Speed));
      printf("Send '%s'\n",buf);*//*
      send_all(data_out,l,32);
    }else{
      data_out[1] = 1; //Fail: DCC address already in use
      send_all(data_out,2,32);
    }
  }
  else if(data[0] == 30){ //Stop train
    printf("Stop Train %i\n",(data[1] << 8) + data[2]);
    train_stop(DCC_train[(data[1] << 8) + data[2]]);
  }
  else if(data[0] == 31){ //Train speed change
    printf("Train %i speed to %i\n",(data[1] << 8) + data[2],data[3]);
    train_set_speed(DCC_train[(data[1] << 8) + data[2]],data[3]);
  }
  else if(data[0] == 32){ //Train change direction
    printf("Train %i change direction (%i)\n",(data[1] << 8) + data[2],data[3]);
    train_set_dir(DCC_train[(data[1] << 8) + data[2]],data[3]);
  }
  else if(data[0] == 33){ //Train function press
    printf("Train %i function %i\n",(data[1] << 8) + data[2],data[3]);
    //train_set_dir(trains[(data[1] << 8 + data[2])],data[3]);
  }

  else if(data[0] == 34){ //Train set Route
    printf("Train %i Set route %i\n",(data[1] << 8) + data[2],data[3]);
    printf("%s\n",stations[data[3]]->Name);
    train_set_route(DCC_train[(data[1] << 8) + data[2]],stations[data[3]]);
    //train_set_dir(trains[(data[1] << 8 + data[2])],data[3]);
  }
  else if(data[0] == 35){ //Train delete Route
    printf("Train %i function %i\n",data[1],data[2]);
    //train_set_dir(trains[(data[1] << 8 + data[2])],data[3]);
  }

  else if(data[0] == 36){ //Train change control
    printf("Train %i function %i\n",data[1],data[2]);
    //train_set_dir(trains[(data[1] << 8 + data[2])],data[3]);
  }

  else if(data[0] == 40){ //Throw Switch
    if(Switch2[data[1]][data[2]]){
      printf("throw switch %i:%i\t",data[1],data[2]);
      printf("%i->%i",Switch2[data[1]][data[2]]->state, !Switch2[data[1]][data[2]]->state);
      throw_switch(Switch2[data[1]][data[2]]);
    }
  }
  else if(data[0] == 41){ //Throw MS_Switch
    //Test
  }
  else if(data[0] == 42){ //Switch something else
    //Test
  }

  else if(data[0] == 'S' || data[0] == 'M' || data[0] == 'm'){ //Throw switch
    char *M = strchr(&data[0], ':');
    char *B = strchr(&data[(M-data)+1], ':');
    if (M != NULL && B != NULL){ /* deal with error: / not present" */;
  /*
      int start = 1;
      int colom1 = M-data;
      int colom2 = B-data;
      int end = strlen(data);

      char s_M[5],s_B[5],s_S[5];

      memset(s_M,0,5);
      memset(s_B,0,5);
      memset(s_S,0,5);

      //printf("%i %i %i\n",colom1,colom2,end);

      for(int i = (start);i<=end;i++){
        if(i < colom1){
          s_M[(i-start)] = data[i];
        }else if(i > colom1 && i < colom2){
          s_B[(i-colom1-1)] = data[i];
        }else if(i > colom2 && i < end){
          s_S[(i-colom2-1)] = data[i];
        }
        if(i == colom1){
          s_M[(i-start)] = 0;
        }else if(i == colom2){
          s_B[(i-colom1-1)] = 0;
        }else if(i == (B-data)){
          s_S[(i-colom2-1)] = 0;
        }
        //printf("%i: %c\n",i,data[i]);
      }

      //printf("s_M %s\ts_B %s\ts_S %s\n",s_M,s_B,s_S);

      int s_m = atoi(s_M);
      int s_b = atoi(s_B);
      int s_s = atoi(s_S);

      if(data[0] == 'S'){
        if(Switch[s_m][s_b][s_s] != NULL){
          printf("throw switch %i:%i:%i\t",s_m,s_b,s_s);
          printf("%i->%i",Switch[s_m][s_b][s_s]->state, !Switch[s_m][s_b][s_s]->state);
          throw_switch(Switch[s_m][s_b][s_s]);
        }
      }
      else if(data[0] == 'M' || data[0] == 'm'){
        if(Moduls[s_m][s_b][s_s] != NULL){
          if(data[0] == 'M'){
            printf("throw ms switch + %i:%i:%i\t",s_m,s_b,s_s);
            printf("%i->%i",Moduls[s_m][s_b][s_s]->state, Moduls[s_m][s_b][s_s]->state+1);
            int q = Moduls[s_m][s_b][s_s]->state;
            q += 1;

            if(q >= Moduls[s_m][s_b][s_s]->length){
              q = 0;
            }
            throw_ms_switch(Moduls[s_m][s_b][s_s],q);
          }else if(data[0] == 'm'){
            printf("throw ms switch - %i:%i:%i\t",s_m,s_b,s_s);
            printf("%i->%i",Moduls[s_m][s_b][s_s]->state, Moduls[s_m][s_b][s_s]->state+1);
            int q = Moduls[s_m][s_b][s_s]->state;
            q -= 1;

            if(q < 0){
              q = Moduls[s_m][s_b][s_s]->length;
            }
            throw_ms_switch(Moduls[s_m][s_b][s_s],q);
          }
        }
      }
    }
  }
  else if(data[0] == 'L'){ //Linking train to follow_ID
    printf("%s\n",data);
    char *M = strchr(&data[0], ':');
    if (M != NULL){ /* deal with error: / not present" */;
  /*
      int start = 1;
      int colom1 = M-data;
      int end = strlen(data);

      char f_ID[5],t_ID[5]; //Follow ID, Train ID (strings)

      memset(f_ID,0,5);
      memset(t_ID,0,5);

      //printf("%i %i %i\n",colom1,colom2,end);

      for(int i = (start);i<=end;i++){
        if(i < colom1){
          f_ID[(i-start)] = data[i];
        }else if(i > colom1 && i < end){
          t_ID[(i-colom1-1)] = data[i];
        }
        if(i == colom1){
          f_ID[(i-start)] = 0;
        }else if(i == end){
          t_ID[(i-colom1-1)] = 0;
        }
      }

      //printf("s_M %s\ts_B %s\ts_S %s\n",s_M,s_B,s_S);

      int fID = atoi(f_ID);
      int tID = atoi(t_ID);

      printf("Linking train %i with dcc address #%i\n",fID,trains[tID]->DCC_ID);
      if(link_train(fID,tID)){
        Web_Link_Train(RELEASE,fID,(char []){tID, trains[tID]->DCC_ID >> 8,trains[tID]->DCC_ID & 0xFF});
        //WS_clear_message(msg_ID);
        Z21_GET_LOCO_INFO(trains[tID]->DCC_ID);
      }
    }
  }
  else if(data[0] == 'E' && (data[1] == 'r' || data[1] == 'c')){ //Clear Emergency stop or Electrical stop
    int start = 2;
    int end = strlen(data);

    char nr_s[4];

    for(int i = start;i<=end;i++){
      nr_s[(i-start)] = data[i];
      if(i == end){
        nr_s[(i-start)] = 0;
      }
      //printf("%i: %c\n",i,buf[i]);
    }

    int nr = atoi(nr_s);

    char buf[20];

    if(data[1] == 'r'){
      printf("Disable/Release Emergency Stop %i\n",nr);
      Web_Emergency_Stop(RELEASE);
    }else if(data[1] == 'c'){
      printf("Disable/Release Emergency Short Circuit Stop %i\n",nr);
      Web_Electrical_Stop(RELEASE);
    }
    status_st[nr] = 0;
  }
  else if(data[0] == 'R' && data[1] == 'N' && data[2] == 't'){ //Register a new train
    char data_out[40];

    data_out[0] = 8;

    printf("Add a new train to the library\n");
    if(data[3] == 'P'){
      printf("Type\tPassenger train\n");
    }else if(data[3] == 'C'){
      printf("Type\tCargo train\n");
    }else{
      data_out[1] = 0;
      send_all(data_out,2,32);
      return;
    }
    char *NAME = strchr(&data[0], ':');
    char *DCC = strchr(&data[(NAME-data)+1], ':');
    if (NAME != NULL && DCC != NULL){ /* deal with error: / not present" */;
  /*
      int start = 4;
      int colom1 = NAME-data;
      int colom2 = DCC-data;
      int end = strlen(data);

      char s_Name[21],s_DCC[5],s_Speed[5];

      memset(s_Name,0,21);
      memset(s_DCC,0,5);
      memset(s_Speed,0,5);

      for(int i = (start);i<=end;i++){
        if(i < colom1){
          s_Name[(i-start)] = data[i];
        }else if(i > colom1 && i < colom2){
          s_DCC[(i-colom1-1)] = data[i];
        }else if(i > colom2 && i < end){
          s_Speed[(i-colom2-1)] = data[i];
        }
        if(i == colom1){
          s_Name[(i-start)] = 0;
        }else if(i == colom2){
          s_DCC[(i-colom1-1)] = 0;
        }else if(i == (DCC-data)){
          s_Speed[(i-colom2-1)] = 0;
        }
      }

      printf("Name\t%s\nDCC\t%s\nSpeed\t%s\n",s_Name,s_DCC,s_Speed);
      int value;
      if((value = create_train(atoi(s_DCC),atoi(s_Speed),s_Name,data[3])) >= 0){
        data_out[1] = 2; //Confirmed
        data_out[2] = value;//Train ID
        /*
        data_out[3];//DCC HIGH byte
        data_out[4];//DCC LOW  byte

        sprintf(buf,"{\"RNTrain\" : [\"Confirmed\",[%i,\"%s\",%i,\"%c\",%i]]}",value,s_Name,atoi(s_DCC),data[3],atoi(s_Speed));
        printf("Send '%s'\n",buf);*//*
        send_all(data_out,3,32);
      }else{
        data_out[1] = 1; //Fail: DCC address already in use
        send_all(data_out,2,32);
      }
      //int s_b = atoi(s_B);
      //int s_s = atoi(s_S);
    }
  }
  else if(data[0] == 't' && (data[1] == 'D' || data[1] == 'A')){
    printf("Switching control type\t");
    char data_out[13];
    data_out[0] = '\0';
    if(data[1] == 'D'){
      printf("DCC\n");
      digital_track = TRUE;
      data_out[1] = 1;
    }else{
      printf("DC\n");
      digital_track = FALSE;
      data_out[1] = 0;
    }
    send_all(data_out,2,1);
  }
  else{
    printf("Wrong request for client\n");
  }*/
}

void * websocket_client(void * thread_data){
  struct client_thread_args *thread_args;
	thread_args = (struct client_thread_args *) thread_data;
	int i = thread_args->thread_id;
	int fd_client = thread_args->fd_client;

  printf("New websocket_client");
  if(websocket_connect(clients[i])){
      char buf[1024];
      memset(buf,0,1024);

      fd_client_list[i] = fd_client;

      _SYS->_Clients++;

      while((_SYS->_STATE & STATE_Modules_Loaded) == 0){

      }
      char data[3];
      data[0] = WSopc_Service_State;
      data[1] = _SYS->_STATE >> 8;
      data[2] = _SYS->_STATE & 0xFF;
      send_packet(fd_client,data,3,1);
      int length = 0;
      if(_SYS->_STATE & STATE_Modules_Loaded && _SYS->_STATE & STATE_Modules_Coupled){
        send_packet(fd_client,(char [6]){2,4,1,8,4,2},6,8);

        printf("Recv 1\n");
        // recv_packet(fd_client,buf,&length);
        memset(buf,0,1024);

        WS_Track_Layout();
        
        printf("Send new client JSON\n");

        WS_NewClient_track_Switch_Update(fd_client);
      }

      printf("Send open messages\n");
      WS_send_open_Messages(fd_client);


      printf("Send broadcast flags\n");

      send_packet(fd_client,(char [2]){WSopc_ChangeBroadcast,clients[i]->client_type},2,0xFF);

      printf("Done\n");

      while(1){
        // If threre is data recieved
        if(recv(fd_client,buf,1024,MSG_PEEK) > 0){
          printf("Data received\n");
          usleep(10000);
          length = 0;
		      memset(buf,0,1024);
          int status = recv_packet(fd_client,buf,&length);
		  /*for(int x = 0;x<length;x++){
			  printf("[%02x]",buf[x]);
		  }*/
          printf("Status: %i\n",status);
          if(status == 1){
            recv_packet_procces(buf,thread_args);
          }
          printf("\nData: %s\n",buf);
          if(status == -8){
            close(fd_client);
            _SYS->_Clients--;
            fd_client_list[i] = 0;
            clients[i]->state = 2;
            return 0;
          }

        }

        if(clients[i]->state == 2){
          return 0;
        }

        if((_SYS->_STATE & STATE_RUN) == 0){
          close(fd_client);
        }
      }
  }else{
    printf("Wrong HTTP request!!!!\n");
    close(fd_client);
    clients[i]->state = 2;
    return 0;
  }
}

void *clear_clients(){
	while(_SYS->_STATE & STATE_RUN){
		for(int i = 0;i<MAX_WEB_CLIENTS;i++){
			if(clients[i]->state == 2){
				pthread_join(client_threads[i], NULL);
				clients[i]->state = 0;
				printf("Reset client %i\n",i);
			}
		}
    usleep(100000);
	}
}

void * web_server(){
  struct sockaddr_in server_addr, client_addr;
  socklen_t sin_len = sizeof(client_addr);
  int fd_server, fd_client;
  int fdimg;
  int on = 1;
  //printf("Server starting...\n");

  long WS_pass_length;
  FILE * f = fopen ("./password.txt", "rb");

  if (f)
  {
    fseek (f, 0, SEEK_END);
    WS_pass_length = ftell (f);
    fseek (f, 0, SEEK_SET);
    WS_password = malloc (WS_pass_length);
    if (WS_password && WS_pass_length == 32)
    {
      fread (WS_password, 1, WS_pass_length, f);
    }
    else{
      printf("PASSWORD: wrong length or unable to allocate memory\n\n");
    }
    fclose (f);
  }


  fd_server = socket(AF_INET, SOCK_STREAM, 0);
  if(fd_server < 0){
    printf("ERROR, SOCKET\n");
    _SYS->_STATE = _SYS->_STATE & ~(STATE_RUN);
    return 0;
  }
  //fcntl(fd_server, F_SETFL, fcntl(fd_server, F_GETFL) | O_NONBLOCK);

  setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(9000);

  if(bind(fd_server, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
    printf("ERROR, BIND\n");
    close(fd_server);
    _SYS->_STATE = _SYS->_STATE & ~(STATE_RUN);
    return 0;
  }

  if(listen(fd_server, MAX_WEB_CLIENTS) == -1){
    printf("ERROR, LISTEN\n");
    close(fd_server);
    _SYS->_STATE = _SYS->_STATE & ~(STATE_RUN);
    return 0;
  }

  int q = 0;

  _SYS->_STATE |= STATE_WebSocket_FLAG;

  while((_SYS->_STATE & STATE_Client_Accept) == 0){
    usleep(10000);
  }

  //Create client structure data
  for(int i = 0;i<MAX_WEB_CLIENTS;i++){
    struct web_client_t * Z = (struct web_client_t *)malloc(sizeof(struct web_client_t));
    Z->state = 0;
    Z->fd_client = 0;
    Z->client_type = 0;
    clients[i] = Z;
  }

  //Thread for joining client thread when they finish
  pthread_t clear;
  pthread_create(&clear, NULL, clear_clients, NULL);

  while(1){
    fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &sin_len);

    if(fd_client == -1){
      if(q == 0){
        printf("ERROR, Client Connect\n");
      }
      q = 1;
      continue;
    }
    q = 0;

    int i = 0;
  	while(1){
  		if(clients[i]->state == 0){
  			clients_data[i].thread_id = i;
  			clients_data[i].fd_client = fd_client;

        clients[i]->fd_client = fd_client;
        clients[i]->state = 1;

  			printf("Create client thread %i\n",i);
  			pthread_create(&client_threads[i], NULL, websocket_client, (void *) &clients_data[i]);
  			break;
  		}
  		i++;
  		if(i==MAX_WEB_CLIENTS){
  			i = 0;
  			printf("Too many clients!!!!!\n\n");
  			usleep(100000);
  		}
  	}

    if((_SYS->_STATE & STATE_RUN) == 0){
      close(fd_server);
      _SYS->_STATE &= 0xFFFF ^ STATE_WebSocket_FLAG;

      // Memory Cleanup
      free(WS_password);

      break;
    }
  }
}
