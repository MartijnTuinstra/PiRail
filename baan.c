#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <pthread.h>
#include <wiringPi.h>
#include <signal.h>

#include <errno.h>

#include "logger.h"
#include "rail.h"
#include "switch.h"
#include "train.h"
#include "module.h"

#include "algorithm.h"

#include "com.h"

#include "websocket_control.h"

#include "system.h"

#include "train_sim.h"

struct systemState * _SYS;

int main(){
  _SYS = _calloc(1, struct systemState);
  _SYS->_STATE = STATE_RUN;
  _SYS->_Clients = 0;
  _SYS->_COM_fd = -1;

  init_logger("log.txt");
  set_level(DEBUG);

  if (signal(SIGINT, sigint_func) == SIG_ERR){
    logger("Cannot catch SIGINT",CRITICAL);
    return 0;
  }

  setbuf(stdout,NULL);
  setbuf(stderr,NULL);
  signal(SIGPIPE, SIG_IGN);
  srand(time(NULL));

  pthread_t th_web_server, th_UART, th_Z21;

  /* Wiring PI */
    wiringPiSetup();

    pinMode(0, OUTPUT); //GPIO17
    pinMode(1, OUTPUT); //GPIO17
    digitalWrite(0,LOW);
    digitalWrite(1,LOW);
  /* Websocket server */
    pthread_create(&th_web_server, NULL, web_server, NULL);
    WS_init_Message_List();
  /* UART */

  /* Z21 Client */

  /* Enable Web clients */

  /* Load cars, engines and trains*/
    init_trains();

  /* Search all blocks */
    init_modules();
    char * DeviceList = _calloc(10, char);

    if(DeviceList[0] != 0){
      //There are allready some device listed. Clear.
      memset(DeviceList,0,10);
    }

    //Send restart message
    COM_DevReset();

    usleep(200000); //Startup time of devices
    usleep(1000000);//Extra time to make sure it collects all info
    logger("Found module 20",INFO);
    DeviceList[0] = 20;
    logger("Found module 21",INFO);
    DeviceList[1] = 21;
    logger("Found module 22",INFO);
    DeviceList[2] = 22;
    logger("Found module 23",INFO);
    DeviceList[3] = 23;
    // DeviceList[4] = 5;
    // DeviceList[5] =10;
    // DeviceList[6] =11;
    // DeviceList[7] = 6;

    for(uint8_t i = 0;i<10;i++){
      if(DeviceList[i]){
        LoadModules(DeviceList[i]);
      }
    }
    

    _SYS_change(STATE_Modules_Loaded,1);

    JoinModules();


    setup_JSON((int [4]){20,21,22,23},(int *)0,4,0);

    Connect_Rail_links();

  //#############################################################
  //Init done

  //scan_All();
  printf("Next 1\n");
  Block *B = Next(Units[20]->B[3],0,1);
  if(B){printf("Block %i:%i\n",B->module, B->id);}
  printf("Next 2\n");
  B = Next(Units[20]->B[3],0,2);
  if(B){printf("Block %i:%i\n",B->module, B->id);}
  printf("Next 3\n");
  B = Next(Units[20]->B[3],0,3);
  if(B){printf("Block %i:%i\n",B->module, B->id);}
  printf("\nPrev 1\n");
  B = Next(Units[20]->B[3],1,1);
  if(B){printf("Block %i:%i\n",B->module, B->id);}
  printf("Prev 2\n");
  B = Next(Units[20]->B[3],1,2);
  if(B){printf("Block %i:%i\n",B->module, B->id);}
  printf("Prev 3\n");
  B = Next(Units[20]->B[3],1,3);
  if(B){printf("Block %i:%i\n",B->module, B->id);}

  delay(5);

  return 1;

  if(_SYS->_Clients == 0){
    printf("                   Waiting until for a client connects\n");
  }
  while(_SYS->_Clients == 0){
    usleep(1000000);
  }

  usleep(400000);

  pthread_t pt_scan_All, pt_train_timers, pt_train_simA;

  pthread_create(&pt_scan_All, NULL, scan_All_continiously, NULL);
  // pthread_create(&pt_train_timers, NULL, clear_train_timers, NULL);
  pthread_create(&pt_train_simA, NULL, TRAIN_SIMA, NULL);

  usleep(10000000);

  WS_ShortCircuit();

  while(_SYS->_STATE & STATE_RUN){
    usleep(100000);
  }

  logger("Stopping Argor",INFO);
  pthread_join(pt_scan_All,NULL);

  logger("Free memory",INFO);
  logger("FREE MEMORY",CRITICAL);
  free_modules();
  free_trains();
  //pthread_join(tid[1],NULL);
  //printf("STOP JOINED\n");
  //pthread_join(tid[2],NULL);
  //printf("Timer JOINED\n");
  logger("Stopping Train Sim",INFO);
  pthread_join(pt_train_simA,NULL);
  //pthread_join(tid[4],NULL);
  //printf("SimB JOINED\n");

  logger("Stopping UART control",INFO);
  pthread_join(th_UART,NULL);

  logger("Stopping Websocket server",INFO);
  pthread_join(th_web_server,NULL);
  //procces(C_Adr(6,2,1),1);

  //----- CLOSE THE UART -----
  close(_SYS->_COM_fd);

  _free(_SYS);

  _free(DeviceList);

  printf("STOPPED");
  //pthread_exit(NULL);
}
