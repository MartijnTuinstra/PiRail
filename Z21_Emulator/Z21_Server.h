#include "Z21_Codes.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
 * error - wrapper for perror
 */
void die(char *msg);

int server();

struct UDP_return;

void Z21E_recv(char * message, struct UDP_return * rMsg);
