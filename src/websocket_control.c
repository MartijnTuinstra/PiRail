#include "system.h"
#include "mem.h"

#include "encryption.h"

#include "websocket_control.h"
#include "logger.h"

char websocket_magic_string[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

pthread_t websocket_clear_thread;
struct web_client_t * websocket_clients;
char * WS_password;

int websocket_client_check(struct web_client_t * client){
  char * buf = _calloc(2048, char);

  read(client->fd, buf, 2040);

  char Connection[20] = "Connection: Upgrade";
  char Connection2[35] = "Connection: keep-alive, Upgrade";
  char UpgradeType[20] = "Upgrade: websocket";
  char Key[25] = "Sec-WebSocket-Key: ";
  char Protocol[20] = "Protocol: ";

  char *key_s, *key_e, *protocol_s, *protocol_e;
  char * key = _calloc(100, char);
  char * _protocol = _calloc(10, char);
  int protocol;

  loggerf(INFO, "Web Client (%d) check", client->fd);

  if((strstr(buf, Connection) || strstr(buf,Connection2)) &&
        strstr(buf, UpgradeType) && strstr(buf, Key)) {
    printf("It is a HTML5 WebSocket!!\n");

    //Search for the Security Key
    key_s = strstr(buf, Key) + strlen(Key);
    if(key_s){
      key_e = strstr(key_s,"\r\n");
      strncat(key, key_s, key_e - key_s);
    }

    // Append magic string
    strcat(key, websocket_magic_string);


    //Search for the Security Key
    protocol_s = strstr(buf, Protocol);
    // Check if protocol exists
    if(protocol_s){
      protocol_s += strlen(Protocol);
      protocol_e = strstr(protocol_s,"\r\n");
      strncat(_protocol, protocol_s, protocol_e - protocol_s);
      protocol = (int)strtol(_protocol, NULL, 10) & ~(0x10); //Deselect admin properties
    }
    else{
      protocol = 0xEF;
      strcpy(_protocol, "239");
    }
    // printf("Protocol: %s => %d\n", _protocol, protocol);

    //Create response Security key by hashing it with SHA1 + base64 encryption
    char hash[SHA_DIGEST_LENGTH];
    SHA1((unsigned char *)key, strlen(key), (unsigned char *)hash);
    char * response_key = _calloc(40, char);
    base64_encode((unsigned char *)hash, sizeof(hash), response_key, 40);

    //Server response to the client
    char response[500] = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ";
    strcat(response, response_key);

    strcat(response,"\r\nSec-WebSocket-Protocol: ");
    strcat(response, _protocol);

    strcat(response,"\r\n\r\n");
    // printf("Sending Response\n\n%s\n\n\n",response);
    write(client->fd, response, strlen(response));

    // printf("Done\n");
    client->type = protocol;

    _free(buf);
    _free(key);
    _free(_protocol);
    _free(response_key);

    //Successfull
    return 1;
  }
  else{
    printf("It's not a HTML5-websocket\n");
    //Server response to the client
    char response[100] = "HTTP/1.1 400 OK\r\n\r\n";
    write(client->fd, response, strlen(response));

    _free(buf);
    _free(key);
    _free(_protocol);

    //Unsuccessfull
    return 0;
  }
}

void * websocket_client_connect(void * p){
  struct web_client_t * client = p;

  if(!websocket_client_check(client)){
    client->state = 2;
    close(client->fd);
    return 0;
  }

  char * buf = _calloc(WS_BUF_SIZE, char);
  int length = 0;

  _SYS->_Clients++;

  char data[3];
  if(_SYS->Websocket_State == _SYS_Module_Init){
    // Require login
    data[0] = WSopc_Admin_Login;
    ws_send(client->fd, data, 1, 0xFF);

    int status = websocket_get_msg(client->fd, buf, &length);

    if(status == 1){
      websocket_decode((uint8_t *)buf, client);
    }
    else if(status == -8){
      loggerf(INFO, "Client %i disconnected", client->fd);
      close(client->fd);
      _SYS->_Clients--;
      client->state = 2;
      _free(buf);
      return 0;
    }

    if((client->type & 0x10) == 0){
      loggerf(ERROR, "Client not authenticated");
      close(client->fd);
      _SYS->_Clients--;
      client->state = 2;
      _free(buf);
      return 0;
    }
  }

  // Send Enabled options
  data[0] = WSopc_Service_State;
  data[1] = _SYS->_STATE >> 8;
  data[2] = _SYS->_STATE & 0xFF;
  ws_send(client->fd, data, 3, 0xFF);

  WS_stc_SubmoduleState();
  
  if(_SYS->_STATE & STATE_Modules_Loaded && _SYS->_STATE & STATE_Modules_Coupled){
    ws_send(client->fd,(char [6]){2,4,1,8,4,2},6,8);

    WS_Track_Layout();
    
    printf("Send new client JSON\n");
    WS_NewClient_track_Switch_Update(client->fd);
  }

  printf("Send open messages\n");
  WS_send_open_Messages(client->fd);


  printf("Send broadcast flags\n");
  ws_send(client->fd, (char [2]){WSopc_ChangeBroadcast,client->type}, 2, 0xFF);

  if(_SYS->_STATE & STATE_TRAIN_LOADED){
    loggerf(INFO, "Update clients libs %i", client->id);
    WS_EnginesLib(client->fd);
    WS_CarsLib(client->fd);
    WS_TrainsLib(client->fd);
    WS_stc_TrainCategories(client->fd);
  }

  if(_SYS->Z21_State & _SYS_Module_Run){
    WS_stc_Z21_info(client->fd);
  }
  WS_stc_Z21_IP(client->fd);

  printf("Done\n");

  //Set timeout
  struct timeval tv;
  tv.tv_sec = WEBSOCKET_CLIENT_TIMEOUT;
  tv.tv_usec = 0;
  setsockopt(client->fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

  memset(buf, 0, WS_BUF_SIZE);

  while(1){
    // If threre is data recieved
    if(recv(client->fd, buf, WS_BUF_SIZE, MSG_PEEK) > 0){
      memset(buf, 0, WS_BUF_SIZE);
      int status = websocket_get_msg(client->fd, buf, &length);

      if(status == 1){
        websocket_decode((uint8_t *)buf, client);
      }
      else if(status == -8){
        loggerf(INFO, "Client %i disconnected", client->id);
        close(client->fd);
        _SYS->_Clients--;
        client->state = 2;
        _free(buf);
        return 0;
      }
    }

    if(client->state == 2){
      _SYS->_Clients--;
      close(client->fd);
      _free(buf);
      return 0;
    }

    if((_SYS->_STATE & STATE_RUN) == 0){
      loggerf(DEBUG, "Websocket stop client");
      close(client->fd);
      _SYS->_Clients--;
      client->state = 2;
      _free(buf);
      return 0;
    }
  }

  _free(buf);

  loggerf(ERROR, "FIX websocket_client");
}

void * websocket_clear_clients(){
  while (_SYS->_STATE & STATE_RUN){
    for(int i = 0; i < MAX_WEB_CLIENTS; i++){
      if(websocket_clients[i].state == 2){
        loggerf(INFO, "Stopping websocket client thread");
        pthread_join(websocket_clients[i].thread, NULL);
        websocket_clients[i].fd = 0;
        websocket_clients[i].state = 0;
      }
    }

    // reduce cpu load
    usleep(1000000);
  }

  return 0;
}

void read_password(){
  long WS_pass_length;
  FILE * f = fopen ("./password.txt", "rb");

  if (f)
  {
    fseek (f, 0, SEEK_END);
    WS_pass_length = ftell (f);
    fseek (f, 0, SEEK_SET);
    WS_password = _calloc(WS_pass_length, char);
    if (WS_password && WS_pass_length == 32)
    {
      fread (WS_password, 1, WS_pass_length, f);
    }
    else{
      printf("PASSWORD: wrong length or unable to allocate memory\n\n");
    }
    fclose (f);
  }
}

void new_websocket_client(int fd){
  //Find a web_client_t
  for(int i = 0; i <= MAX_WEB_CLIENTS; i++){
    if(i == MAX_WEB_CLIENTS){
      loggerf(ERROR, "MAX WEB CLIENTS");
    }

    if(websocket_clients[i].state == 0){
      websocket_clients[i].id = i;
      websocket_clients[i].fd = fd;
      websocket_clients[i].state = 1;

      pthread_create(&websocket_clients[i].thread, NULL, websocket_client_connect, (void *) &websocket_clients[i]);
      break;
    }
  }
}

void * websocket_server(){
  loggerf(INFO, "Starting websocket server at port %i", WEBSOCKET_PORT);

  //Memory alloc for clients list and thread data
  websocket_clients = _calloc(MAX_WEB_CLIENTS, struct web_client_t);

  struct sockaddr_in server_addr, client_addr;

  socklen_t sin_len = sizeof(client_addr);

  int server, fd_client;
  int on = 1;

  read_password();

  server = socket(AF_INET, SOCK_STREAM, 0);
  if(server < 0){
    loggerf(CRITICAL, "SOCKET ERROR");
    _SYS_change(STATE_RUN, 2);
    return 0;
  }

  setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(WEBSOCKET_PORT);

  if(bind(server, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
    loggerf(CRITICAL, "BIND ERROR");
    close(server);
    _free(websocket_clients);
    _SYS_change(STATE_RUN, 2);
    return 0;
  }

  if(listen(server, MAX_WEB_CLIENTS) == -1){
    loggerf(CRITICAL, "LISTEN ERROR");
    close(server);
    _free(websocket_clients);
    _SYS_change(STATE_RUN, 2);
    return 0;
  }

  //Start clear_clients
  pthread_create(&websocket_clear_thread, NULL, websocket_clear_clients, NULL);

  WS_init_Message_List();
  
  _SYS->Websocket_State = _SYS_Module_Init;
  _SYS_change(STATE_WebSocket_FLAG, 0);

  loggerf(DEBUG, "Listening for Websocket Clients");
  while((_SYS->_STATE & STATE_RUN) == STATE_RUN){
    // Run until system is stopped, or until client_accept is closed

    fd_client = accept(server, (struct sockaddr *)&client_addr, &sin_len);

    if(fd_client == -1){
      loggerf(WARNING, "Failed to connect with client");
      continue;
    }

    loggerf(INFO, "New socket client");
    new_websocket_client(fd_client);
  }

  close(server);

  loggerf(INFO, "Stopping websocket_clear_clients");
  pthread_join(websocket_clear_thread, NULL);

  _SYS_change(STATE_WebSocket_FLAG, 2);
  _SYS->Websocket_State = _SYS_Module_Stop;

  _free(websocket_clients);
  _free(WS_password);

  return 0;
}
