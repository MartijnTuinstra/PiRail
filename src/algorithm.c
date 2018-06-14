#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#include "./../lib/system.h"

#include "./../lib/algorithm.h"



#include "./../lib/rail.h"
#include "./../lib/trains.h"
#include "./../lib/switch.h"
#include "./../lib/signals.h"

#include "./../lib/modules.h"
#include "./../lib/com.h"
#include "./../lib/status.h"

pthread_mutex_t mutex_lockA;

void * scan_All_continiously(){
	while(_SYS->_STATE & STATE_RUN){
		//printf("\n\n\n");
		clock_t t;
		t = clock();
		pthread_mutex_lock(&mutex_lockA);
		#ifdef En_UART
		for(int i = 0;i<strlen(List_of_Modules);i++){
			printf("R%i ",List_of_Modules[i]);
			struct COM_t C;
			memset(C.Data,0,32);
			C.Adr = List_of_Modules[i];
			C.Opcode = 6;
			C.Length = 0;

			pthread_mutex_lock(&mutex_UART);
			COM_Send(C);
			char COM_data[20];
			memset(COM_data,0,20);
			COM_Recv(COM_data);
			COM_Parse(COM_data);
			pthread_mutex_unlock(&mutex_UART);
			usleep(10);
		}
		printf("\n");
		#endif
		_Bool debug;
		for(int i = 0;i<MAX_Modules;i++){
			if(Units[i]){
				for(int j = 0;j<=Units[i]->B_L;j++){
					if(Units[i]->B[j]){
						//printf("%i:%i\n",i,j);
						procces(Units[i]->B[j],0);
					}
				}
			}
		}
		WS_trackUpdate(0);
		WS_SwitchesUpdate(0);
		pthread_mutex_unlock(&mutex_lockA);
		t = clock() - t;
		//printf ("It took me %d clicks (%f seconds).\n",t,((float)t)/CLOCKS_PER_SEC);
		//printf("\n\n\n\n\n\n");

		procces_accessoire();

		//FILE *data;
		//data = fopen("data.txt", "a");
		//fprintf(data,"%d\n",t);
		//fclose(data);

		usleep(1000000);
	}
}

void scan_All(){
	pthread_mutex_lock(&mutex_lockA);
	for(int i = 0;i<MAX_Modules;i++){
		if(Units[i]){
			for(int j = 0;j<=Units[i]->B_L;j++){
				if(Units[i]->B[j]){
					//printf("%i:%i\n",i,j);
					procces(Units[i]->B[j],0);
				}
			}
		}
	}
	// COM_change_A_signal(4);
	// COM_change_switch(4);
	WS_trackUpdate(0);
	pthread_mutex_unlock(&mutex_lockA);
}

void change_block_state2(struct procces_block * A,int State){
	if(!A->blocked){
		for(int i = 0;i<A->length;i++){
			if(A->B[i]->state != State){
				A->B[i]->change = 1;
				A->B[i]->state = State;
			}
		}
	}else{
		for(int i = 0;i<A->length;i++){
			if(A->B[i]->blocked){
				break;
			}
			if(A->B[i]->state != State){
				A->B[i]->change = 1;
				A->B[i]->state = State;
			}
		}
	}
}

