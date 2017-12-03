#define _BSD_SOURCE

//int stop = 0;

int websocket_connect(int fd_client);

int recv_packet(int fd_client, char outbuf[]);

int send_packet(int fd_client, char data[]);

int send_all(char data[]);

int recv_packet_procces(char data[]);

void * websocket_client(void * thread_data);

void *clear_clients();

void * web_server();
