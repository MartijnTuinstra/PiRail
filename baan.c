#define _BSD_SOURCE
// #define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
// #include <stdarg.h>
// #include <fcntl.h>
// #include <termios.h>
// #include <time.h>
// #include <sys/time.h>
// #include <math.h>
// #include <time.h>
#include <pthread.h>
#include <wiringPi.h>
#include <signal.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <string.h>
#include <errno.h>

#include "./lib/system.h"

#include "./lib/train_sim.h"
#include "./lib/websocket.h"
#include "./lib/status.h"
#include "./lib/Z21.h"
#include "./lib/com.h"

#include "./lib/rail.h"
#include "./lib/switch.h"
#include "./lib/signals.h"
#include "./lib/trains.h"

#include "./lib/modules.h"
#include "./lib/algorithm.h"

#include "./lib/pathfinding.h"

struct systemState * _SYS;

void main(){
	_SYS = (struct systemState *)malloc(sizeof(struct systemState));
	_SYS->_STATE = STATE_RUN;
	_SYS->_Clients = 0;
	_SYS->_COM_fd = -1;

	setbuf(stdout,NULL);
	setbuf(stderr,NULL);
	signal(SIGPIPE, SIG_IGN);
	srand(time(NULL));
	/*Starting wiringPi*/
		wiringPiSetup();

		pinMode(0, OUTPUT); //GPIO17
		pinMode(1, OUTPUT); //GPIO17
		digitalWrite(0,LOW);
		digitalWrite(1,LOW);
	/*Splash screen*/
		printf("\n\n                o  o   o\n");
		printf("            O  o  o\n");
		printf("        o oO  o\n");
		printf("      OoO\n");
		printf("     oOo ___             __________  ___________  ___________  __-----__\n");
	 printf("    _\\/__|_|_  _,o—o.,_  | |.\\/.| |  |         |  | EXPRESS |  ||_| |_||\n");
		printf("   [=      _|--|______|--|_|_/\\_|_|--|_________|--|_________|--|_______|\n");
		printf("   //o--=OOO-  ‘o’  ‘o’  ‘o’    ‘o’  ‘o’     ‘o’  ‘o’     ‘o’  ‘o’   ‘o’\n");

		printf("----------------------------------------------------------------------------\n");
		printf("|                                                                          |\n");
		printf("|                         RASPBERRY RAIL SOFTWARE                          |\n");
		printf("|                               is booting                                 |\n");
		printf("|                                                                          |\n");
		printf("----------------------------------------------------------------------------\n");
		printf("|                                                                          |\n");
	/**/
	/*Start web server*/
		printf("----------------------------------------------------------------------------\n");
		printf("|                                                                          |\n");
		printf("|                               Web server                                 |\n");
		printf("|                                                                          |\n");
		pthread_t thread_web_server;
		pthread_create(&thread_web_server, NULL, web_server, NULL);
		WS_init_Message_List();
		printf("|                           MessageBox Cleared                             |\n");
		printf("|                                                                          |\n");
		usleep(100000);
	/*Start UART*/
		printf("|                                  UART                                    |\n");
		printf("|                                                                          |\n");
		//Start UART port and recv handler
		pthread_t thread_UART;
		pthread_create(&thread_UART, NULL, UART, NULL);
		usleep(100000);
	/*Z21 Client*/
		printf("|                          Z21@%s:%i\t                   |\n",Z21_IP,Z21_PORT);
		printf("|                                                                          |\n");
		//Start UDP port
		pthread_t thread_Z21_client;
		//pthread_create(&thread_Z21_client, NULL, Z21_client, NULL);
		//Z21_client();
		usleep(100000);
	/*Enable web clients*/
		_SYS_change(STATE_Client_Accept,0);

	/*Search all blocks*/
		printf("|                              BLOCK LINKING                               |\n");
		printf("|                                                                          |\n");
		/*--             NEW LINKING              --*/
		/* new linking is just scanning for devices */
		/* then the blocks can be joined using a    */
		/* train riding over all modules            */

		if(DeviceList[0] != 0){
			//There are allready some device listed. Clear.
			memset(DeviceList,0,MAX_Devices);
		}

		//Send restart message
		COM_DevReset();

		usleep(200000); //Startup time of devices
		usleep(1000000);//Extra time to make sure it collects all info
		DeviceList[0] = 1;
		DeviceList[1] = 2;
		DeviceList[2] = 4;
		DeviceList[3] = 8;
		DeviceList[4] = 5;
		DeviceList[5] =10;
		DeviceList[6] =11;

		for(uint8_t i = 0;i<MAX_Devices;i++){
			if(DeviceList[i]){
				LoadModules(DeviceList[i]);
				printf("|                       UART Module %i\t found                             |\n",DeviceList[i]);
			}
		}

		_SYS_change(STATE_Modules_Loaded,1);

		JoinModules();


		setup_JSON((int [4]){1,8,4,2},(int *)0,4,0);

		Connect_Segments();
		

		//clear_Modules();


		//Initialising two link object with empty blocks
		/*struct link LINK;
		struct link LINK2;
		struct link LINK;
		struct link LINK2;
		LINK.Adr1 = EMPTY_BL;
		LINK.Adr2 = EMPTY_BL;
		LINK.Adr3 = EMPTY_BL;
		LINK.Adr4 = EMPTY_BL;
		LINK2.Adr1 = EMPTY_BL;
		LINK2.Adr2 = EMPTY_BL;
		int nr_Modules = 0;
		usleep(100000);
		digitalWrite(1,HIGH);

		int setup[MAX_Modules] = {1,8,4,2,0};
		int setup2[5] = {11,6,7,0};

		for(int i = 0;i<4;i++){
			#ifdef En_UART
				char Line_Data[25];
				memset(Line_Data,0,25);
				int L = COM_Recv(Line_Data);
				if(L == 3 && Line_Data[0] == 0 && (Line_Data[1] & 0xF) == 0){
					printf("|                       UART Module %i\t found                             |\n",Line_Data[2]);
					//printf("Module %i\tfound\n",setup[i]);
					LINK = Modules(Line_Data[2],LINK);
					if(!Adr_Comp2(LINK.Adr3,EMPTY_BL)){
						printf("Branch needed\n");
						LINK2.Adr1 = LINK.Adr3;
						LINK2.Adr2 = LINK.Adr4;
					}

					setup[i] = Line_Data[2];
					List_of_Modules[nr_Modules++] = Line_Data[2];
				}else{
					printf("Wrong data length recieved: %i\n",L);
					for(int i = 0;i<L;i++){
						printf("[%i]",Line_Data[i]);
					}
					printf("\n");
				}
			#endif
			#ifndef En_UART
				printf("|                            Module %i\t found                             |\n",setup[i]);
				//printf("Module %i\tfound\n",setup[i]);
				LINK = Modules(setup[i],LINK);
				if(!Adr_Comp2(LINK.Adr3,EMPTY_BL)){
					printf("Branch needed\n");
					printf("%i==%i\t%i==%i\t%c==%c\n",LINK.Adr3.Module,EMPTY_BL.Module,LINK.Adr3.Adr,EMPTY_BL.Adr,LINK.Adr3.type,EMPTY_BL.type);
					LINK2.Adr1 = LINK.Adr3;
					LINK2.Adr2 = LINK.Adr4;
				}
			#endif
		}

		if(!Adr_Comp2(LINK2.Adr1,EMPTY_BL)){
			printf("|                                                                          |\n");
			printf("|                               Branch one                                 |\n");
		}

		for(int i = 0;i<0;i++){
			printf("|                            Module %i\t found                             |\n",setup2[i]);
			LINK2 = Modules(setup2[i],LINK2);
		}
		Connect_Segments();
		setup_JSON(setup,setup2,4,0);
		usleep(1000000);
		/**/
	/*Loading Trains*/
		printf("|                                                                          |\n");
		printf("|                             Loading trains                               |\n");
		printf("|                                                                          |\n");

		init_trains();

	printf("|                                                                          |\n");
	printf("----------------------------------------------------------------------------\n\n");
	printf("                              Initialization Done\n");
	printf("          To complete the setup please load or connect the modules          \n");

	//Set all Switches and Signals to known positions
	scan_All();
	//procces(blocks2[2][0],1);
	//procces(blocks2[2][1],1);
	//procces(blocks2[2][2],1);
	//procces(blocks2[2][3],1);

	delay(5);

	/*COM test*/
		//COM_Recv();

		/*
		struct COM_t{
			char Adr;
			char Length;
			char Opcode;
			char Data[32];
		};


		struct COM_t C;
		C.Adr = 1;
		C.Length = 4;
		C.Opcode = 14;
		memset(C.Data,0,32);
		C.Data[0] = 89;
		C.Data[1] = 74;
		C.Data[2] = 62;
		C.Data[3] = 98;

		COM_Send(C);
		delay(1100);
		C.Adr = 0xFF;
		COM_Send(C);
		delay(1100);
		C.Adr = 4;
		COM_Send(C);
		delay(1100);
		C.Adr = 8;

		//Read blocks from address 8
		COM_Send(C);
		delay(1100);
		C.Adr = 2;
		C.Opcode = 6;
		C.Length = 0;
		COM_Send(C);
		char COM_data[20];
		memset(COM_data,0,20);
		COM_Recv(COM_data);
		printf("Data recieved:\n");
		printf("Address: 0x%02x\n",COM_data[0]);
		printf("Opcode:  0x%01x\n",(COM_data[1] & 0b1111));
		char length;
		printf("Length:  0x%01x\n",length = (COM_data[1] >> 4));
		for(int i = 0;i<length;i++){
			printf(BYTE_TO_BINARY_PATTERN,BYTE_TO_BINARY(COM_data[i+2]));
		}
		printf("\n");
	  digitalWrite(0,LOW);/**/


	/*Pathfinding test

		struct Sw_PATH2 * (Route)[MAX_ROUTE] = {NULL};
		//memset(Route,NULL,MAX_ROUTE);

		//train_set_path(trains[4],A,B);
		pathFinding2(Units[4]->B[23],stations[1]->Blocks[0],Route);*/

	//Done with setup when there is at least one client
	if(_SYS->_Clients == 0){
		printf("                   Waiting until for a client connects\n");
	}
	while(_SYS->_Clients == 0){
		usleep(1000000);
	}

	usleep(400000);

	pthread_t tid[10];
	//throw_switch(Switch[5][2][1]);
	//throw_switch(Switch[6][3][1]);

	/*Timers Test code*/
		/*printf("Create clear thread\n");*/
		//pthread_create(&tid[0], NULL, clear_timers, NULL);
		/*
		printf("Create timer threads\n");
		for(int j = 0;j<100;j++){
			if(j == 3){
				status_add(4,"[0,5,2,3,6,3,4,6]");
			}else if(j == 47){
				status_rem(0,4);
			}
			printf("J:%i\t",j);
			create_timer();
			usleep(75000);
		}
		int i = 0;
		int k = 0;
		while(1){
			if(timers[i] != 0){
				k = 1;
			}
			i++;
			if(i==MAX_TIMERS){
				if(k == 0){
					break;
				}
				k = 0;
				i = 0;
			}
		}
		stop = 1;
		pthread_join(tid[0],NULL);
		pthread_join(tid[1],NULL);
		*/
	/*Time test code*/
		/*
		clock_t t;
		t = clock();
		usleep(1000000);
		t = clock() - t;
		printf ("It took me %d clicks (%f seconds).\n",t,((float)t)/CLOCKS_PER_SEC);
		struct timeval  tv1, tv2;
		gettimeofday(&tv1, NULL);
		usleep(1000000);
		gettimeofday(&tv2, NULL);

		printf ("Total time = %f seconds\n",
	         (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
	         (double) (tv2.tv_sec - tv1.tv_sec));*/

	printf("Test octal: 023 = %i\n",023);

	//usleep(5000000);


	printf("Creating Threads\n");
	pthread_create(&tid[0], NULL, scan_All_continiously, NULL);
	
	pthread_create(&tid[2], NULL, clear_train_timers, NULL);

	pthread_create(&tid[3], NULL, TRAIN_SIMA, NULL);
	usleep(500000);
	//pthread_create(&tid[4], NULL, TRAIN_SIMB, NULL);
  //pthread_create(&tid[5], NULL, TRAIN_SIMC, NULL);
  //pthread_create(&tid[6], NULL, TRAIN_SIMD, NULL);

	usleep(10000000);

	WS_ShortCircuit();


	//COM_Recv();

	//Web_Emergency_Stop(ACTIVATE);
	/*
	usleep(22000000);

	pthread_mutex_lock(&mutex_lockB);
	//throw_switch(Switch[4][6][1]);
	//throw_switch(Switch[4][6][2]);
	pthread_mutex_unlock(&mutex_lockB);
	*/
	pthread_join(tid[0],NULL);
	printf("Magic JOINED\n");
	//pthread_join(tid[1],NULL);
	//printf("STOP JOINED\n");
	//pthread_join(tid[2],NULL);
	//printf("Timer JOINED\n");
	pthread_join(tid[3],NULL);
	printf("SimA JOINED\n");
	//pthread_join(tid[4],NULL);
	//printf("SimB JOINED\n");
	pthread_join(thread_UART,NULL);
	pthread_join(thread_web_server,NULL);
	//procces(C_Adr(6,2,1),1);

  //----- CLOSE THE UART -----
	close(_SYS->_COM_fd);

	printf("STOPPED");
	//pthread_exit(NULL);
}


void _SYS_change(int STATE,char send){
	printf("_SYS_change %x\n",_SYS->_STATE);
	//printf("%x & %x = %i",_SYS->_STATE &);
	if(_SYS->_STATE & STATE && send & 0x02){
		_SYS->_STATE &= 0xFFFF ^ STATE;
	}else if(!(_SYS->_STATE & STATE)){
		_SYS->_STATE |= STATE;
	}
	printf("_SYS_change %x\n",_SYS->_STATE);

	if(send & 0x01){
		char data[5];
		data[0] = WSopc_Service_State;
		data[1] = _SYS->_STATE >> 8;
		data[2] = _SYS->_STATE & 0xFF;
		send_all(data,3,WS_Flag_Admin || WS_Flag_Track);
	}
}