void procces(struct Seg * B,int debug){
	if(B->type == 'T'){
		if(!B->blocked){
			B->train = 0;
		}
		if(debug){
			struct Seg *BA = B;
			if(BA->train != 0){
				printf("ID: %i\t%c%i:%i\n",BA->train,BA->Module,BA->id);
			}
			//printf("\t\t          \tA  %i:%i:%i;T%iR%i",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->train,BA->dir);
			//if(BA->blocked){
			//	printf("B");
			//}
			//printf("\n");
		}
	}
	else{
		//printf("B\n");

		struct procces_block BPPP,BPP,BP,BA,BN,BNN,BNNN;
		BA.B[0] = B;
		BA.blocked = B->blocked;
		BA.length = 1;

		struct Seg * bl[8] = {0};
		struct Seg * bpl[6] = {0};
		struct Seg * bp,*bpp,*bppp;
		struct Seg * Bl;
		bl[0] = B;
		bpl[0] = B;
		Bl = B;
		int i = 0;
		int p = 0;
		struct Seg * tB;
		//Get blocks in avalable path
		int q = 4;

		//Get the 3 next blocks
		for(i = 0;(i+1)<q;i){
			if(bl[i]->type != 'T'){ //If block has no contact points
				Bl = bl[i];
			}
			i++;
			//printf("i%i\t%i:%i:%c\n",i-1,bl[i-1]->Module,bl[i-1]->id,bl[i-1]->type);
			struct Rail_link A;
			if(dir_Comp(B,Bl)){
				A = NADR2(Bl);
			}else{
				A = PADR2(Bl);
			}
			if(A.type == 0){
				//printf("A.type == 0\n");
				q = i;
				break;
			}else if(A.type == 's' || A.type == 'S' || A.type == 'm' || A.type == 'M'){
				//printf("%i:%i Check_switch %i:%i\n",Bl->Module,Bl->id,((Switch *)A.ptr)->Module,((Switch *)A.ptr)->id);
				if(!check_Switch(Bl,0,FALSE)){
					//printf("WSw\n");
					q = i;
					break;
				}
			}
			bl[i] = Next2(B,i);
			//printf(".%i<%i\n",i,q);
			//printf("%i  %c%i:%i\t",i,bl[i]->type,bl[i]->Module,bl[i]->id);
			if(i > 1 && bl[i-1]->type == 'T' && bl[i]->type == 'T' && !Block_cmp(bl[i-1],bl[i])){
				//printf("Double T rail\n");
				q++;
			}else if(!bl[i]){
				q = i;
				break;
			}
		}
		i--;

		char r = 4;
		//Get the 3 previous blocks
		for(p = 0;p<=r;){
			if(bpl[p]->type != 'T'){ //If block has no contact points
				Bl = bpl[p];
			}
			p++;
			//printf("i%i\t%i:%i:%c\n",p-1,bpl[p-1]->Module,bpl[p-1]->id,bpl[p-1]->type);
			struct Rail_link A;
			if(dir_Comp(B,Bl)){
				A = PADR2(Bl);
			}else{
				A = NADR2(Bl);
			}
			if(A.type == 0){
				//printf("A.type == 0\n");
				r = p;
				break;
			}else if(A.type == 's' || A.type == 'S' || A.type == 'm' || A.type == 'M'){
				//printf("Check_switch %i:%i\n",Bl->Module,Bl->id);
				if(!check_Switch(Bl,1,FALSE)){
					//printf("WSw\n");
					r = p;
					break;
				}
			}
			bpl[p] = Prev2(B,p);
			
			if(!bpl[p]){
				r = p-1;
				break;
			}
			//printf(".%i<%i\n",p,r);
			//printf("%i  %c%i:%i\t",p,bpl[p]->type,bpl[p]->Module,bpl[p]->id);
			if(p > 1 && bpl[p-1]->type == 'T' && bpl[p]->type == 'T' && !Block_cmp(bpl[p-1],bpl[p])){
				//printf("Double T rail\n");
				r++;
			}
			if(p == r && bpl[p]->type == 'T'){
				r++;
			}
		}
		p--;

		i = 1;
		p = 1;
		int j = 0;
		int k = -1;
		int l = -1;

		//Clear pointer
		BPPP.B[0] = NULL;BPPP.B[1] = NULL;BPPP.B[2] = NULL;BPPP.B[3] = NULL;BPPP.B[4] = NULL;
		BPP.B[0] = NULL;BPP.B[1] = NULL;BPP.B[2] = NULL;BPP.B[3] = NULL;BPP.B[4] = NULL;
		BP.B[0] = NULL;BP.B[1] = NULL;BP.B[2] = NULL;BP.B[3] = NULL;BP.B[4] = NULL;
		BN.B[0] = NULL;BN.B[1] = NULL;BN.B[2] = NULL;BN.B[3] = NULL;BN.B[4] = NULL;
		BNN.B[0] = NULL;BNN.B[1] = NULL;BNN.B[2] = NULL;BNN.B[3] = NULL;BNN.B[4] = NULL;
		BNNN.B[0] = NULL;BNNN.B[1] = NULL;BNNN.B[2] = NULL;BNNN.B[3] = NULL;BNNN.B[4] = NULL;
		//Clear data
		BPPP.blocked = 0;BPP.blocked = 0;BP.blocked = 0;BN.blocked = 0;BNN.blocked = 0;BNNN.blocked = 0;
		BPPP.length = 0;BPP.length = 0;BP.length = 0;BN.length = 0;BNN.length = 0;BNNN.length = 0;

		//Assign next pointers
		//k == number of block ahead
		for(i;i<q;i++){
			if(i > 1 && bl[i-1]->type == 'T' && bl[i]->type == 'T'){
				j++;
			}else{
				k++;
				j = 0;
			}

			if(k == 0 && bl[i]){
				BN.B[j] = bl[i];
				BN.blocked |= BN.B[j]->blocked;
				BN.length = j+1;
			}else if(k == 1 && bl[i]){
				BNN.B[j] = bl[i];
				BNN.blocked |= BNN.B[j]->blocked;
				BNN.length = j+1;
			}else if(k == 2 && bl[i]){
				BNNN.B[j] = bl[i];
				BNNN.blocked |= BNNN.B[j]->blocked;
				BNNN.length = j+1;
			}
		}
		k++;
		i = k;


		//Assign prev pointers
		//p == number of block backward
		for(p;p<=r;p++){
			//printf("p%i/%i: %c%i:%i\n",p,r,bpl[p]->type,bpl[p]->Module,bpl[p]->id);
			if(p > 1 && bpl[p-1]->type == 'T' && bpl[p]->type == 'T'){
				//printf("%c%i:%i==%c%i:%i\n",bpl[p-1]->type,bpl[p-1]->Module,bpl[p-1]->id,bpl[p]->type,bpl[p]->Module,bpl[p]->id);
				j++;
			}else{
				l++;
				j = 0;
			}

			if(l == 0 && bpl[p]){
				BP.B[j] = bpl[p];
				BP.blocked |= BP.B[j]->blocked;
				BP.length++;
			}else if(l == 1 && bpl[p]){
				BPP.B[j] = bpl[p];
				BPP.blocked |= BPP.B[j]->blocked;
				BPP.length++;
			}else if(l == 2 && bpl[p]){
				BPPP.B[j] = bpl[p];
				BPPP.blocked |= BPPP.B[j]->blocked;
				BPPP.length++;
			}
		}
		l++;
		p = l;
		/*Debug info*/
			if(BA.B[0]->train != 0){
				//printf("ID: %i\t%i:%i:%i\n",BA->train,BA->Adr.M,BA->Adr.B,BA->Adr.S);
			}
			if(debug || (B->Module == 4 && B->id == 12)){
				if(p > 2){
					printf("PPP ");
					for(int i = 1;i>=0;i--){
						if(BPPP.B[i]){
							printf("%02i:%02i",BPPP.B[i]->Module,BPPP.B[i]->id);
							if(BPPP.B[i]->blocked){
								printf("B  ");
							}else{
								printf("   ");
							}
						}else{
							printf("        ");
						}
					}
				}else{
					printf("                    ");
				}
				if(p > 1){
					printf("PP ");
					for(int i = 1;i>=0;i--){
						if(BPP.B[i]){
							printf("%02i:%02i",BPP.B[i]->Module,BPP.B[i]->id);
							if(BPP.B[i]->blocked){
								printf("B  ");
							}else{
								printf("   ");
							}
						}else{
							printf("        ");
						}
					}
				}else{
					printf("                   ");
				}
				if(p > 0){
					printf("P ");
					for(int i = 1;i>=0;i--){
						if(BP.B[i]){
							printf("%02i:%02i",BP.B[i]->Module,BP.B[i]->id);
							if(BP.B[i]->blocked){
								printf("B  ");
							}else{
								printf("   ");
							}
						}else{
							printf("        ");
						}
					}
				}else{
					printf("                  ");
				}
				printf("A%i %c%02i:%02i;T%iD%iS%i",BA.length,BA.B[0]->type,BA.B[0]->Module,BA.B[0]->id,BA.B[0]->train,BA.B[0]->dir,BA.B[0]->state);
				if(BA.B[0]->blocked){
					printf("B");
				}
				printf("\t");
				if(i > 0){
					printf("N ");
					for(int i = 0;i<2;i++){
						if(BN.B[i]){
							printf("%02i:%02i",BN.B[i]->Module,BN.B[i]->id);
							if(BN.B[i]->blocked){
								printf("B  ");
							}else{
								printf("   ");
							}
						}else{
							printf("        ");
						}
					}
				}if(i > 1){
					printf("NN ");
					for(int i = 0;i<2;i++){
						if(BNN.B[i]){
							printf("%02i:%02i",BNN.B[i]->Module,BNN.B[i]->id);
							if(BNN.B[i]->blocked){
								printf("B  ");
							}else{
								printf("   ");
							}
						}else{
							printf("        ");
						}
					}
				}
				if(i > 2){
					printf("NNN ");
					for(int i = 0;i<2;i++){
						if(BNNN.B[i]){
							printf("%02i:%02i",BNNN.B[i]->Module,BNNN.B[i]->id);
							if(BNNN.B[i]->blocked){
								printf("B  ");
							}else{
								printf("   ");
							}
						}else{
							printf("        ");
						}
					}
				}
				//if(i == 0){
				printf("\n");
				//}
				if(BA.B[0]->PSi || BA.B[0]->NSi){
					if(BA.B[0]->PSi){
						printf("\t\t\t\t\t\t%i  ",BA.B[0]->PSi->id);
						uint8_t state = BA.B[0]->PSi->state & 0x7F;
						if(state == SIG_RED  )printf("RED");
						if(state == SIG_AMBER)printf("AMBER");
						if(state == SIG_GREEN)printf("GREEN");
						if(state == SIG_RESTRICTED)printf("RESTR");
						printf("\t\t");
					}else{
						printf("\t\t\t\t\t\t\t\t\t");
					}
					if(BA.B[0]->NSi){
						printf("\t%i  ",BA.B[0]->NSi->id);
						uint8_t state = BA.B[0]->NSi->state & 0x7F;
						if(state == SIG_RED  )printf("RED");
						if(state == SIG_AMBER)printf("AMBER");
						if(state == SIG_GREEN)printf("GREEN");
						if(state == SIG_RESTRICTED)printf("RESTR");
					}
					printf("\n\n");
				}
			}
		/**/


		//------------------------------------------------------------------------------------------ roadmap: more efficient scanning of the block. skip the blocks that are not changed/blocked and there neighbours are also not blocked
		//SPEEDUP function / if all blocks are not blocked skip!!
		/*if(k > 0 && !BA.blocked && !BN.blocked){
			if(p > 0 && !BP.blocked){
				return;
			}else if(p == 0){
				return;
			}
		}*/

		/*Train ID following*/
			if(!BA.blocked && BA.B[0]->train != 0){
				//Reset
				BA.B[0]->train = 0;
			}else if(BA.blocked && BA.B[0]->train != 0 && train_link[BA.B[0]->train] && !train_link[BA.B[0]->train]->Block){
				 train_link[BA.B[0]->train]->Block = BA.B[0];
			}
			if(k > 0 && BA.blocked && !BP.blocked && BN.blocked && BN.B[0]->train && !BA.B[0]->train){
				//REVERSED
				BA.B[0]->dir ^= 0b100;
				BN.B[0]->dir ^= 0b100;
			}
			else if(p > 0 && k > 0 && BA.blocked && BA.B[0]->train == 0 && BN.B[0]->train == 0 && BP.B[0]->train == 0){
				//NEW TRAIN
				BA.B[0]->train = ++bTrain;
				//Create a message for WebSocket
				WS_NewTrain(bTrain,BA.B[0]->Module,BA.B[0]->id);
			}
			else if(p > 0 && k > 0 && BN.blocked && BP.blocked && BN.B[0]->train == BP.B[0]->train){
				//A train has split
				WS_TrainSplit(BN.B[0]->train,BP.B[0]->Module,BP.B[0]->id,BN.B[0]->Module,BN.B[0]->id);
			}
			if(p > 0 && BP.blocked && BA.blocked && BA.B[0]->train == 0 && BP.B[0]->train != 0){
				BA.B[0]->train = BP.B[0]->train;
				if(train_link[BA.B[0]->train])
					train_link[BA.B[0]->train]->Block = BA.B[0];
			}
			if(k > 0 && BN.B[0]->type == 'T' && BN.blocked){
				if(BN.B[0]->train == 0 && BN.B[0]->blocked && BA.blocked && BA.B[0]->train != 0){
					BN.B[0]->train = BA.B[0]->train;
					if(train_link[BN.B[0]->train])
						train_link[BN.B[0]->train]->Block = BN.B[0];
				}else if(BN.length > 1){
					for(int a = 1;a<BN.length;a++){
						if(BN.B[a-1]->blocked && BN.B[a]->blocked && BN.B[a]->train == 0 && BN.B[a-1]->train != 0){
							BN.B[a]->train = BN.B[a-1]->train;
							if(train_link[BN.B[a]->train])
								train_link[BN.B[a]->train]->Block = BN.B[a];
							break;
						}
					}
				}
			}
		/**/
		/**/
		/*Check switch*/
			//
			int New_Switch = 0;
			struct Rail_link NAdr,NNAdr;

			NAdr = NADR2(BA.B[BA.length - 1]);
			if(k > 0){
				NNAdr = NADR2(BN.B[BN.length - 1]);
			}

			if((NNAdr.type == 's' || NNAdr.type == 'S' || NNAdr.type == 'm' || NNAdr.type == 'M') && BA.blocked){
				//There is a switch after the next block
				if(BA.B[0]->train != 0 && train_link[BA.B[0]->train] && train_link[BA.B[0]->train]->Destination){
					//If train has a route
					if(check_Switch_state(NNAdr)){
						//Switch is free to use
						New_Switch = 2;
						printf("Free switch ahead (OnRoute %i:%i)\n",BA.B[0]->Module,BA.B[0]->id);
						if(!free_Route_Switch(BN.B[BN.length - 1],0,train_link[BA.B[0]->train])){
							printf("FAILED to set switch according Route\n");
							New_Switch = 0;
						}
					}
				}else{
					//No route
					if(check_Switch_state(NNAdr)){
						//Switch is free to use
						New_Switch = 2;
						printf("Free switch ahead %i:%i\n",BA.B[0]->Module,BA.B[0]->id);
						if(!BN.B[BN.length - 1]){
							printf("Check_switch but no block R\n");
						}
						if(!check_Switch(BN.B[BN.length - 1],0,TRUE)){
							//The switch is in the wrong state / position
							printf("Check Switch\n");
							if(!free_Switch(BN.B[BN.length - 1],0)){
								New_Switch = 0;
				}}}}
			}else if((NAdr.type == 's' || NAdr.type == 'S' || NAdr.type == 'm' || NAdr.type == 'M') && BA.blocked){
				//There is a switch after the current block
				if(BA.B[0]->train != 0 && train_link[BA.B[0]->train] && train_link[BA.B[0]->train]->Destination){
					//If train has a route
					if(check_Switch_state(NAdr)){
						//Switch is free to use
						New_Switch = 1;
						printf("Free switch ahead (OnRoute %i:%i)\n",BA.B[0]->Module,BA.B[0]->id);
						if(!free_Route_Switch(BA.B[BA.length - 1],0,train_link[BA.B[0]->train])){
							New_Switch = 0;
						}
					}
				}else{
					//No route
					if(check_Switch_state(NNAdr)){
						New_Switch = 1;
						//Switch is free to use
						printf("Free switch ahead %i:%i\n",BA.B[0]->Module,BA.B[0]->id);
						if(!check_Switch(BN.B[BN.length - 1],0,TRUE)){
							//The switch is in the wrong state / position
							printf("Check Switch\n");
							if(!free_Switch(BN.B[BN.length - 1],0)){
								printf("FAILED to set switch according Route\tSTOPPING TRAIN\n");
								New_Switch = 0;
				}}}}
			}

			//Extend if switches are thrown
			if(New_Switch > 0){
				printf("Switch thrown");
				if(k < 1){
					printf("BN  NEEDED\t");
					BN.B[0] = Next2(BA.B[0],1+BN.length);
					if(BN.B[0]){
						BN.length++;
						BN.blocked |= BN.B[0]->blocked;
						if(BN.B[0]->type == 'T'){
							BN.B[1] = Next2(BA.B[0],1+BN.length);
							if(BN.B[1]->type == 'T'){
								BN.length++;
								BN.blocked |= BN.B[1]->blocked;
							}else{
								BN.B[1] = NULL;
							}
						}
						k++;
					}
				}
				if(k < 2){
					printf("BNN  NEEDED\t");
					BNN.B[0] = Next2(BA.B[0],1+BN.length+BNN.length);
					if(BNN.B[0]){
						BNN.length++;
						BNN.blocked |= BNN.B[0]->blocked;
						if(BNN.B[0]->type == 'T'){
							BNN.B[1] = Next2(BA.B[0],1+BN.length+BNN.length);
							if(BNN.B[1]->type == 'T'){
								BNN.length++;
								BNN.blocked |= BNN.B[1]->blocked;
							}else{
								BNN.B[1] = NULL;
							}
						}
						k++;
					}
				}
				if(k < 3){
					BNNN.B[0] = Next2(BA.B[0],1+BN.length+BNN.length+BNNN.length);
					if(BNNN.B[0]){
						BNNN.length++;
						BNNN.blocked |= BNNN.B[0]->blocked;
						if(BNNN.B[0]->type == 'T'){
							BNNN.B[1] = Next2(BA.B[0],1+BN.length+BNN.length+BNNN.length);
							if(BNNN.B[1]->type == 'T'){
								BNNN.length++;
								BNNN.blocked |= BNNN.B[1]->blocked;
							}else{
								BNNN.B[1] = NULL;
							}
						}
						k++;
					}
				}
			}

			if(New_Switch == 1){
				change_block_state2(&BN,RESERVED);
			}else if(New_Switch == 2){
				change_block_state2(&BNN,RESERVED);
			}

			/*

				if((NAdr.type == 's' || NAdr.type == 'S' || NAdr.type == 'm' || NAdr.type == 'M') && BA.blocked){
					if(((NAdr.type == 's' || NAdr.type == 'S') && NAdr.Sw->Detection_Block && NAdr.Sw->Detection_Block->state != RESERVED) ||
						 ((NAdr.type == 'm' || NAdr.type == 'M') &&  NAdr.M->Detection_Block &&  NAdr.M->Detection_Block->state != RESERVED)){
						if(!check_Switch(BN.B[BN.length - 1],0,TRUE)){
							//The switch is not reserved and is in the wrong position
							printf("Check Switch\n");
							if(free_Switch2(BN.B[BN.length - 1],0)){
								printf("Freed");

								printf("BNN RESERVED\n");
								change_block_state2(&BNN,RESERVED);
							}
						}
						else{
							//If switch is in correct position but is not reserved
							change_block_state2(&BNN,RESERVED);
						}
					}
				}
			}*/
		/**/
		/**/
		/*Reverse block after one or two zero-blocks*/
			//If the next block is reversed, and not blocked
			if(i > 0 && BA.blocked && BN.B[0] && BN.B[0]->type != 'T' && !BN.blocked && !dir_Comp(BA.B[0],BN.B[0])){
				BN.B[0]->dir ^= 0b100;
			}

			//Reverse one block in advance
			if(i > 1 && BA.blocked && BNN.B[0] && BNN.B[0]->type != 'T' && !dir_Comp(BA.B[0],BNN.B[0]) &&
							(BN.B[0]->type == 'T' || !(dir_Comp(BA.B[0],BN.B[0]) == dir_Comp(BN.B[0],BNN.B[0])))){

				printf("Reverse in advance 1\n");
				if(BNN.B[0]->type == 'S'){ //Reverse whole platform if it is one
					printf("Whole platform\n");
					for(int a = 0;a<8;a++){
						if(BNN.B[0]->Station->Blocks[a]){
							if(BNN.B[0]->Station->Blocks[a]->blocked){
								break;
							}
							BNN.B[0]->Station->Blocks[a]->dir ^= 0b100;
						}
					}
				}else{
					printf("Block %x\n",BNN.B[0]);
					BNN.B[0]->dir ^= 0b100;
				}
			}
		/**/
		/**/
		/*State coloring*/
			//Block behind train (blocked) becomes RED
			//Second block behind trin becomes AMBER
			//After that GREEN

			//Double 0-block counts as one block

			//If current block is blocked and previous block is free
			if(BA.B[0]->Module == 5 && (BA.B[0]->id == 2 || BA.B[0]->id == 3)){
				int karamba = 0;
			}
			if(p > 0 && BA.blocked && !BP.blocked){
				change_block_state2(&BP,RED);
				if(p > 2)
					change_block_state2(&BPPP,GREEN);
				if(p > 1)
					change_block_state2(&BPP,AMBER);
			}
			else if(i > 0 && !BA.blocked && BN.blocked && BN.B[0]->type == 'T'){
				change_block_state2(&BA,RED);
				if(p > 0)
					change_block_state2(&BP,AMBER);
				if(p > 1)
					change_block_state2(&BPP,GREEN);
			}
			else if(p > 1 && k > 1 && !BA.blocked && !BN.blocked && !BP.blocked && !BNN.blocked && !BPP.blocked){
				change_block_state2(&BA,GREEN);
				if(p > 2 && !BPPP.blocked){
					change_block_state2(&BP,GREEN);
				}
				if(k > 2 && !BNNN.blocked){
					change_block_state2(&BN,GREEN);
				}
			}
		/**/
		/**/
		/*Signals*/
			//If a signal is at Next end and BN exists
			if(BA.B[0]->NSi && k > 0){
				//Wrong Switch
				//if current block is in forward and there are blocked switches
				// or if the block is in the wrong direction (reverse)
				if(((BA.B[0]->dir == 0 || BA.B[0]->dir == 1 || BA.B[0]->dir == 2) && !check_Switch(BA.B[0],0,TRUE)) || (BA.B[0]->dir == 4 || BA.B[0]->dir == 5 || BA.B[0]->dir == 6)){
					set_signal(BA.B[0]->NSi,SIG_RED);
				}

				if(!(BA.B[0]->dir == 4 || BA.B[0]->dir == 5 || BA.B[0]->dir == 6) && check_Switch(BA.B[0],0,TRUE) && i > 0){
					//Next block is RED/Blocked
					if(BN.blocked || BN.B[0]->state == RED){
						set_signal(BA.B[0]->NSi,SIG_RED);
					}else if(BN.B[0]->state == PARKED){
						set_signal(BA.B[0]->NSi,SIG_RESTRICTED); //Flashing RED
					}else if(BN.B[0]->state == AMBER){	//Next block AMBER
						set_signal(BA.B[0]->NSi,SIG_AMBER);
					}else{ // //Next block AMBER	if(BN->state == GREEN)
						set_signal(BA.B[0]->NSi,SIG_GREEN);
					}
				}
			}
			else if(BA.B[0]->NSi && k == 0){ //If the track stops due to switches, set it to RED / DANGER
				set_signal(BA.B[0]->NSi,SIG_RED);
			}

			//If a signal is at Prev side and BP exists
			if(BA.B[0]->PSi && p > 0){
				//printf("Signal at %i:%i\n",BA.B[0]->Module,BA.B[0]->id);
				//printf("check_Switch: %i\n",check_Switch(BA.B[0],0,TRUE));
				//printf("Signal at %i:%i:%i\t0x%x\n",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->signals);
				//if current block is in reverse and there are blocked switches
				// or if the block is in the wrong direction (forward)
				if(((BA.B[0]->dir == 4 || BA.B[0]->dir == 5 || BA.B[0]->dir == 6) && !check_Switch(BA.B[0],0,TRUE)) || (BA.B[0]->dir == 0 || BA.B[0]->dir == 1 || BA.B[0]->dir == 2)){
					set_signal(BA.B[0]->PSi,SIG_RED);
					//printf("%i:%i:%i\tRed signal R2\n",BA->Adr.M,BA->Adr.B,BA->Adr.S);
				}

				if(!(BA.B[0]->dir == 0 || BA.B[0]->dir == 1 || BA.B[0]->dir == 2) && check_Switch(BA.B[0],0,TRUE) && i > 0){
					//Next block is RED/Blocked
					if(BN.blocked || BN.B[0]->state == RED){
						set_signal(BA.B[0]->PSi,SIG_RED);
					}else if(BN.B[0]->state == PARKED){
						set_signal(BA.B[0]->PSi,SIG_RESTRICTED); //Flashing RED
					}else if(BN.B[0]->state == AMBER){	//Next block AMBER
						set_signal(BA.B[0]->PSi,SIG_AMBER);
					}else{ // //Next block AMBER	if(BN->state == GREEN)
						set_signal(BA.B[0]->PSi,SIG_GREEN);
					}
				}
			}
			else if(BA.B[0]->PSi && p == 0){ //If the track stops due to switches, set it to RED / DANGER
				set_signal(BA.B[0]->NSi,SIG_RED);
			}
		/**/
		/**/
		/*TRAIN control*/
			//Only if track is DCC controled and NOT DC!!
			if(BA.blocked && train_link[BA.B[0]->train]){
				if(k == 0){
					train_link[BA.B[0]->train]->halt = 1;
				}else if(k > 0 && train_link[BA.B[0]->train]->halt == 1){
					train_link[BA.B[0]->train]->halt = 0;
				}
			}
			if(_SYS->_STATE & STATE_TRACK_DIGITAL){

			/*SPEED*/
				//Check if current and next block are blocked, and have different trainIDs
				if(BA.blocked && BN.B[0]->blocked && BA.B[0]->train != BN.B[0]->train){
					if(train_link[BA.B[0]->train]){
						//Kill train
						printf("COLLISION PREVENTION\n\t");
						train_stop(train_link[BA.B[0]->train]);
					}else{
						//No train coupled
						printf("COLLISION PREVENTION\tEM_STOP\n\t");
						WS_EmergencyStop(); //WebSocket Emergency Stop
					}
				}
				//Check if next block is a RED block
				if(((BA.blocked && !BN.blocked && BN.B[0]->state == RED) || (k == 0 && BA.blocked))){
					if(train_link[BA.B[0]->train]){
						if(train_link[BA.B[0]->train]->timer != 1){
							//Fire stop timer
							printf("NEXT SIGNAL: RED\n\tSTOP TRAIN:");
							train_signal(BA.B[0],train_link[BA.B[0]->train],RED);
						}
					}else{
						//No train coupled
						printf("STOP TRAIN\tEM_STOP\n\t");
						WS_EmergencyStop();
					}
				}
				else if(k > 1 && BN.blocked && !BNN.blocked && BNN.B[0]->state == RED){
					if(train_link[BN.B[0]->train]){
						if(train_link[BN.B[0]->train]->timer != 1){
							//Fire stop timer
							printf("NEXT SIGNAL: RED\n\tSTOP TRAIN:");
							train_signal(BN.B[0],train_link[BN.B[0]->train],RED);
						}
					}else{
						//No train coupled
						printf("STOP TRAIN\tEM_STOP\n\t");
						WS_EmergencyStop();
					}
				}
				//Check if next block is a AMBER block
				if(((BA.blocked && !BN.blocked && BN.B[0]->state == AMBER) || (k == 1 && BA.blocked && !BN.blocked))){
					printf("Next AMBER\n");
					if(train_link[BA.B[0]->train]){
						if(train_link[BA.B[0]->train]->timer != 1){
							//Fire slowdown timer
							printf("NEXT SIGNAL: AMBER\n\tSLOWDOWN TRAIN:\t");
							train_signal(BA.B[0],train_link[BA.B[0]->train],RED);
						}
					}
				}

				//If the next 2 blocks are free, accelerate
				//If the next block has a higher speed limit than the current
				if(k > 0 && !BN.blocked && BA.B[0]->train != 0 && train_link[BA.B[0]->train] && train_link[BA.B[0]->train]->timer != 2 && train_link[BA.B[0]->train]->timer != 1){
					if((BN.B[0]->state == GREEN || BN.B[0]->state == RESERVED) && train_link[BA.B[0]->train]->cur_spd < BA.B[0]->max_speed && BN.B[0]->max_speed >= BA.B[0]->max_speed){
						printf("Next block has a higher speed limit (%i > %i)",BN.B[0]->max_speed,BA.B[0]->max_speed);
						train_speed(BA.B[0],train_link[BA.B[0]->train],BA.B[0]->max_speed);
					}
				}

				//If the next block has a lower speed limit than the current
				if(BA.B[0]->train != 0 && train_link[BA.B[0]->train] && train_link[BA.B[0]->train]->timer != 2){
					if(k > 0 && (BN.B[0]->state == GREEN || BN.B[0]->state == RESERVED) && train_link[BA.B[0]->train]->cur_spd > BN.B[0]->max_speed && BN.B[0]->type != 'T'){
						printf("Next block has a lower speed limit");
						train_speed(BN.B[0],train_link[BA.B[0]->train],BN.B[0]->max_speed);
					}else if(k > 1 && BN.B[0]->type == 'T' && BNN.B[0]->type != 'T' && (BNN.B[0]->state == GREEN || BNN.B[0]->state == RESERVED) && train_link[BA.B[0]->train]->cur_spd > BNN.B[0]->max_speed){
						printf("Block after Switches has a lower speed limit");
						train_speed(BNN.B[0],train_link[BA.B[0]->train],BNN.B[0]->max_speed);
					}else if(train_link[BA.B[0]->train]->cur_spd != BN.B[0]->max_speed && BN.B[0]->type != 'T'){
						printf("%i <= %i\n",train_link[BA.B[0]->train]->cur_spd,BN.B[0]->max_speed && BN.B[0]->type != 'T');
					}
				}
			//
			/*Station / Route*/
				//If next block is the destination
				if(BA.B[0]->train != 0 && train_link[BA.B[0]->train] && !Block_cmp(0,train_link[BA.B[0]->train]->Destination)){
					if(k > 0 && Block_cmp(train_link[BA.B[0]->train]->Destination,BN.B[0])){
						printf("Destination almost reached\n");
						train_signal(BA.B[0],train_link[BA.B[0]->train],AMBER);
					}else if(Block_cmp(train_link[BA.B[0]->train]->Destination,BA.B[0])){
						printf("Destination Reached\n");
						train_signal(BA.B[0],train_link[BA.B[0]->train],RED);
						train_link[BA.B[0]->train]->Destination = 0;
						train_link[BA.B[0]->train]->halt = TRUE;
					}
				}

			}
		/**/
		/**/
	}
}

