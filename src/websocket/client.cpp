#include <sys/socket.h>
#include <sys/time.h>

#include "websocket/client.h"
#include "websocket/message.h"


// #include "websocket.h"

#include "utils/encryption.h"
#include "utils/logger.h"
#include "utils/mem.h"
#include "system.h"


#include <openssl/sha.h>

void print_hex(char * data, int size);

// Scheduler.cpp
struct timespec operator -(const struct timespec lhs, const struct timespec rhs);
bool operator <(const struct timespec lhs, const struct timespec rhs);

namespace Websocket {

Client::Client(Websocket::Server * _Server, int _fd){
  // Find an empty web_client_t
  // Start a new thread for this client
  Server = _Server;
  fd = _fd;

  if(!websocket_check()){
    char response[100] = "HTTP/1.1 400 OK\r\n\r\n";
    write(fd, response, strlen(response));
    close(fd);
    connected = false;
    return;
  }


  //Set timeout
  struct timeval tv;
  tv.tv_sec = WEBSOCKET_CLIENT_TIMEOUT;
  tv.tv_usec = 0;
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

  //Reset Subscribed Trains
  subscribedTrains[0] = 0xFF;
  subscribedTrains[1] = 0xFF;

  // Set ping time
  clock_getres(CLOCK_REALTIME , &lastPing);

  uint8_t * buf = (uint8_t *)_calloc(WS_BUF_SIZE, uint8_t);
  int length = 0;

  // Send first connect data
  // Return 0 if client is not authenticated properly
  if(!first_connect(&buf, WS_BUF_SIZE, &length)){
    close(fd);
    connected = false;
    _free(buf);
    return;
  }
  _free(buf);

  Server->newClientCallback(this);

  thread_started = true;
  pthread_create(&thread, NULL, (void* (*)(void*))&run, (void *) this);


}

Client::~Client(){
  loggerf(INFO, "Client Destructor");
}

void Client::disconnect(){
  loggerf(DEBUG, "Disconnect Client %i", fd);
  close(fd);
  connected = false;
}

void * Client::run(Client * context){
  uint8_t * buf = (uint8_t *)_calloc(WS_BUF_SIZE, char);
  uint8_t * packet;
  int length = 0;

  while(context->connected){
    // If threre is data recieved
    int status = MessageGet(context->fd, &buf, &packet, WS_BUF_SIZE, &length);

    switch(status){
      case WEBSOCKET_SUCCESS:
        Parse((uint8_t *)buf, context);
        break;

      case WEBSOCKET_SUCCESS_CONTROL_FRAME:
        // TODO
        break;

      case WEBSOCKET_NO_MESSAGE:
        context->timeoutCheck();
        break;

      case WEBSOCKET_FAILED_CLOSE:
        // Unrecoverable error
        context->connected = false;
        break;
    }

    // if(status == 1){
    //   Parse((uint8_t *)buf, context);

    //   timeout_counter = 0;
    // }
    // else if(status == -7){
    //   // timeoutCheck();
    //   timeout_counter++;

    //   if(timeout_counter > 20){
    //     if(!context->ping()){
    //       context->disconnect();
    //       _free(buf);
    //       return 0;
    //     }
    //     else{
    //       timeout_counter = 0;
    //     }
    //   }
    // }
    // else if(status == -8){
    //   context->disconnect();
    //   _free(buf);
    //   return 0;
    // }

    // Server is shutting down
    if(SYS->stop)
      context->connected = false;
  }

  context->disconnect();
  _free(buf);
}


int Client::websocket_check(){
  char * buf = (char *)_calloc(2048, char);

  read(fd, buf, 2040);

  char Connection[20] = "Connection: Upgrade";
  char Connection2[35] = "Connection: keep-alive, Upgrade";
  char UpgradeType[20] = "Upgrade: websocket";
  char Key[25] = "Sec-WebSocket-Key: ";
  char Protocol[20] = "Protocol: ";

  char *key_s, *key_e, *protocol_s, *protocol_e;
  char * key = (char *)_calloc(100, char);
  char * _protocol = (char *)_calloc(10, char);
  int protocol;

  loggerf(INFO, "Web Client (%d) check", fd);

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
    
    write(fd, response, strlen(response));

    type = protocol;

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
    write(fd, response, strlen(response));

    _free(buf);
    _free(key);
    _free(_protocol);

    //Unsuccessfull
    return 0;
  }
}


uint8_t Client::first_connect(uint8_t ** buf, uint16_t bufSize, int * length){
  int counter = 0;
  char data[3];
  uint8_t * packet;

  if(SYS->WebSocket.state == Module_Init){
    loggerf(WARNING, "LOGIN required");
    while(1){
      // Require login
      data[0] = WSopc_Admin_Login;
      send(data, 1, 0xFF);

      usleep(100000);

      int status = MessageGet(fd, buf, &packet, bufSize, length);

      if(status == 1){
        Parse(packet, this);
      }
      else if(status == -7){
        counter++;

        if(counter > 10){
          if(!ping()){
            loggerf(WARNING, "Client %i timeout", fd);
            return 0;
          }
          else{
            counter = 0;
          }
        }
        continue;
      }
      else if(status == -8){
        loggerf(INFO, "Client %i disconnected", fd);
        return 0;
      }

      if((type & 0x10) == 0){
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
  buf[1] = 0; // No length

  if(write(this->fd, buf, 2) == -1){
    loggerf(WARNING, "socket write error %x", errno);
  };

  // int i = 0;
  // while(i < 5){
  //   memset(buf, 0, 10);

  //   int32_t recvlength = recv(this->fd, buf, 10, 0);

  //   if(recvlength <= 0){
  //     return 0;
  //   }

  //   log_hex("WS Client ping", buf, recvlength);

  //   // opcode
  //   int opcode = buf[0] & 0b00001111;

  //   if(opcode == WEBSOCKET_PONG){
  //     return 1;
  //   }

  //   i++;
  // }

  return 0;
}

bool Client::timeoutCheck(){
  struct timespec time;
  clock_getres(CLOCK_REALTIME, &time);

  time = time - lastPing;

  if(time.tv_sec < 30)
    return true;

  if(!timeout){
    timeout = true;

    ping();
  }
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