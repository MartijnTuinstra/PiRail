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

  void send_all(char data[],int length,int flag);

  int recv_packet_procces(char data[]);

  void * websocket_client(void * thread_data);

  void *clear_clients();

  void * web_server();

  //Opcodes
  #define WSopc_ToggleSwitch       0x20
  #define WSopc_ToggleMSSwitchUp   0x21
  #define WSopc_ToggleMSSwitchDown 0x22
  #define WSopc_SetSwitch          0x23
  #define WSopc_SetSwitchReserved  0x24
  #define WSopc_SetSwitchRout      0x25
  #define WSopc_BroadTrack         0x26
  #define WSopc_BroadSwitch        0x27

  #define WSopc_EmergencyStop      0x10
  #define WSopc_ShortCircuitStop   0x11
  #define WSopc_ClearEmergency     0x12
  #define WSopc_NewMessage         0x13
  #define WSopc_ClearMessage       0x14

#endif
