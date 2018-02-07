#ifndef H_web
  #define H_web
  //int stop = 0;

  struct web_client_t{
    int fd_client;
    int client_type; /*Flags for client type
                      255 = All messages

                      1  = System messages
                      2  = Switches
                      4  = Signals
                      8  = Track
                      16 = Train info
                      32 = Trains
                      64 =
                      128=*/
    int state;
  };

  int websocket_connect(struct web_client_t * C);

  int recv_packet(int fd_client, char outbuf[], int * L);

  int send_packet(int fd_client, char data[],int length,int flag);

  int send_all(char data[],int length,int flag);

  int recv_packet_procces(char data[]);

  void * websocket_client(void * thread_data);

  void *clear_clients();

  void * web_server();
#endif
