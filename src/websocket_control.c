#include "system.h"

#include "websocket_control.h"
#include "logger.h"

pthread_t websocket_clear_thread;
struct web_client_t * websocket_clients;
char * WS_password;

int websocket_client_check(struct web_client_t * C){
  loggerf(ERROR, "FIX websocket_connect");
}

void * websocket_client_connect(void * p){
  struct web_client_t * client = p;

  if(!websocket_client_check(client)){
    client->state = 2;
    close(client->fd);
    return 0;
  }

  char * buf = _calloc(1024, char);

  _SYS->_Clients++;

  char data[3];
  data[0] = WSopc_Service_State;
  data[1] = _SYS->_STATE >> 8;
  data[2] = _SYS->_STATE & 0xFF;
  send_packet(client->fd, data, 3, 0xFF);
  
  if(_SYS->_STATE & STATE_Modules_Loaded && _SYS->_STATE & STATE_Modules_Coupled){
    send_packet(client->fd,(char [6]){2,4,1,8,4,2},6,8);

    WS_Track_Layout();
    
    printf("Send new client JSON\n");
    WS_NewClient_track_Switch_Update(client->fd);
  }

  printf("Send open messages\n");
  WS_send_open_Messages(client->fd);


  printf("Send broadcast flags\n");
  send_packet(client->fd, (char [2]){WSopc_ChangeBroadcast,client->type}, 2, 0xFF);

  if(_SYS->_STATE & STATE_TRAIN_LOADED){
    loggerf(INFO, "Update clients libs %i", client->id);
    WS_EnginesLib(client->fd);
    WS_CarsLib(client->fd);
    WS_TrainsLib(client->fd);
  }

  printf("Done\n");

  //Set timeout
  struct timeval tv;
  tv.tv_sec = WEBSOCKET_CLIENT_TIMEOUT;
  tv.tv_usec = 0;
  setsockopt(client->fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

  memset(buf,0,1024);

  while(1){
    // If threre is data recieved
    if(recv(client->fd, buf, 1024, MSG_PEEK) > 0){
      printf("Data received\n");
      usleep(10000);
      int length = 0;
      memset(buf,0,1024);
      int status = websocket_get_msg(client->fd, buf, &length);

      printf("Status: %i\n", status);
      if(status == 1){
        recv_packet_procces(buf, client);
      }
      printf("\nData: %s\n", buf);
      if(status == -8){
        close(client->fd);
        _SYS->_Clients--;
        client->state = 2;
        return 0;
      }
    }

    if(client->state == 2){
      return 0;
    }

    if((_SYS->_STATE & STATE_RUN) == 0){
      loggerf(DEBUG, "Websocket stop client");
      close(client->fd);
      client->state = 2;
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
      }
    }

    // reduce cpu load
    usleep(1000000);
  }
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

      pthread_create(&websocket_threads[i], NULL, websocket_client_connect, (void *) &websocket_clients[i]);
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
  int fdimg;
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
    _SYS_change(STATE_RUN, 2);
    return 0;
  }

  if(listen(server, MAX_WEB_CLIENTS) == -1){
    loggerf(CRITICAL, "LISTEN ERROR");
    close(server);
    _SYS_change(STATE_RUN, 2);
    return 0;
  }

  //Start clear_clients
  pthread_create(&websocket_clear_thread, NULL, websocket_clear_clients, NULL);
  
  _SYS_change(STATE_WebSocket_FLAG, 0);

  loggerf(DEBUG, "Waiting for STATE_Client_Accept");
  while((_SYS->_STATE & STATE_Client_Accept) == 0){
    usleep(100000);
  }

  while((_SYS->_STATE & (STATE_RUN | STATE_Client_Accept)) == (STATE_RUN | STATE_Client_Accept)){
    // Run until system is stopped, or until client_accept is closed

    fd_client = accept(server, (struct sockaddr *)&client_addr, &sin_len);

    if(_SYS->_STATE & STATE_Client_Accept == 0){
      break;
    }

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

  _free(WS_password);
}