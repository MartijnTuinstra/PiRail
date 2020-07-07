#ifndef _INCLUDE_WEBSOCKET_CLIENT_H
#define _INCLUDE_WEBSOCKET_CLIENT_H

#include <pthread.h>
#include <stdint.h>

#include "websocket/server.h"

namespace Websocket {

class Client {
  public:
    int fd;
    Websocket::Server * Server;

    uint8_t subscribedTrains[2] = {0xFF,0xFF};
    
    pthread_t thread;
    int type; /*Flags for client type
                        255 = All messages
                        1  = Trains
                        2  = Track
                        4  = Switches
                        8  = Messages
                        16 = Admin
                        32 = 
                        64 =
                        128=*/

    bool connected = true;

    Client(Websocket::Server * Server, int fd);
    ~Client();

    static void * run(Client * context);

    void disconnect();
    uint8_t first_connect(char * buf, int * length);
    int websocket_check();

    int ping();

    void send(char * data, int length, int flag);
    void send(char * data, int length);
};

}

#endif