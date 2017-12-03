#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <openssl/sha.h>
#include "b64.c"

int stop = 0;
#define MAX_WEB_CLIENTS 10

char websocket_magic_string[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

pthread_t client_threads[MAX_WEB_CLIENTS];

struct client_thread_args{
  int fd_client;
  int thread_id;
};

struct client_thread_args clients_data[MAX_WEB_CLIENTS];
int clients[MAX_WEB_CLIENTS] = {0};
int fd_client_list[MAX_WEB_CLIENTS] = {0};

int websocket_connect(int fd_client){
  char buf[1024];

  memset(buf, 0, 1024);
  printf("\n");
  read(fd_client, buf, 1023);
  printf("\n");


  printf("%s\n\n",buf); //Print request buffer

  char a[] = "Connection: Upgrade";
  char b[] = "Upgrade: websocket";
  char c[] = "Sec-WebSocket-Key: ";

  char *start, *end, target[60];
  memset(target,0,60);

  if(strstr(buf, a) != NULL && strstr(buf, b) != NULL && strstr(buf, c) != NULL) {
    printf("\nIt is a HTML5 WebSocket!!\n\n");
    start = strstr(buf, c);
    start += strlen(c);
    printf("start: %i\n",&start);
    if (end = strstr(start,"\r\n")){
        strncat(target,start,end-start);
        //memcpy( target, start, end - start );
        //target[end - start] = '\0';
    }
    //strcat(target,"dGhlIHNhbXBsZSBub25jZQ==");
    printf("Socket-key: '%s'\n\n",target);
    strcat(target,websocket_magic_string);
    printf("before hash: %i chars: %s\n\n",sizeof(target),target);

    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(target, sizeof(target), hash);
    printf("hex hash (%i): ",SHA_DIGEST_LENGTH);
    for(int i = 0;i<20;i++){
      printf("%x",hash[i]);
    }
    char b64[40] = "";
    base64_encode(hash,sizeof(hash),b64,40);
    printf("\n\nbaseoutput: %s\n\n",b64);

    char response[] = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ";
    strcat(response,b64);
    strcat(response,"\r\n\r\n");
    printf("response:\n\n%s\n\n",response);
    write(fd_client, response, strlen(response));
    return 1;
  }else{
    return 0;
  }
}

int recv_packet(int fd_client, char outbuf[]){
  char buf[1024];
  memset(buf,0,1024);
  memset(outbuf,0,sizeof(outbuf));
  usleep(10000);
  recv(fd_client,buf,1024,0);

  int byte = 0;

  byte = 0;
  printf("FIN\topCode\tMask\tLength\n");
  printf("%i\t",((buf[byte] & 0b10000000)>>7));
  int opcode = buf[byte++] & 0b00001111;
  printf("%i\t",opcode);
  printf("%i\t",((buf[byte] & 0b10000000)>>7));
  unsigned int mes_length = buf[byte++] & 0b01111111;
  if(mes_length == 126){
    mes_length = (buf[byte++] << 8) + buf[byte++];
    //printf("len %2x%2x\n",buf[byte++],buf[byte++]);
  }else if(mes_length == 127){
    printf("len %2x%2x%2x%2x%2x%2x%2x%2x\n",buf[byte++],buf[byte++],buf[byte++],buf[byte++],buf[byte++],buf[byte++],buf[byte++],buf[byte++]);
  }
  printf("%i\n",mes_length);
  int masking_index = byte;
  unsigned int masking_key = (buf[byte++] << 24) + (buf[byte++] << 16) + (buf[byte++] << 8) + (buf[byte++]);

  char output[mes_length+2];
  memset(output,0,mes_length+2);

  for(int i = 0;i<mes_length;0){
    unsigned int test;
    unsigned int text;
    test = (buf[byte++] << 24) + (buf[byte++] << 16) + (buf[byte++] << 8) + (buf[byte++]);
    text = test ^ masking_key;
      //printf("%c ",(text & 0xFF000000) >> 24);
      output[i++] = (text & 0xFF000000) >> 24;
      if(i<mes_length){
        //printf("%c ",(text & 0xFF0000) >> 16);
        output[i++] = (text & 0xFF0000) >> 16;
      }
      if(i<mes_length){
        //printf("%c ",(text & 0xFF00) >> 8);
        output[i++] = (text & 0xFF00) >> 8;
      }
      if(i<mes_length){
        //printf("%c ",(text & 0xFF));
        output[i++] = text & 0xFF;
      }
      //i+=4;
  }

  if(opcode == 8){
    printf("Connection closed by client\n\n");
    return -8;
  }

  strcat(outbuf,output);
  return 1;
}

int send_packet(int fd_client, char data[]){
  char outbuf[1024];
  memset(outbuf,0,1024);
  outbuf[0] = 0b10000000 + 0b00000001;
  if(strlen(data) < 126){
    outbuf[1] = strlen(data);
    strcat(outbuf,data);
  }else if(strlen(data) < 65535){
    outbuf[1] = 126;
    outbuf[2] = 0xFF;
    outbuf[3] = 0xFF;
    strcat(outbuf,data);
    outbuf[2] = strlen(data) & 0xFF00 >> 8;
    outbuf[3] = strlen(data) & 0xFF;
  }
  write(fd_client,outbuf,strlen(outbuf));
}

int send_all(char data[]){
  for(int i = 0;i<MAX_WEB_CLIENTS;i++){
    if(fd_client_list[i] != 0){
      send_packet(fd_client_list[i],data);
    }
  }
}

void * websocket_client(void * thread_data){
  struct client_thread_args *thread_args;
	thread_args = (struct client_thread_args *) thread_data;
	int i = thread_args->thread_id;
	int fd_client = thread_args->fd_client;

  printf("New websocket_client");
  if(websocket_connect(fd_client)){
      char buf[1024];
      memset(buf,0,1024);

      while(1){
        // If threre is data recieved
        if(recv(fd_client,buf,1024,MSG_PEEK | MSG_DONTWAIT) > 1){
          printf("Data received\n");
          usleep(10000);
          int status = recv_packet(fd_client,buf);
          if(status == 1){
            printf("\n%s\n",buf);
          }else if(status == -8){
            close(fd_client);
            clients[i] = 2;
            return;
          }

        }

        if(0){ //If there is data to send
          printf("Just send data!!\n");
          usleep(1000000);
          send_packet(fd_client,"Hello World!!");
        }

      }
  }else{
    printf("Wrong HTTP request!!!!\n");
    close(fd_client);
    clients[i] = 2;
    return;
  }
}

void *clear_clients(){
	while(!stop){
		for(int i = 0;i<MAX_WEB_CLIENTS;i++){
			if(clients[i] == 2){
				pthread_join(client_threads[i], NULL);
				clients[i] = 0;
				printf("Reset client %i\n",i);
			}
		}
	}
}

void * web_server(){
  struct sockaddr_in server_addr, client_addr;
  socklen_t sin_len = sizeof(client_addr);
  int fd_server, fd_client;
  int fdimg;
  int on = 1;
  printf("Server starting...\n");
  fd_server = socket(AF_INET, SOCK_STREAM, 0);
  if(fd_server < 0){
    printf("ERROR, SOCKET\n");
    exit(1);
  }
  //fcntl(fd_server, F_SETFL, fcntl(fd_server, F_GETFL) | O_NONBLOCK);

  setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(9000);

  if(bind(fd_server, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
    printf("ERROR, BIND\n");
    close(fd_server);
    exit(1);
  }

  if(listen(fd_server, MAX_WEB_CLIENTS) == -1){
    printf("ERROR, LISTEN\n");
    close(fd_server);
    exit(1);
  }

  pthread_t clear;
  pthread_create(&clear, NULL, clear_clients, NULL);

  int q = 0;

  while(1){
    fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &sin_len);

    if(fd_client == -1){
      if(q == 0){
        printf("ERROR, Client Connect\n");
      }
      q = 1;
      continue;
    }
    q = 0;

    int i = 0;
  	while(1){
  		if(clients[i] == 0){
  			clients[i] = 1;
  			clients_data[i].thread_id = i;
  			clients_data[i].fd_client = fd_client;
  			printf("Create client thread %i\n",i);
  			pthread_create(&client_threads[i], NULL, websocket_client, (void *) &clients_data[i]);
  			break;
  		}
  		i++;
  		if(i==MAX_WEB_CLIENTS){
  			i = 0;
  			printf("Too many clients!!!!!\n\n");
  			usleep(100000);
  		}
  	}

    if(stop){
      close(fd_server);
      break;
    }
  }
}
