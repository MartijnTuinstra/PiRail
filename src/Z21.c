#define Z21_IP "192.168.0.102"
#define Z21_PORT 4129

void * Z21_client(){
  struct sockaddr_in server_addr;
  socklen_t sin_len = sizeof(server_addr);
  int fd_server;
  int on = 1;
  printf("Server starting...\n");
  fd_server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(fd_server < 0){
    printf("ERROR, SOCKET\n");
    exit(1);
  }
  //fcntl(fd_server, F_SETFL, fcntl(fd_server, F_GETFL) | O_NONBLOCK);

  //setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(Z21_PORT);

  if( inet_aton(Z21_IP, &server_addr.sin_addr) == 0){
    printf("inet_aton() failed\n");
    exit(1);
  }

  char message[100] = "";

  while(1){
    memset(message,'\0', 100);
    printf("Enter message : ");
    gets(message);

    //send the message
    if (sendto(fd_server, message, strlen(message) , 0 , (struct sockaddr *) &server_addr, sin_len)==-1)
    {
        printf("FAILED: sendto()");
    }

    //receive a reply and print it
    //clear the buffer by filling null, it might have previously received data
    memset(message,'\0', 100);
    //try to receive some data, this is a blocking call
    if (recvfrom(fd_server, message, 100, 0, (struct sockaddr *) &server_addr, &sin_len) == -1)
    {
        pirntf("FAILED: recvfrom()");
    }

    puts(message);
  }

}
