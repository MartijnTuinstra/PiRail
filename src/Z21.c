#include "Z21.h"
#include "logger.h"

void die(char *s){
	loggerf(CRITICAL, "Crashed: %s", s);
}

void Z21_client(){}

char Z21_prio_list[05][30];
char Z21_send_list[10][30];

void Z21_recv(char message[100]){
	loggerf(CRITICAL, "UNIMPLEMENTED (%s)", message);
}

void Z21_send(int Header,char data[30]){
	loggerf(CRITICAL, "UNIMPLEMENTED (%i, %s)", Header, data);
}

void Z21_get_train(Trains * T){
  loggerf(ERROR, "Implement Z21_get_train (%x)", (unsigned int)T);
}

void Z21_get_engine(int dcc){
  loggerf(ERROR, " Implement Z21_get_engine (%x)", dcc);
}

void Z21_SET_LOCO_DRIVE(int DCC_Adr,char steps,_Bool dir,char drive){
  loggerf(ERROR, "Implement Z21_SET_LOCO_DRIVE (%x, %x, %x, %x)", DCC_Adr, steps, dir, drive);
}

void Z21_SET_LOCO_FUNCTION(int DCC_Adr,char function_nr,char switch_type){
  loggerf(ERROR, "Implement Z21_SET_LOCO_Function (%x, %x, %x)", DCC_Adr, function_nr, switch_type);
}
