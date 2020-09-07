#include <sys/socket.h>

#include "websocket/client.h"
#include "websocket/message.h"


// #include "websocket.h"

#include "encryption.h"
#include "logger.h"
#include "mem.h"
#include "system.h"

void print_hex(char * data, int size);

namespace Websocket {

Client::Client(Websocket::Server * Server, int fd){
  // Find an empty web_client_t
  // Start a new thread for this client
  this->Server = Server;
  this->fd = fd;

  if(!this->websocket_check()){
    char response[100] = "HTTP/1.1 400 OK\r\n\r\n";
    write(this->fd, response, strlen(response));
    close(this->fd);
    this->connected = false;
    return;
  }


  //Set timeout
  struct timeval tv;
  tv.tv_sec = WEBSOCKET_CLIENT_TIMEOUT;
  tv.tv_usec = 0;
  setsockopt(this->fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

  //Reset Subscribed Trains
  this->subscribedTrains[0] = 0xFF;
  this->subscribedTrains[1] = 0xFF;

  char * buf = (char *)_calloc(WS_BUF_SIZE, char);
  int length = 0;

  // Send first connect data
  // Return 0 if client is not authenticated properly
  if(!this->first_connect(buf, &length)){
    close(this->fd);
    this->connected = false;
    _free(buf);
    return;
  }
  _free(buf);

  this->Server->newClientCallback(this);

  this->thread_started = true;
  pthread_create(&this->thread, NULL, (void* (*)(void*))&this->run, (void *) this);


}

Client::~Client(){
  loggerf(INFO, "Client Destructor");
}

void Client::disconnect(){
  loggerf(DEBUG, "Disconnect Client %i", this->fd);
  close(this->fd);
  this->connected = false;
}

void * Client::run(Client * context){
  uint8_t timeout_counter = 0;
  char * buf = (char *)_calloc(WS_BUF_SIZE, char);
  int length = 0;

  while(1){
    // If threre is data recieved
    memset(buf, 0, WS_BUF_SIZE);
    int status = MessageGet(context->fd, buf, &length);

    if(status == 1){
      Parse((uint8_t *)buf, context);

      timeout_counter = 0;
    }
    else if(status == -7){
      timeout_counter++;

      if(timeout_counter > 20){
        if(!context->ping()){
          context->disconnect();
          _free(buf);
          return 0;
        }
        else{
          timeout_counter = 0;
        }
      }
    }
    else if(status == -8){
      context->disconnect();
      _free(buf);
      return 0;
    }

    // Client in closing state
    if(!context->connected || SYS->stop){
      context->disconnect();
      _free(buf);
      return 0;
    }
  }

  _free(buf);
  context->disconnect();
}


int Client::websocket_check(){
  char * buf = (char *)_calloc(2048, char);

  read(this->fd, buf, 2040);

  char Connection[20] = "Connection: Upgrade";
  char Connection2[35] = "Connection: keep-alive, Upgrade";
  char UpgradeType[20] = "Upgrade: websocket";
  char Key[25] = "Sec-WebSocket-Key: ";
  char Protocol[20] = "Protocol: ";

  char *key_s, *key_e, *protocol_s, *protocol_e;
  char * key = (char *)_calloc(100, char);
  char * _protocol = (char *)_calloc(10, char);
  int protocol;

  loggerf(INFO, "Web Client (%d) check", this->fd);

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
    char * response_key = (char *)_calloc(40, char);
    base64_encode((unsigned char *)hash, sizeof(hash), response_key, 40);

    //Server response to the client
    char response[500] = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ";
    strcat(response, response_key);

    strcat(response,"\r\nSec-WebSocket-Protocol: ");
    strcat(response, _protocol);

    strcat(response,"\r\n\r\n");
    
    write(this->fd, response, strlen(response));

    this->type = protocol;

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
    write(this->fd, response, strlen(response));

    _free(buf);
    _free(key);
    _free(_protocol);

    //Unsuccessfull
    return 0;
  }
}


uint8_t Client::first_connect(char * buf, int * length){
  int counter = 0;
  char data[3];

  if(SYS->WebSocket.state == Module_Init){
    loggerf(WARNING, "LOGIN required");
    while(1){
      // Require login
      data[0] = WSopc_Admin_Login;
      this->send(data, 1, 0xFF);

      usleep(100000);

      int status = MessageGet(this->fd, buf, length);

      if(status == 1){
        Parse((uint8_t *)buf, this);
      }
      else if(status == -7){
        counter++;

        if(counter > 10){
          if(!this->ping()){
            loggerf(WARNING, "Client %i timeout", this->fd);
            return 0;
          }
          else{
            counter = 0;
          }
        }
        continue;
      }
      else if(status == -8){
        loggerf(INFO, "Client %i disconnected", this->fd);
        return 0;
      }

      if((this->type & 0x10) == 0){
        loggerf(ERROR, "Client not authenticated");
        return 0;
      }
      else{
        break;
      }
    }
  }

  return 1;
}

int Client::ping(){
  loggerf(DEBUG, "Client::ping fd:%i", this->fd);
  char buf[10];
  buf[0] = WEBSOCKET_FIN | WEBSOCKET_PING;
  buf[1] = 0;

  if(write(this->fd, buf, 2) == -1){
    loggerf(WARNING, "socket write error %x", errno);
  };

  int i = 0;
  while(i < 5){
    memset(buf, 0, 10);

    int32_t recvlength = recv(this->fd, buf, 10, 0);

    if(recvlength <= 0){
      return 0;
    }

    log_hex("WS Client ping", buf, recvlength);

    // opcode
    int opcode = buf[0] & 0b00001111;

    if(opcode == WEBSOCKET_PONG){
      return 1;
    }

    i++;
  }

  return 0;
}

void Client::send(char * data, int length, int flag){
  char * outbuf = (char *)_calloc(length + 100, char);
  int outlength = 0;

  if(!(SYS->WebSocket.state == Module_Run || SYS->WebSocket.state == Module_Init)){
    _free(outbuf);
    return;
  }

  MessageCreate(data, length, outbuf, &outlength);

  pthread_mutex_lock(&m_websocket_send);

  this->send(outbuf, outlength);

  pthread_mutex_unlock(&m_websocket_send);

  _free(outbuf);
}

void Client::send(char * data, int length){
  if(write(this->fd, data, length) == -1){
    if(errno == EPIPE){
      printf("Broken Pipe!!!!!\n\n");
    }
    else if(errno == EFAULT){
      loggerf(ERROR, "EFAULT ERROR");
    }
    else{
      loggerf(ERROR, "Unknown write error, closing connection");
    }
    this->disconnect();
  }  
}

} // Namespace