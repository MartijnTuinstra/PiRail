#include "Z21_Codes.h"

#include <sys/types.h>
//#include <sys/socket.h>
#include <sys/stat.h>
//#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "Z21_Server.c"

/*
 * error - wrapper for perror
 */
void error(char *msg);

int server();
