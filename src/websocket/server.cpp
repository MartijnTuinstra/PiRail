#include <sys/socket.h>
#include <stdint.h>

#include "websocket/client.h"
#include "websocket/server.h"
#include "websocket/message.h"
#include "websocket/message_structure.h"
#include "websocket/stc.h"

#include "switchboard/manager.h"
#include "switchboard/unit.h"

#include "rollingstock/manager.h"

#include "scheduler/scheduler.h"
#include "system.h"
#include "utils/logger.h"
#include "utils/mem.h"
#include "sim.h"

pthread_mutex_t m_websocket_send;
Websocket::Server * WSServer;

namespace Websocket {

Server::Server(){}
Server::~Server(){}

void Server::run(){
  if(!this->init())
    return;

  this->loop();

  this->teardown();
}

void Server::readPassword(){
  long WS_pass_length;
  FILE * f = fopen ("./password.txt", "rb");

  if (f)
  {
    fseek (f, 0, SEEK_END);
    WS_pass_length = ftell (f);
    fseek (f, 0, SEEK_SET);
    if (WS_pass_length == 32)
    {
      fread (this->password, 1, WS_pass_length, f);
    }
    else{
      loggerf(ERROR, "PASSWORD: wrong length or unable to allocate memory");
    }
    fclose (f);
  }
}

int Server::init(){
  loggerf(INFO, "Starting websocket server at port %i", WEBSOCKET_PORT);

  struct sockaddr_in server_addr;

  int on = 1;

  memset(this->password, 0, 100);
  this->readPassword();

  loggerf(INFO, "Websocket Password: %s", this->password);

  this->fd = socket(AF_INET, SOCK_STREAM, 0);
  if(this->fd < 0){
    loggerf(CRITICAL, "SOCKET ERROR");
    SYS->stop = 1;
    SYS_set_state(&SYS->WebSocket.state, Module_Fail);
    return 0;
  }

  setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(WEBSOCKET_PORT);

  if(bind(this->fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
    loggerf(CRITICAL, "BIND ERROR");
    close(this->fd);
    SYS->stop = 1;
    SYS_set_state(&SYS->WebSocket.state, Module_Fail);
    return 0;
  }

  if(listen(this->fd, MAX_WEB_CLIENTS) == -1){
    loggerf(CRITICAL, "LISTEN ERROR");
    close(this->fd);
    SYS->stop = 1;
    SYS_set_state(&SYS->WebSocket.state, Module_Fail);
    return 0;
  }

  //Start clear_clients
  struct SchedulerEvent event = {{30, 0}, (void (*)(void *))&this->ClearUnusedSockets, this, "_ClearUnusedSockets", 0, {0, 0}};
  scheduler->addEvent(event);

  // WS_init_Message_List();
  
  SYS_set_state(&SYS->WebSocket.state, Module_Init);

  //Set server timeout
  struct timeval tv;
  tv.tv_sec = WEBSOCKET_CLIENT_TIMEOUT;
  tv.tv_usec = 0;
  setsockopt(this->fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

  return 1;
}

void Server::loop(){
  loggerf(INFO, "Listening for  Clients %i", SYS->stop);
  int fd_client;
  struct sockaddr_in client_addr;
  socklen_t sin_len = sizeof(client_addr);

  while(SYS->stop == 0){
    // Run until system is stopped, or until client_accept is closed

    fd_client = accept(this->fd, (struct sockaddr *)&client_addr, &sin_len);

    if(fd_client == -1){
      if(errno == EAGAIN || errno == ETIMEDOUT){
        continue;
      }
      // else if(errno == EINTR){
      //   break;
      // }
      else{
        loggerf(WARNING, "Failed to connect with client %i", errno);
        continue;
      }
    }

    unsigned long addr = client_addr.sin_addr.s_addr;
    loggerf(INFO, "New socket client %i.%i.%i.%i", addr&0xFF, (addr >> 8)&0xFF, (addr >> 16)&0xFF, (addr >> 24)&0xFF);

    if(this->clients.size() < MAX_WEB_CLIENTS)
      this->clients.push_back(new Client(this, fd_client));
    else{
      char response[100] = "HTTP/1.1 400 OK\r\n\r\n";
      write(fd, response, strlen(response));
      close(fd);
    }
  }
}

void Server::teardown(){
  loggerf(INFO, "Stopping  Server");

  scheduler->removeEvent("_ClearUnusedSockets");

  close(this->fd);

  loggerf(INFO, "Stopping websocket_clear_clients");

  SYS_set_state(&SYS->WebSocket.state, Module_STOP);
}

void * Server::ClearUnusedSockets(Server * context){
  for(uint8_t i = 0; i < context->clients.size(); i++){
    if(!context->clients[i]->connected){
      loggerf(INFO, "Stopping websocket client %i thread", i);
      if(context->clients[i]->thread_started)
        pthread_join(context->clients[i]->thread, NULL);
      else
        loggerf(WARNING, "Client not even started");
      delete context->clients[i];
      context->clients.erase(context->clients.begin() + i);
      i--;
    }
  }

  return nullptr;
}

void Server::newClientCallback(Client * client){

  char data[10] = {0,0,0,0,0,0,0,0,0,0};

  // Send Enabled options
  // data[0] = WSopc_Service_State;
  // data[1] = SYS->_STATE >> 8;
  // data[2] = SYS->_STATE & 0xFF;
  // client->send(data, 3, 0xFF);

  //Send submodule status
  WS_stc_SubmoduleState(0);
  
  //Send track layout and data
  if(SYS->modules_loaded){
    // Send Track Layout Data
    for(int i = 0; i < switchboard::SwManager->Units.size; i++){
      if(!switchboard::Units(i))
        continue;

      WS_stc_Track_LayoutDataOnly(i, client);
    }

    WS_stc_StationLib(client);

    if(SYS->modules_linked){
      WS_stc_Track_Layout(client);
      
      // Send new client JSON
      WS_stc_NewClient_track_Switch_Update(client);
    }
  }

  // Send open messages
  WS_send_open_Messages(client);


  // Send broadcast flags
  data[0] = WSopc_ChangeBroadcast;
  data[1] = client->type;
  client->send(data, 2, 0xFF);

  if(SYS->trains_loaded){
    loggerf(INFO, "Update clients libs %i", client->fd);
    WS_stc_EnginesLib(client);
    WS_stc_CarsLib(client);
    WS_stc_TrainSetsLib(client);
    WS_stc_TrainCategories(client);

    //train_link, train_link_lenlink_id
    // Send all linked trains
    for(uint8_t i = 0; i < RSManager->Trains.size; i++){
      Train * T = RSManager->Trains[i];
      if(!T)
        continue;

      struct s_opc_LinkTrain msg;
      msg.follow_id = i;
      if(T->type){
        loggerf(WARNING, "Sending train T %i\t(%i) %s", i, T->p.T->id, T->p.T->name);
        msg.real_id = T->p.T->id;
        msg.type = 0;
      }
      else{
        loggerf(WARNING, "Sending train E %i\t(%i) %s", i, T->p.E->id, T->p.E->name);
        msg.real_id = T->p.E->id;
        msg.type = 1;
      }
      msg.message_id_H = 0;
      msg.message_id_L = 0;

      WS_stc_LinkTrain(&msg);
    }
  }

  if(SYS->Z21.state & Module_Run){
    WS_stc_Z21_info(client);
  }
  WS_stc_Z21_IP(client);

  SIM_Client_Connect_cb();

  loggerf(INFO, "Client Connected Done");

}



void Server::send_all(char * data,int length,int flag){
  char * outbuf = (char *)_calloc(length + 100, char);
  int outlength = 0;

  if(!(SYS->WebSocket.state == Module_Run || SYS->WebSocket.state == Module_Init)){
    _free(outbuf);
    return;
  }

  MessageCreate(data, length, outbuf, &outlength);

  pthread_mutex_lock(&m_websocket_send);
  for(uint8_t i = 0; i < this->clients.size(); i++){
    if(this->clients[i]->connected && (this->clients[i]->type & flag) != 0){
      this->clients[i]->send(outbuf, outlength);
    }
  }
  pthread_mutex_unlock(&m_websocket_send);

  _free(outbuf);
}


} // Namespace