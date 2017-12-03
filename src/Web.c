char return_succ[] = "HTTP/1.1 204 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: text/plain; charset=URF-8\r\n\r\n";
char return_fail[] = "HTTP/1.1 205 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: text/plain; charset=URF-8\r\n\r\nF\r\n";

void * web_server(){
  struct sockaddr_in server_addr, client_addr;
  socklen_t sin_len = sizeof(client_addr);
  int fd_server, fd_client;
  char buf[1024];
  int fdimg;
  int on = 1;
  printf("Server starting...\n");
  fd_server = socket(AF_INET, SOCK_STREAM, 0);
  if(fd_server < 0){
    printf("ERROR, SOCKET\n");
    exit(1);
  }

  setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(9000);

  if(bind(fd_server, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
    printf("ERROR, BIND\n");
    close(fd_server);
    exit(1);
  }

  if(listen(fd_server, 10) == -1){
    printf("ERROR, LISTEN\n");
    close(fd_server);
    exit(1);
  }

  while(1){
    fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &sin_len);

    if(fd_client == -1){
      printf("ERROR, Client Connect\n");
      continue;
    }

    printf("\nGot client\n");

    memset(buf, 0, 1024);
    printf("\n");
    read(fd_client, buf, 1023);
    printf("\n");

    char *a = strchr(buf, '/');


    printf("%s\n\n",buf); //Print request buffer

    if(a == NULL){
      write(fd_client, return_fail, sizeof(return_fail) - 1);
      printf("Wrong request!!!\n\n");
      close(fd_client);
      continue;
    }
    //Commands for changing switches and complex_Switches
    if(buf[a-buf+1] == 'S' || buf[a-buf+1] == 'M' || buf[a-buf+1] == 'm' || buf[a-buf+1] == 'L'){
      char *M = strchr(&buf[(a-buf)+1], ':');
      char *B = strchr(&buf[(M-buf)+1], ':');
      char *S = strchr(&buf[(B-buf)+1], '?');
      if (a != NULL&& M != NULL && B != NULL && S != NULL){ /* deal with error: / not present" */;
        *S = 0;

        int start = a-buf+1;
        int colom1 = M-buf;
        int colom2 = B-buf;
        int end = S-buf;

        char s_M[5],s_B[5],s_S[5];

        memset(s_M,0,5);
        memset(s_B,0,5);
        memset(s_S,0,5);

        for(int i = (start+1);i<=end;i++){
          if(i < colom1){
            s_M[(i-(start+1))] = buf[i];
          }else if(i > colom1 && i < colom2){
            s_B[(i-(colom1+1))] = buf[i];
          }else if(i > colom2 && i < end){
            s_S[(i-(colom2+1))] = buf[i];
          }
          if(i == colom1){
            s_M[(i-(start+1))] = 0;
          }else if(i == colom2){
            s_B[(i-(colom1+1))] = 0;
          }else if(i == (B-buf)){
            s_S[(i-(colom2+1))] = 0;
          }
          printf("%i: %c\n",i,buf[i]);
        }

        printf("s_M %s\ts_B %s\ts_S %s\n",s_M,s_B,s_S);

        int s_m = atoi(s_M);
        int s_b = atoi(s_B);
        int s_s = atoi(s_S);

        if(buf[start] == 'S'){
          if(Switch[s_m][s_b][s_s] != NULL){
            printf("throw switch %i:%i:%i\t",s_m,s_b,s_s);
            printf("%i->%i",Switch[s_m][s_b][s_s]->state, !Switch[s_m][s_b][s_s]->state);
            pthread_mutex_lock(&mutex_lockB);
            Switch[s_m][s_b][s_s]->state = !Switch[s_m][s_b][s_s]->state;
            pthread_mutex_unlock(&mutex_lockB);
            write(fd_client, return_succ, sizeof(return_succ) - 1);
          }else{
            write(fd_client, return_fail, sizeof(return_fail) - 1);
          }
        }else if(buf[start] == 'M' || buf[start] == 'm'){
          if(Moduls[s_m][s_b][s_s] != NULL){
            if(buf[start] == 'M'){
              printf("throw switch + %i:%i:%i\t",s_m,s_b,s_s);
              printf("%i->%i",Moduls[s_m][s_b][s_s]->state, Moduls[s_m][s_b][s_s]->state+1);
              pthread_mutex_lock(&mutex_lockB);
              Moduls[s_m][s_b][s_s]->state += 1;

              if(Moduls[s_m][s_b][s_s]->state >= Moduls[s_m][s_b][s_s]->length){
                Moduls[s_m][s_b][s_s]->state = 0;
              }
              pthread_mutex_unlock(&mutex_lockB);
              write(fd_client, return_succ, sizeof(return_succ) - 1);
            }
          }else{
            write(fd_client, return_fail, sizeof(return_fail) - 1);
          }
        }else{
          //Link trains
          status_rem(s_m,11);
          write(fd_client, return_succ, sizeof(return_succ) - 1);
          printf("Linking train %i with dcc address #%i",s_b,s_s);
        }
      }else{
        printf("Pointer problem\n");
        write(fd_client, return_fail, sizeof(return_fail) - 1);
      }
    }else if(buf[a-buf+1] == 'E' && (buf[a-buf+2] == 'r' || buf[a-buf+2] == 'c')){
      char *S = strchr(buf, '?');

      int start = a-buf+1;
      int end = S-buf;

      char nr_s[4];

      for(int i = (start+2);i<=end;i++){
        nr_s[(i-(start+2))] = buf[i];
        if(i == end){
          nr_s[(i-(start+2))] = 0;
        }
        //printf("%i: %c\n",i,buf[i]);
      }

      int nr = atoi(nr_s);

      if(buf[a-buf+2] == 'r'){
        printf("Disable/Release Emergency Stop %i\n",nr);
        status_rem(nr,1);
      }else if(buf[a-buf+2] == 'c'){
        printf("Disable/Release Emergency Short Circuit Stop %i\n",nr);
        status_rem(nr,2);
      }
      write(fd_client, return_succ, sizeof(return_succ) - 1);
    }

    printf("\nclosing client..\n");

    close(fd_client);

    if(stop){
      close(fd_server);
      break;
    }
  }
}