void procces_accessoire(){
	for(int i = 0;i<MAX_Modules;i++){
		if(Units[i] && Units[i]->Sig_change){
			printf("Signals of module %i changed\n",i);
			for(int j = 0;j<(MAX_Blocks*MAX_Segments);j++){
				if(Units[i]->Signals[j]){
					printf("Signal id: %i\n",Units[i]->Signals[j]->id);
				}
			}
			Units[i]->Sig_change = FALSE;
		}
	}
}

int init_connect_Algor(struct ConnectList * List){
	// printf("init_connect_Algor\n");
	int return_value = 0;
	for(int i = 0;i < MAX_Modules;i++){
		if(Units[i]){
			for(int j = 0;j < Units[i]->B_L; j++){
				if(Units[i]->B[j]){
					if(Units[i]->B[j]->NextC.type == 'C' || Units[i]->B[j]->PrevC.type == 'C'){
						printf("found block %i:%i\n",i,j);
						if(List->list_index <= List->length + 1){
							struct Rail_link ** temp = (struct Rail_link **)calloc(List->list_index+8,sizeof(struct Rail_link *));
							for(int q = 0;q < List->list_index;q++){
								temp[q] = List->R_L[q];
							}
							List->R_L = temp;
							List->list_index += 8;
						}
						// printf("write index: %i\n",List->length);
						List->R_L[List->length] = (struct Rail_link *)malloc(sizeof(struct Rail_link));
						List->R_L[List->length]->type = 'R';
						List->R_L[List->length++]->ptr  = Units[i]->B[j];
					}
				}
			}

			for(int j = 0;j < Units[i]->S_L; j++){
				if(Units[i]->S[j]){
					if(Units[i]->S[j]->AppC.type == 'C' || Units[i]->S[j]->DivC.type == 'C' || Units[i]->S[j]->StrC.type == 'C'){
						printf("module %i, switch %i\n",i,j);
						if(List->list_index <= List->length + 1){
							struct Rail_link ** temp = (struct Rail_link **)calloc(List->list_index+8,sizeof(struct Rail_link *));
							for(int q = 0;q < List->list_index;q++){
								temp[q] = List->R_L[q];
							}
							List->R_L = temp;
							List->list_index += 8;
						}
						List->R_L[List->length] = (struct Rail_link *)malloc(sizeof(struct Rail_link));
						List->R_L[List->length]->type = 'S';
						List->R_L[List->length++]->ptr  = Units[i]->S[j];
					}
				}
			}

			return_value += Units[i]->connect_points;
		}
	}
	return return_value;
}

