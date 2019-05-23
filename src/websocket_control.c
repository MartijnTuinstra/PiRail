#include "system.h"
#include "mem.h"

#include "encryption.h"
#include "module.h"
#include "config.h"

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
    
    write(client->fd, response, strlen(response));

    client->type = protocol;

    _free(buf);
    _free(key);
    _free(_protocol);
    _free(response_key);

    //Successfull
    return 1;
  }
  else{
    loggerf(INFO, "It's not a HTML5-websocket\n");

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

int websocket_ping(int fd){
  loggerf(WARNING, "Websocket ping fd:%i", fd);
  char buf[10];
  buf[0] = WEBSOCKET_FIN | WEBSOCKET_PING;
  buf[1] = 0;

  if(write(fd, buf, 2) == -1){
    loggerf(WARNING, "socket write error %x", errno);
  };

  int i = 0;
  while(i < 5){
    memset(buf, 0, 10);

    int32_t recvlength = recv(fd, buf, 10, 0);

    if(recvlength <= 0){
      return 0;
    }

    print_hex(buf, recvlength);

    //Websocket opcode
    int opcode = buf[0] & 0b00001111;

    if(opcode == WEBSOCKET_PONG){
      return 1;
    }

    i++;
  }

  return 0;
}

void * websocket_client_connect(void * p){
  struct web_client_t * client = p;

  if(!websocket_client_check(client)){
    client->state = 2;
    close(client->fd);
    return 0;
  }

  //Set timeout
  struct timeval tv;
  tv.tv_sec = WEBSOCKET_CLIENT_TIMEOUT;
  tv.tv_usec = 0;
  setsockopt(client->fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

  //Reset Subscribed Trains
  client->trains[0].id = 0xFFF;
  client->trains[1].id = 0xFFF;

  char * buf = _calloc(WS_BUF_SIZE, char);
  int length = 0;

  _SYS->_Clients++;

  int counter = 0;

  char data[3];
  if(_SYS->Websocket_State == _SYS_Module_Init){
    while(1){
      // Require login
      data[0] = WSopc_Admin_Login;
      ws_send(client->fd, data, 1, 0xFF);

      usleep(100000);

      int status = websocket_get_msg(client->fd, buf, &length);

      if(status == 1){
        websocket_decode((uint8_t *)buf, client);
      }
      else if(status == -7){
        counter++;

        if(counter > 10){
          if(!websocket_ping(client->fd)){
            loggerf(INFO, "Client %i timeout", client->fd);
            close(client->fd);
            _SYS->_Clients--;
            client->state = 2;
            _free(buf);
            return 0;
          }
          else{
            counter = 0;
          }
        }
        continue;
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
      else{
        break;
      }
    }
    _free(buf);
  }

  // Send Enabled options
  data[0] = WSopc_Service_State;
  data[1] = _SYS->_STATE >> 8;
  data[2] = _SYS->_STATE & 0xFF;
  ws_send(client->fd, data, 3, 0xFF);

  //Send submodule status
  WS_stc_SubmoduleState();
  
  //Send track layout and data
  if(_SYS->_STATE & STATE_Modules_Loaded){
    // Send Track Layout Data
    for(int i = 0; i < unit_len; i++){
      if(!Units[i])
        continue;

      WS_Track_LayoutDataOnly(i, client->fd);
    }

    if(_SYS->_STATE & STATE_Modules_Coupled){
      WS_Track_Layout(client->fd);
      
      // Send new client JSON
      WS_NewClient_track_Switch_Update(client->fd);
    }
  }

  // Send open messages
  WS_send_open_Messages(client->fd);


  // Send broadcast flags
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

  memset(buf, 0, WS_BUF_SIZE);

  uint8_t timeout_counter = 0;

  while(1){
    // If threre is data recieved
    memset(buf, 0, WS_BUF_SIZE);
    int status = websocket_get_msg(client->fd, buf, &length);

    if(status == 1){
      websocket_decode((uint8_t *)buf, client);

      timeout_counter = 0;
    }
    else if(status == -7){
      timeout_counter++;
      loggerf(INFO, "%i Time out counter %i", client->id, timeout_counter);

      if(timeout_counter > 20){
        if(!websocket_ping(client->fd)){
          loggerf(INFO, "Client %i timed out", client->id);
          close(client->fd);
          _SYS->_Clients--;
          client->state = 2;
          _free(buf);
          return 0;
        }
        else{
          timeout_counter = 0;
        }
      }
    }
    else if(status == -8){
      loggerf(INFO, "Client %i disconnected", client->id);
      close(client->fd);
      _SYS->_Clients--;
      client->state = 2;
      _free(buf);
      return 0;
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
        loggerf(INFO, "Stopping websocket client %i thread", i);
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
    WS_password = _calloc(WS_pass_length + 2, char);
    if (WS_password && WS_pass_length == 32)
    {
      fread (WS_password, 1, WS_pass_length, f);
    }
    else{
      loggerf(ERROR, "PASSWORD: wrong length or unable to allocate memory");
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
      return;
    }
  }

  // Send failed code
  char response[100] = "HTTP/1.1 400 OK\r\n\r\n";
  write(fd, response, strlen(response));

  close(fd);
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

  //Set server timeout
  struct timeval tv;
  tv.tv_sec = WEBSOCKET_CLIENT_TIMEOUT;
  tv.tv_usec = 0;
  setsockopt(server, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

  loggerf(DEBUG, "Listening for Websocket Clients");
  while((_SYS->_STATE & STATE_RUN) == STATE_RUN){
    // Run until system is stopped, or until client_accept is closed

    fd_client = accept(server, (struct sockaddr *)&client_addr, &sin_len);

    if(fd_client == -1){
      if(errno == EAGAIN || errno == ETIMEDOUT){
        continue;
      }
      else if(errno == EINTR){
        break;
      }
      else{
        loggerf(WARNING, "Failed to connect with client %i", errno);
        continue;
      }
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
