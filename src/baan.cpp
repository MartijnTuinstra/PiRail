#include "system.h"
#include "mem.h"

#include "logger.h"
// #include "rail.h"
// #include "switch.h"
#include "train.h"
#include "modules.h"


#include "scheduler/scheduler.h"

#include "websocket/server.h"

#include "pathfinding.h"
// #include "train_sim.h"
#include "Z21.h"

struct s_systemState * SYS;

char * UART_Serial_Port = 0;

int main(int argc, char * argv[]){
  init_main();
  init_logger("log.txt");
  set_level(MEMORY);
  set_logger_print_level(DEBUG);

  if(argc > 1){
    printf("Got argument %s\n", argv[1]);
    UART_Serial_Port = (char *)_calloc(strlen(argv[1])+5, char);
    strcpy(UART_Serial_Port, argv[1]);
  }

  // Stop program when receiving SIGINT
  if (signal(SIGINT, sigint_func) == SIG_ERR){
    logger("Cannot catch SIGINT", CRITICAL);
    return 0;
  }

  setbuf(stdout,NULL);
  setbuf(stderr, NULL);
  signal(SIGPIPE, SIG_IGN);
  srand(time(NULL));
  
  load_module_Configs();
  load_rolling_Configs();

  scheduler->start();

  Z21 = new Z21_Client();

  WSServer = new Websocket::Server();
  WSServer->init();
  WSServer->loop();
  WSServer->teardown();
  // websocket_server();

  // UART_stop();
  loggerf(WARNING, "Shutdown sequence started");

  // Stop all
  SYS->stop = 1;

  scheduler->stop();

  unload_module_Configs();
  unload_rolling_Configs();

  loggerf(INFO, "STOPPED");
  exit_logger(); //Close logger
  delete WSServer;
  _free(SYS);

  print_allocs();
}

//   pthread_t th_web_server;
//   //pthread_t th_UART;

//   /* Wiring PI */
//     wiringPiSetup();

//     pinMode(0, OUTPUT); //GPIO17
//     pinMode(1, OUTPUT); //GPIO17
//     digitalWrite(0,LOW);
//     digitalWrite(1,LOW);
//   /* Websocket server */
//   /* UART */

//   /* Z21 Client */
//     pthread_t th_Z21;
//     Z21(&th_Z21);

//   /* Enable Web clients */
//     usleep(20000000);
//     loggerf(INFO, "Stop");
//     _SYS_change(STATE_RUN | STATE_Client_Accept, 3);

//     pthread_join(th_Z21, NULL);
//     return;
//   /* Load cars, engines and trains*/
//     init_trains();

//   /* Search all blocks */
//     init_modules();
//     char * DeviceList = (char *)_calloc(10, char);

//     if(DeviceList[0] != 0){
//       //There are allready some device listed. Clear.
//       memset(DeviceList,0,10);
//     }

//     //Send restart message
//     COM_DevReset();

//     usleep(200000); //Startup time of devices
//     usleep(1000000);//Extra time to make sure it collects all info
//     logger("Found module 20",INFO);
//     DeviceList[0] = 20;
//     logger("Found module 21",INFO);
//     DeviceList[1] = 21;
//     logger("Found module 22",INFO);
//     DeviceList[2] = 22;
//     logger("Found module 23",INFO);
//     DeviceList[3] = 23;
//     // DeviceList[4] = 5;
//     // DeviceList[5] =10;
//     // DeviceList[6] =11;
//     // DeviceList[7] = 6;

//     for(uint8_t i = 0;i<10;i++){
//       if(DeviceList[i]){
//         LoadModuleFromConfig(DeviceList[i]);
//       }
//     }


//     _SYS_change(STATE_Modules_Loaded,1);

//     JoinModules();


//     setup_JSON((int [4]){20,21,22,23},(int *)0,4,0);

//     Connect_Rail_links();


//   // Allow web clients
//   _SYS_change(STATE_Client_Accept,1);

//   //#############################################################
//   //Init done