_Bool find_and_connect(char ModuleA, char anchor_A, char ModuleB, char anchor_B){
	//Node shouldn't be connected to the same Module
	if(ModuleA == ModuleB){return FALSE;}

	char typeA = 0;
	char typeB = 0;

	_Bool connected = FALSE;

	// printf("find_and_connect: %i:%i\t\t%i:%i\n",ModuleA,anchor_A,ModuleB,anchor_B);

	for(int Rail = 1;Rail<3;Rail++){
		struct Rail_link A;A.ptr = 0;
		struct Rail_link B;B.ptr = 0;

		//Find Anchor A
		// - Find a Block
			for(int k = 0;k<Units[ModuleA]->B_L;k++){
				if(Units[ModuleA]->B[k]){
					if(Units[ModuleA]->B[k]->PrevC.type == 'C'){
						// printf(" - A block Prev %i:%i",ModuleA,k);
						if(Units[ModuleA]->B[k]->PrevC.Module == anchor_A && Units[ModuleA]->B[k]->PrevC.Adr == Rail){
							// printf("++++++\n");
							A.type = 'R';
							typeA  = 'P';
							A.ptr = Units[ModuleA]->B[k];
							break;
						}
						// printf("\n");
					}
					else if(Units[ModuleA]->B[k]->NextC.type == 'C'){
						// printf(" - A block Next %i:%i",ModuleA,k);
						if(Units[ModuleA]->B[k]->NextC.Module == anchor_A && Units[ModuleA]->B[k]->NextC.Adr == Rail){
							// printf("++++++\n");
							A.type = 'R';
							typeA  = 'N';
							A.ptr = Units[ModuleA]->B[k];
							break;
						}
						// printf("\n");
					}
				}
			}
		// - Find a switch
			if(!A.ptr){
				for(int k = 0;k<Units[ModuleA]->S_L;k++){
					if(Units[ModuleA]->S[k]){
						if(Units[ModuleA]->S[k]->AppC.type == 'C'){
							// printf(" - A Switch App %i:%i",ModuleA,k);
							if(Units[ModuleA]->S[k]->AppC.Module == anchor_A && Units[ModuleA]->S[k]->AppC.Adr == Rail){
								// printf("++++++\n");
								A.type = 'S';
								typeA  = 'A';
								A.ptr = Units[ModuleA]->S[k];
								break;
							}
							// printf("\n");
						}
						else if(Units[ModuleA]->S[k]->StrC.type == 'C'){
							// printf(" - A Switch Str %i:%i",ModuleA,k);
							if(Units[ModuleA]->S[k]->StrC.Module == anchor_A && Units[ModuleA]->S[k]->StrC.Adr == Rail){
								// printf("++++++\n");
								A.type = 'S';
								typeA  = 'S';
								A.ptr = Units[ModuleA]->S[k];
								break;
							}
							// printf("\n");
						}
						else if(Units[ModuleA]->S[k]->DivC.type == 'C'){
							// printf(" - A Switch Div %i:%i",ModuleA,k);
							if(Units[ModuleA]->S[k]->DivC.Module == anchor_A && Units[ModuleA]->S[k]->DivC.Adr == Rail){
								// printf("++++++\n");
								A.type = 'S';
								typeA  = 'D';
								A.ptr = Units[ModuleA]->S[k];
								break;
							}
							// printf("\n");
						}
					}
				}
			}
		// - Find a msswitch
			if(!A.ptr){
				printf("Mayby a MSwitch, but not IMPLEMENTED!!\n");
				continue;
			}

		//Find Anchor B
		// - Find a block
			for(int k = 0;k<Units[ModuleB]->B_L;k++){
				if(Units[ModuleB]->B[k]){
					if(Units[ModuleB]->B[k]->NextC.type == 'C'){
						// printf(" - B block Prev %i:%i",ModuleB,k);
						if(Units[ModuleB]->B[k]->NextC.Module == anchor_B && Units[ModuleB]->B[k]->NextC.Adr == Rail){
							// printf("++++++\n");
							B.type = 'R';
							typeB  = 'N';
							B.ptr = Units[ModuleB]->B[k];
							break;
						}
						// printf("\n");
					}
					else if(Units[ModuleB]->B[k]->PrevC.type == 'C'){
						// printf(" - B block Prev %i:%i",ModuleB,k);
						if(Units[ModuleB]->B[k]->PrevC.Module == anchor_B && Units[ModuleB]->B[k]->PrevC.Adr == Rail){
							// printf("++++++\n");
							B.type = 'R';
							typeB  = 'P';
							B.ptr = Units[ModuleB]->B[k];
							break;
						}
						// printf("\n");
					}
				}
			}
		// - Find a Switch
			if(!B.ptr){
				for(int k = 0;k<Units[ModuleB]->S_L;k++){
					if(Units[ModuleB]->S[k]){
						if(Units[ModuleB]->S[k]->AppC.type == 'C'){
							// printf(" - B switch App %i:%i",ModuleB,k);
							if(Units[ModuleB]->S[k]->AppC.Module == anchor_B && Units[ModuleB]->S[k]->AppC.Adr == Rail){
								// printf("++++++\n");
								B.type = 'S';
								typeB  = 'A';
								B.ptr = Units[ModuleB]->S[k];
								break;
							}
							// printf("\n");
						}
						else if(Units[ModuleB]->S[k]->StrC.type == 'C'){
							// printf(" - B switch Str %i:%i",ModuleB,k);
							if(Units[ModuleB]->S[k]->StrC.Module == anchor_B && Units[ModuleB]->S[k]->StrC.Adr == Rail){
								// printf("++++++\n");
								B.type = 'S';
								typeB  = 'S';
								B.ptr = Units[ModuleB]->S[k];
								break;
							}
							// printf("\n");
						}
						else if(Units[ModuleB]->S[k]->DivC.type == 'C'){
							// printf(" - B switch Div %i:%i",ModuleB,k);
							if(Units[ModuleB]->S[k]->DivC.Module == anchor_B && Units[ModuleB]->S[k]->DivC.Adr == Rail){
								// printf("++++++\n");
								B.type = 'S';
								typeB  = 'D';
								B.ptr = Units[ModuleB]->S[k];
								break;
							}
							// printf("\n");
						}
					}
				}
			}
		// - Find a MSwitch
			if(!B.ptr){
				printf("Mayby a MSwitch, but not IMPLEMENTED!!\n");
				continue;
			}

		if(A.type == 'R' && B.type == 'R'){
			printf("Connecting R %i:%i <==> %i:%i R\n",((block *)A.ptr)->Module,((block *)A.ptr)->id,((block *)B.ptr)->Module,((block *)B.ptr)->id);
		}
		else if(A.type == 'S' && B.type == 'R'){
			printf("Connecting S %i:%i <==> %i:%i R\n",((Switch *)A.ptr)->Module,((Switch *)A.ptr)->id,((block *)B.ptr)->Module,((block *)B.ptr)->id);	
		}
		else if(A.type == 'R' && B.type == 'S'){
			printf("Connecting R %i:%i <==> %i:%i S\n",((block *)A.ptr)->Module,((block *)A.ptr)->id,((Switch *)B.ptr)->Module,((Switch *)B.ptr)->id);		
		}

		connected = TRUE;

		//Connect
		if(A.type == 'R' && B.type == 'R'){
			if(typeA == 'P'){
				((block *)A.ptr)->PrevC.Module = ((block *)B.ptr)->Module;
				((block *)A.ptr)->PrevC.Adr    = ((block *)B.ptr)->id;
				((block *)A.ptr)->PrevC.type   = 'R';
				((block *)B.ptr)->NextC.Module = ((block *)A.ptr)->Module;
				((block *)B.ptr)->NextC.Adr    = ((block *)A.ptr)->id;
				((block *)B.ptr)->NextC.type   = 'R';
			}
			else{
				((block *)A.ptr)->NextC.Module = ((block *)B.ptr)->Module;
				((block *)A.ptr)->NextC.Adr    = ((block *)B.ptr)->id;
				((block *)A.ptr)->NextC.type   = 'R';
				((block *)B.ptr)->PrevC.Module = ((block *)A.ptr)->Module;
				((block *)B.ptr)->PrevC.Adr    = ((block *)A.ptr)->id;
				((block *)B.ptr)->PrevC.type   = 'R';
			}
		}
		else if(A.type == 'S' && B.type == 'R'){
			if(typeB == 'N'){
				((block  *)B.ptr)->NextC.Module = ((Switch *)A.ptr)->Module;
				((block  *)B.ptr)->NextC.Adr    = ((Switch *)A.ptr)->id;
				((block  *)B.ptr)->NextC.type   = (typeA == 'A') ? 'S' : 's';
			}
			else{
				((block  *)B.ptr)->PrevC.Module = ((Switch *)A.ptr)->Module;
				((block  *)B.ptr)->PrevC.Adr    = ((Switch *)A.ptr)->id;
				((block  *)B.ptr)->PrevC.type   = (typeA == 'A') ? 'S' : 's';
			}

			if(typeA == 'A'){
				((Switch *)A.ptr)->AppC.Module = ((block *)B.ptr)->Module;
				((Switch *)A.ptr)->AppC.Adr    = ((block *)B.ptr)->id;
				((Switch *)A.ptr)->AppC.type   = 'R';
			}
			else if(typeA == 'S'){
				((Switch *)A.ptr)->StrC.Module = ((block *)B.ptr)->Module;
				((Switch *)A.ptr)->StrC.Adr    = ((block *)B.ptr)->id;
				((Switch *)A.ptr)->StrC.type   = 'R';
			}
			else if(typeA == 'D'){
				((Switch *)A.ptr)->DivC.Module = ((block *)B.ptr)->Module;
				((Switch *)A.ptr)->DivC.Adr    = ((block *)B.ptr)->id;
				((Switch *)A.ptr)->DivC.type   = 'R';
			}
		}
		else if(A.type == 'R' && B.type == 'S'){
			if(typeA == 'N'){
				((block  *)A.ptr)->NextC.Module = ((Switch *)B.ptr)->Module;
				((block  *)A.ptr)->NextC.Adr    = ((Switch *)B.ptr)->id;
				((block  *)A.ptr)->NextC.type   = (typeB == 'A') ? 'S' : 's';
			}
			else{
				((block  *)A.ptr)->PrevC.Module = ((Switch *)B.ptr)->Module;
				((block  *)A.ptr)->PrevC.Adr    = ((Switch *)B.ptr)->id;
				((block  *)A.ptr)->PrevC.type   = (typeB == 'A') ? 'S' : 's';
			}

			if(typeB == 'A'){
				((Switch *)B.ptr)->AppC.Module = ((block *)A.ptr)->Module;
				((Switch *)B.ptr)->AppC.Adr    = ((block *)A.ptr)->id;
				((Switch *)B.ptr)->AppC.type   = 'R';
			}
			else if(typeB == 'S'){
				((Switch *)B.ptr)->StrC.Module = ((block *)A.ptr)->Module;
				((Switch *)B.ptr)->StrC.Adr    = ((block *)A.ptr)->id;
				((Switch *)B.ptr)->StrC.type   = 'R';
			}
			else if(typeB == 'D'){
				((Switch *)B.ptr)->DivC.Module = ((block *)A.ptr)->Module;
				((Switch *)B.ptr)->DivC.Adr    = ((block *)A.ptr)->id;
				((Switch *)B.ptr)->DivC.type   = 'R';
			}
		}
	}

	if(connected && ModuleA && anchor_A && ModuleB && anchor_B){
		Units[ModuleA]->Connect[anchor_A-1] = Units[ModuleB];
		Units[ModuleB]->Connect[anchor_B-1] = Units[ModuleA];
	}

	return connected;
}

