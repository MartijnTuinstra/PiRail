#include "logger.h"

void die(char *s){
}

void Z21_client(){}

char Z21_prio_list[05][30];
char Z21_send_list[10][30];

void Z21_recv(char message[100]){}

void Z21_send(int Header,char data[30]){}

void Z21_GET_LOCO_INFO(int DCC_Adr){
  loggerf(ERROR, "Implement Z21_GET_LOCO_INFO");
}

void Z21_SET_LOCO_DRIVE(int DCC_Adr,char steps,_Bool dir,char drive){}

void Z21_SET_LOCO_FUNCTION(int DCC_Adr,char function_nr,char switch_type){}