//   // scan_All();
//   // printf("Next 1\n");
//   // Block *B = Next(Units[20]->B[3], NEXT, 1);
//   // if(B){printf("Block %i:%i\n",B->module, B->id);}
//   // printf("Next 2\n");
//   // B = Next(Units[20]->B[3], NEXT, 2);
//   // if(B){printf("Block %i:%i\n",B->module, B->id);}
//   // printf("Next 3\n");
//   // B = Next(Units[20]->B[3], NEXT, 3);
//   // if(B){printf("Block %i:%i\n",B->module, B->id);}
//   // printf("Next 4\n");
//   // B = Next(Units[20]->B[3], NEXT, 4);
//   // if(B){printf("Block %i:%i\n",B->module, B->id);}
//   // printf("Next 5\n");
//   // B = Next(Units[20]->B[3], NEXT, 5);
//   // if(B){printf("Block %i:%i\n",B->module, B->id);}
//   // printf("Next 6\n");
//   // B = Next(Units[20]->B[3], NEXT, 6);
//   // if(B){printf("Block %i:%i\n",B->module, B->id);}

//   set_level(DEBUG);

//   // U_Sw(20, 5)->state = 1;
//   //U_Sw(20, 4)->state = 1;

//   process(U_B(20, 11), 3);
//   process(U_B(22, 2), 3);
//   //process(U_B(20, 5), 3);
//   //process(U_B(20, 6), 3);

//   set_level(DEBUG);

//   printf("Baan.c block debug\n");
//   Algor_print_block_debug(U_B(20, 0)->Alg);
//   Algor_print_block_debug(U_B(20, 5)->Alg);
//   Algor_print_block_debug(U_B(20, 6)->Alg);
//   Algor_print_block_debug(U_B(20, 11)->Alg);

//   delay(5);

//   set_level(INFO);

//   // return 1;

//   for(int i = 0; i < unit_len;i++){
//     if(!Units[i])
//       continue;

//     for(int j = 0; j < Units[i]->block_len; j++){
//       if(!U_B(i,j))
//         continue;

//       U_B(i,j)->changed |= Block_Algor_Changed;
//     }
//   }

//   scan_All();

//   set_level(INFO);

//   // return 1;

//   if(_SYS->_Clients == 0){
//     printf("                   Waiting until for a client connects\n");
//   }
//   while(_SYS->_Clients == 0 && (_SYS->_STATE & STATE_RUN)){
//     usleep(1000000);
//   }
//   printf("Continue\n");

//   usleep(40000);

//   //pthread_t pt_scan_All;
//   //pthread_create(&pt_scan_All, NULL, scan_All_continiously, NULL);
//   // pthread_t pt_train_timers;
//   // pthread_create(&pt_train_timers, NULL, clear_train_timers, NULL);
//   pthread_t pt_train_simA;
//   pthread_create(&pt_train_simA, NULL, TRAIN_SIMA, NULL);
//   // pthread_t pt_train_simB;
//   // pthread_create(&pt_train_simB, NULL, TRAIN_SIMB, NULL);
//   usleep(100);

//   sem_wait(&AlgorQueueNoEmpty);
//   processAlgorQueue();

//   usleep(1000000);

//   Units[20]->B[8]->state = RESTRICTED;
//   Units[20]->B[4]->state = RESTRICTED;

//   WS_ShortCircuit();

//   loggerf(WARNING, "SYS-State: %x", _SYS->_STATE);

//   sem_init(&AlgorQueueNoEmpty, 1, 0);

//   while(_SYS->_STATE & STATE_RUN){
//     sem_wait(&AlgorQueueNoEmpty);
//     processAlgorQueue();

//     mutex_lock(&algor_mutex, "Algor Mutex");
//     //Notify clients
//     WS_trackUpdate(0);
//     WS_SwitchesUpdate(0);

//     mutex_unlock(&algor_mutex, "Algor Mutex");

//     usleep(1000);
//   }

//   logger("Stopping Argor",INFO);
//   //pthread_join(pt_scan_All,NULL);

//   logger("Free memory",INFO);
//   logger("FREE MEMORY",CRITICAL);
//   clear_Modules();
//   free_trains();
//   //pthread_join(tid[1],NULL);
//   //printf("STOP JOINED\n");
//   //pthread_join(tid[2],NULL);
//   //printf("Timer JOINED\n");
//   logger("Stopping Train Sim",INFO);
//   pthread_join(pt_train_simA,NULL);
//   // pthread_join(pt_train_simB,NULL);
//   //pthread_join(tid[4],NULL);
//   //printf("SimB JOINED\n");

//   logger("Stopping UART control",INFO);
//   //pthread_join(th_UART,NULL);

//   logger("Stopping Z21 control",INFO);
//   pthread_join(th_Z21, NULL);

//   logger("Stopping Websocket server",INFO);
//   pthread_join(th_web_server,NULL);
//   //procces(C_Adr(6,2,1),1);

//   //----- CLOSE THE UART -----
//   //pthread_exit(NULL);

  // _free(DeviceList);
// }
