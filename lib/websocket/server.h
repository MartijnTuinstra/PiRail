#ifndef _INCLUDE_WEBSOCKET_SERVER_H
#define _INCLUDE_WEBSOCKET_SERVER_H

#include <vector>

#define WEBSOCKET_PORT 9000
#define MAX_WEB_CLIENTS 20
#define WS_BUF_SIZE 1024

namespace Websocket {

class Client;
class Messages;

class Server {
  public:
    std::vector<Client *> clients;

    char password[100];

    std::vector<Messages *> messages;

    int fd = -1;

    Server();
    ~Server();

    int init();
    void loop();
    void teardown();

    void run();

    void readPassword();
    static void * ClearUnusedSockets(Server * context);

    void newClientCallback(Client * client);
    void send_all(char * data,int length,int flag);
};

}

extern pthread_mutex_t m_websocket_send;
extern Websocket::Server * WSServer;

#endif