int connect_Algor(struct ConnectList * List){
	struct Rail_link * R = 0;

	char Anchor_A,Rail_A,Anchor_B,Rail_B;

	int value = 0;

	for(int i = 0;i<List->length;i++){
		if(!List->R_L[i]->ptr){
			continue;
		}
		if(List->R_L[i]->type == 'R'){
			if(((block *)List->R_L[i]->ptr)->NextC.type != 'C' && ((block *)List->R_L[i]->ptr)->PrevC.type != 'C') {
				value++;
				continue;
			}
			// printf("Found block %i:%i %i\t",((block*)List->R_L[i]->ptr)->Module,((block*)List->R_L[i]->ptr)->id,((block*)List->R_L[i]->ptr)->blocked);
			if(((block *)List->R_L[i]->ptr)->blocked){
				//Blocked block
				// printf("Found\n");
				if(!R){
					R = List->R_L[i];
				}
				else{
					_Bool connected = FALSE;
					char anchor_A = 0;
					char anchor_B = 0;

					int ModuleA = 0;
					int ModuleB = 0;
					if(R->type == 'R'){
						ModuleA = ((block *)R->ptr)->Module;
						ModuleB = ((block *)List->R_L[i]->ptr)->Module;

						if(((block *)R->ptr)->NextC.type == 'C'){
							anchor_A = ((block *)R->ptr)->NextC.Module;
							anchor_B = ((block *)List->R_L[i]->ptr)->PrevC.Module;
						}
						else if(((block *)R->ptr)->PrevC.type == 'C'){
							anchor_A = ((block *)R->ptr)->PrevC.Module;
							anchor_B = ((block *)List->R_L[i]->ptr)->NextC.Module;
						}

						connected = find_and_connect(ModuleA,anchor_A,ModuleB,anchor_B);
					}
					else if(R->type == 'S'){
						ModuleA = ((Switch *)R->ptr)->Module;
						ModuleB = ((block *)List->R_L[i]->ptr)->Module;
						if(((block *)List->R_L[i]->ptr)->NextC.type == 'C'){
							anchor_B = ((block *)List->R_L[i]->ptr)->NextC.Module;
						}
						else{
							anchor_B = ((block *)List->R_L[i]->ptr)->PrevC.Module;
						}

						if(((Switch *)R->ptr)->AppC.type == 'C'){
							anchor_A = ((Switch *)R->ptr)->AppC.Module;
						}
						else if(((Switch *)R->ptr)->StrC.type == 'C'){
							anchor_A = ((Switch *)R->ptr)->StrC.Module;
						}
						else if(((Switch *)R->ptr)->DivC.type == 'C'){
							anchor_A = ((Switch *)R->ptr)->DivC.Module;
						} //End Switch approach type

						connected = find_and_connect(ModuleA,anchor_A,ModuleB,anchor_B);
					} // End Switch type

					if(connected){
						WS_Partial_Layout(ModuleA,ModuleB);
						_Bool connected = FALSE;
					}
				}
			}
		}
		else if(List->R_L[i]->type == 'S' && ((Switch *)List->R_L[i]->ptr)->Detection_Block->blocked){
			//Blocked switch
			if(!R){
				R = List->R_L[i];
			}
		}
	}

	value = 0;
	int total = 0;

	for(int i = 0;i < MAX_Modules;i++){
		if(Units[i]){
			for(int j = 0;j<Units[i]->connect_points;j++){
				total++;
				if(Units[i]->Connect[j]){
					value++;
				}
			}
		}
	}
	int i = List->length - 1;
	
	if(value == total){
		_SYS_change(STATE_Modules_Coupled, 1);
	}
	
	return value;
}