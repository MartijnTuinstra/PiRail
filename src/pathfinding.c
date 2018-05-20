#define _BSD_SOURCE
// #define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "./../lib/system.h"

#include "./../lib/rail.h"
#include "./../lib/switch.h"
#include "./../lib/trains.h"

#include "./../lib/pathfinding.h"

int pathFinding(struct Seg * Begin, struct Seg * End, struct Sw_train_PATH  *(OUT_Sw_Nodes)[MAX_ROUTE], int * len){
	struct Rail_link NAdr,SNAdr;
	struct Seg * B = Begin;
	struct Sw_A_PATH * Sw_Nodes[MAX_ROUTE] = {NULL};
	int nr_switches = 0;
	struct Sw_A_PATH * Prev_PATH_Node = NULL;
	struct Sw_A_PATH * Curr_PATH_Node = NULL;

	char list_of_M[MAX_Modules] = {0};

	_Bool init = TRUE;
	_Bool found = FALSE;

	int a = 0;

	*len = 0;

	printf("Pathfinding form %c%i:%i to %c%i:%i\n",B->type,B->Module,B->id,End->type,End->Module,End->id);
	//printf("\n%i\n",i);
	int direct = B->dir;
	char prev_dir = direct;

	Next_Adr:{};

	if(!B){
		goto Next_Switch;
	}

	char dir = B->dir;

	//Get next block based on the direction of the block and the direction of travel (0x80 in prev_dir)
	if((prev_dir == 0 && dir == 1) || (prev_dir == 129 && dir == 0)){
		prev_dir ^= 0x80;	//Reverse
		NAdr = B->Next;
	}
	else if((prev_dir == 1 && dir == 0) || (prev_dir == 128 && dir == 1)){
		prev_dir ^= 0x80;	//Reverse
		NAdr = B->Prev;
	}
	else if((prev_dir >> 7) == 1){
		if(dir == 0 || dir == 2 || dir == 5){
			NAdr = B->Prev;
		}else{
			NAdr = B->Next;
		}
	}
	else if(dir == 0 || dir == 2 || dir == 5){
		NAdr = B->Next;
	}
	else{
		NAdr = B->Prev;
	}

	prev_dir = (prev_dir & 0xC0) + (dir & 0xF);

	Next_Switch:{};
	//printf("\n");
	/*
	printf("\nNAdr type:%c\t",NAdr.type);
  if(NAdr.type == 'R'){
    printf("R   %02i:%02i\t",NAdr.B->Module,NAdr.B->id);
  }else if(NAdr.type == 'S' || NAdr.type == 's'){
    printf("Sw  %02i:%02i\t",NAdr.Sw->Module,NAdr.Sw->id);
  }else if(NAdr.type == 'M' || NAdr.type == 'm'){
    printf("MSw %02i:%02i\t",NAdr.M->Module,NAdr.M->id);
  }
	printf("Curr_PATH_Node: 0x%X",Curr_PATH_Node);*/

	//Segment is Rail and is the same as the destination segment
	if(NAdr.type == 'R' && End == (block *)NAdr.ptr){
		printf("\nFOUND\n\n");
		FOUND:{}

		//Set all switches in try mode to Successfull
		//and register the module number
		for(int i = 0;i<MAX_ROUTE;i++){
			if(Sw_Nodes[i]){
				for(int j = 0;j<10;j++){
					if(Sw_Nodes[i]->suc[j] == 1){
						Sw_Nodes[i]->suc[j] = 2;
					}
				}
					//if(Sw_Nodes[i]->adr.type == 'S')
						//printf("S  %02i:%02i\t%i\t%i\n",Sw_Nodes[i]->adr.Sw->Module,Sw_Nodes[i]->adr.Sw->id,Sw_Nodes[i]->suc[0],Sw_Nodes[i]->suc[1] );
				_Bool A = FALSE;
				for(int k = 0;k<MAX_Modules;k++){
					if(Sw_Nodes[i]->adr.type == 'S' && (list_of_M[k] == ((Switch *)Sw_Nodes[i]->adr.ptr)->Module || list_of_M[k] == 0)){
						list_of_M[k] = ((Switch *)Sw_Nodes[i]->adr.ptr)->Module;
						break;
					}else if(Sw_Nodes[i]->adr.type == 'M' && (list_of_M[k] == ((msswitch *)Sw_Nodes[i]->adr.ptr)->Module || list_of_M[k] == 0)){
						list_of_M[k] = ((msswitch *)Sw_Nodes[i]->adr.ptr)->Module;
						break;
					}
				}
			}else{
				break;
			}
		}
		found = TRUE;

		for(int i = 0;i<MAX_ROUTE;i++){
			//printf("i:%i\t",i);
			if(Sw_Nodes[i] != NULL){
				//printf("Adr.type = %c\t",Sw_Nodes[i]->adr.type);
				if(Sw_Nodes[i]->adr.type == 'S' && (Sw_Nodes[i]->suc[0] == 0 || Sw_Nodes[i]->suc[1] == 0)){
					Curr_PATH_Node = Sw_Nodes[i];
          NAdr = Curr_PATH_Node->adr;
          dir = prev_dir = Curr_PATH_Node->dir;
					//printf("Retrying %c%i:%i\n",NAdr.type,NAdr.Sw->Module,NAdr.Sw->id);
					usleep(1000);
					goto Next_Switch;
				}else if((Sw_Nodes[i]->adr.type == 'm' || Sw_Nodes[i]->adr.type == 'M')){
					//printf("Test Moduls\n");
					/*
					for(int j = 0;j<Moduls[Sw_Nodes[i]->adr.M][Sw_Nodes[i]->adr.B][Sw_Nodes[i]->adr.S]->length;j++){
						if(Sw_Nodes[i]->suc[j] == 0){
							Curr_PATH_Node = Sw_Nodes[i];
							NAdr = Adr = Curr_PATH_Node->adr;
							dir = prev_dir = Curr_PATH_Node->dir;
							printf("Retrying %i:%i:%i\n",NAdr.M,NAdr.B,NAdr.S);
							usleep(1000);
							goto Next_Switch;
						}
					}
					*/
				}
			}else{
				break;
			}
		}
		//printf("FOUND the route\n");
		goto DONE;
		return 1;//blocks[Adr.M][Adr.B][Adr.S];
	}

	usleep(2000);

	if(NAdr.type == 0 || (NAdr.type == 'R' && ((block *)NAdr.ptr)->oneWay == 1 && (char)(dir + (prev_dir & 0x80)) != ((block *)NAdr.ptr)->dir)){ //End of rails or wrong way
	    if(NAdr.type == 0){
		    //printf("End of rails returning to Current PATH Node\n");
	    }else{
	      //printf("Wrong Way!!\n");
	    }
		goto FAIL;
	}
	else if(NAdr.type == 'R' && (block *)NAdr.ptr == Begin){ //Back at starting Node??
		//printf("Back at starting Node\n");
		goto FAIL;
	}
	else if(NAdr.type == 'R'){ //If rail
		B = (block *)NAdr.ptr;
		goto Next_Adr;
	}
	else if(NAdr.type == 'S' || NAdr.type == 's' || NAdr.type == 'm' || NAdr.type == 'M'){ //Next node is a (MS) Switch

		if((NAdr.type == 'S' || NAdr.type == 's') && ((Switch *)NAdr.ptr)->Detection_Block && 
						!Block_cmp(((Switch *)NAdr.ptr)->Detection_Block,B)){
			a++;
		}
		if(NAdr.type == 'S'){ //If Approaching Switch
			//Add to List
			goto New_S_Node; //Function with current stack
			Ret_S_Node:{}   //Return point for function

			//If a node is present
			if(Curr_PATH_Node){
				if(Curr_PATH_Node->suc[0] == 1){ //If state 0 is in try
					SNAdr = ((Switch *)NAdr.ptr)->str;
				}
				else if(Curr_PATH_Node->suc[1] == 1){ //If state 1 is in try
					SNAdr = ((Switch *)NAdr.ptr)->div;
				}
			}
		}
		else if(NAdr.type == 's'){ //Mergin Switch
			SNAdr = ((Switch *)NAdr.ptr)->app;
			//printf("Approach\t");
      //printf("SNAdr: %i:%i:%i\n",SNAdr.M,SNAdr.B,SNAdr.S);
		}
		else if(NAdr.type == 'M'){ //MSSwitch approach M side
			goto New_M_Node; //Function with current stack
			Ret_M_Node:{}   //Return point for function

			//If a node is present
			if(Curr_PATH_Node){
				for(int i = 0;i<((msswitch *)NAdr.ptr)->length;i++){ //Browse through all states
					if(Curr_PATH_Node->suc[i] == 1){ //If state i is in try
						SNAdr = ((msswitch *)NAdr.ptr)->m_Adr[i];
						break;
		}}}}
		else if(NAdr.type == 'm'){ //MSSwitch approach m side
			goto New_M_Node; //Function with current stack
			Ret_m_Node:{}   //Return point for function

			//If a node is present
			if(Curr_PATH_Node != NULL){
				for(int i = 0;i<((msswitch *)NAdr.ptr)->length;i++){ //Browse through all states
					if(Curr_PATH_Node->suc[i] == 1){ //If state i is in try
						SNAdr = ((msswitch *)NAdr.ptr)->M_Adr[i];
		}}}}

		if(SNAdr.type == 'S' || SNAdr.type == 's' || SNAdr.type == 'M' || SNAdr.type == 'm'){ //If next is a switch again
			NAdr = SNAdr;
			goto Next_Switch; //Procces switch
		}
		else if(SNAdr.type == 'R'){ //Just a rail
			B = (block *)SNAdr.ptr;
			NAdr.ptr = (void *)B;
			NAdr.type = 'R';

			goto Next_Switch; //Procces rail segment
		}else if(SNAdr.type == 0){ //If empty (end of rails)
			goto FAIL;
		}
	}
	else{ //A wrong type
		printf("\n\n!!!!!!!!!UNKOWN RAIL_LINK TYPE!!!!!\n\n");
	}
  printf("Return 0\n");
	return 0;

	FAIL:{
		//Current path has failed
		//Change state of last switch

		if(Curr_PATH_Node){

      if(Curr_PATH_Node->adr.type == 'S'){ //If last switch was a normal Switch
        if(Curr_PATH_Node->suc[0] == 1){ //If state 0 was is try
          Curr_PATH_Node->suc[0] = -1;  //Set it to fail
	        Curr_PATH_Node->suc[1] = 1;  //And set state 1 to try

					//Reset NAdr and dir
					NAdr = Curr_PATH_Node->adr;
					dir = prev_dir = Curr_PATH_Node->dir;

					//And return to the main procces
					if(NAdr.type == 'S' || NAdr.type == 's' || NAdr.type == 'M' || NAdr.type == 'm'){
						goto Next_Switch;
					}else{
						goto Next_Adr;
					}
        }
				else if(Curr_PATH_Node->suc[1] == 1){ //If state 1 was in try
          Curr_PATH_Node->suc[1] = -1; //Set state 1 to a fail
        }
				if(Curr_PATH_Node->suc[0] == 2 || Curr_PATH_Node->suc[1] == 2){ //Both were a fail
					//Scan missing switch states or go to previous switch
					if(found == TRUE){
						goto FOUND;
					}else if(!Curr_PATH_Node->Prev){
						goto NOT_FOUND;
					}
				}
      }
			else{ //It was not a normal switch but a MSSwitch
  			for(int i = 0;i<10;i++){ //Browse through all states
  				if(Curr_PATH_Node->suc[i] == 1){ //If state i was in try
  					Curr_PATH_Node->suc[i] = -1;  //Set it to fail

						if((i+1) < ((msswitch *)Curr_PATH_Node->adr.ptr)->length){ //If state i is not the last state
	  					Curr_PATH_Node->suc[i+1] = 1; //Try next state
						}

						//Reset NAdr and dir
  					NAdr = Curr_PATH_Node->adr;
  					dir = prev_dir = Curr_PATH_Node->dir;

						//And return to the main procces
  					if(NAdr.type == 'S' || NAdr.type == 's' || NAdr.type == 'M' || NAdr.type == 'm'){
  						goto Next_Switch;
  					}else{
  						goto Next_Adr;
  					}
  				}
  			}
      }

			//If the switch failed go to previous node and rerun FAIL
			Curr_PATH_Node = Curr_PATH_Node->Prev;
			goto FAIL;
		}else{
			printf("No Curr_PATH_Node\n");
		}
	}

	printf("Return FAIL\n");
	return 0;

	New_S_Node:{
		//A Switch Node is encounterd

		if((Curr_PATH_Node &&	!Link_cmp(Curr_PATH_Node->adr,NAdr)) ||	!Curr_PATH_Node){
			if(Sw_Nodes[0] && Link_cmp(Sw_Nodes[0]->adr,NAdr)){ //Back to first discovered turnout??
				//printf("Back to start!!\n");

				if(Curr_PATH_Node){
					for(int i = 0;i<10;i++){
						if(Curr_PATH_Node->suc[i] == 1){
							Curr_PATH_Node->suc[i] = -1;
							NAdr = Curr_PATH_Node->adr;
							dir = Curr_PATH_Node->dir;
							prev_dir = dir;
							if(NAdr.type == 'S' || NAdr.type == 's' || NAdr.type == 'M' || NAdr.type == 'm'){
								goto Next_Switch;
							}else{
								goto Next_Adr;
							}
						}
					}
				}
			}
			else if(found == TRUE){
				//printf("Check if between borders\t");
				for(int i = 0;i<MAX_Modules;i++){
					if(list_of_M[i] == ((Switch *)NAdr.ptr)->Module){
						break;
					}
					//Oustide
					if((i+1) == MAX_Modules){
						if(Curr_PATH_Node){
							for(int i = 0;i<10;i++){
								if(Curr_PATH_Node->suc[i] == 1){
									Curr_PATH_Node->suc[i] = -1;
									NAdr = Curr_PATH_Node->adr;
									dir = Curr_PATH_Node->dir;
									prev_dir = dir;
									if(NAdr.type == 'S' || NAdr.type == 's' || NAdr.type == 'M' || NAdr.type == 'm'){
										goto Next_Switch;
									}else{
										goto Next_Adr;
									}
								}
							}
							if(found == TRUE){
								goto FOUND;
							}
							Curr_PATH_Node = Curr_PATH_Node->Prev;
						}
					}
				}
				//printf("Check if allready known??\n");
				for(int i = 0;i<MAX_ROUTE;i++){
					if(Sw_Nodes[i] != NULL && Link_cmp(Sw_Nodes[i]->adr,NAdr)){
						//printf("Allready known Node\n");
						if(Sw_Nodes[i]->adr.type == 'S'){
							//printf("It a switch\t");
							if(Sw_Nodes[i]->suc[0] == -1 && Sw_Nodes[i]->suc[1] == -1){
								//printf("Both have no route\t");
								goto FAIL;
							}
						}
						goto FOUND;
						break;
					}
				}
			}

			//printf("Save PATH Node\tDir=%i\t",dir);

			struct Sw_A_PATH * Z = (struct Sw_A_PATH *)malloc(sizeof(struct Sw_A_PATH ));

			Z->adr = NAdr;
			Z->length = 2;
			memset(Z->suc,0,10);
			Z->suc[0] = 1;
			Z->dir = dir + (prev_dir & 0x80);
			Z->Prev = Curr_PATH_Node;

			for(int i = 0;i<MAX_ROUTE;i++){
				if(Sw_Nodes[i] == NULL){
					//printf("i:%i\t",i);
					Sw_Nodes[i] = Z;
					//printf("%i:%i:%i:%c",Sw_Nodes[i]->adr.M,Sw_Nodes[i]->adr.B,Sw_Nodes[i]->adr.S,Sw_Nodes[i]->adr.type);
					break;
				}else{
					//printf("No\t%i:%i:%i:%c\n",Sw_Nodes[i]->adr.M,Sw_Nodes[i]->adr.B,Sw_Nodes[i]->adr.S,Sw_Nodes[i]->adr.type);
				}
			}
			//printf("\n");
			Curr_PATH_Node = Z;
		}
		else if(Curr_PATH_Node && Link_cmp(Curr_PATH_Node->adr,NAdr)){ //If current Node exists
			//printf("Success [%i][%i]",Curr_PATH_Node->suc[0],Curr_PATH_Node->suc[1]);
			if(Curr_PATH_Node->suc[0] != 0 && Curr_PATH_Node->suc[0] != 1){
				if(Curr_PATH_Node->suc[1] == 0){
					//printf("\tTry next");
					Curr_PATH_Node->suc[1] = 1;
				}else if(Curr_PATH_Node->suc[1] == -1){
					goto FAIL;
				}
			}
		}
		goto Ret_S_Node;
	}

	printf("Return New_S_Node\n");
	return 0;

	New_M_Node:{
		//printf("NMN\t");
		if((Curr_PATH_Node && !Link_cmp(Curr_PATH_Node->adr,NAdr)) || !Curr_PATH_Node){
			if(Sw_Nodes[0] && Link_cmp(Sw_Nodes[0]->adr,NAdr)){ //Back to first discovered turnout??
				//printf("Back to start!!\n");

				if(Curr_PATH_Node != NULL){
					for(int i = 0;i<10;i++){
						if(Curr_PATH_Node->suc[i] == 1){
							Curr_PATH_Node->suc[i] = -1;
							NAdr = Curr_PATH_Node->adr;
							dir = Curr_PATH_Node->dir;
							prev_dir = dir;
							if(NAdr.type == 'S' || NAdr.type == 's' || NAdr.type == 'M' || NAdr.type == 'm'){
								goto Next_Switch;
							}else{
								goto Next_Adr;
							}
						}
					}
				}
			}
			else if(found == TRUE){
				//printf("Check if between borders\n");
				for(int i = 0;i<MAX_Modules;i++){
					if(list_of_M[i] == ((msswitch *)NAdr.ptr)->Module){
						break;
					}
					//Oustide
					if((i+1) == MAX_Modules){
						if(Curr_PATH_Node != NULL){
							for(int i = 0;i<10;i++){
								if(Curr_PATH_Node->suc[i] == 1){
									Curr_PATH_Node->suc[i] = -1;
									NAdr = Curr_PATH_Node->adr;
									dir = Curr_PATH_Node->dir;
									prev_dir = dir;
									if(NAdr.type == 'S' || NAdr.type == 's' || NAdr.type == 'M' || NAdr.type == 'm'){
										printf("Next_Switch\n");
										goto Next_Switch;
									}else{
										goto Next_Adr;
									}
								}
							}
							if(found == TRUE){
								goto FOUND;
							}
							Curr_PATH_Node = Curr_PATH_Node->Prev;
						}
					}
				}
			}
			//printf("Check if allready known??\n");
			for(int i = 0;i<MAX_ROUTE;i++){
				if(Sw_Nodes[i] && Link_cmp(Sw_Nodes[i]->adr,NAdr)){
					printf("Allready known Node\n");
					if(Sw_Nodes[i]->adr.type == 'S'){
						printf("It a switch\t");
						if(Sw_Nodes[i]->suc[0] == -1 && Sw_Nodes[i]->suc[1] == -1){
							printf("Both have no route\t");
							goto FAIL;
						}
					}else if(Sw_Nodes[i]->adr.type == 'M' || Sw_Nodes[i]->adr.type == 'm'){
						printf("It a MS switch\t");
						_Bool NoSucc = TRUE;
						for(int i = 0;i<((msswitch *)Sw_Nodes[i]->adr.ptr)->length;i++){
							if(Sw_Nodes[i]->suc[i] == 2){
								NoSucc = FALSE;
								break;
							}
						}
						if(NoSucc == TRUE){
							printf("No route\t");
							goto FAIL;
						}
					}
					goto FOUND;
					break;
				}
			}

			//printf("Save PATH Node\t");

			struct Sw_A_PATH * Z = (struct Sw_A_PATH *)malloc(sizeof(struct Sw_A_PATH ));

			Z->adr = NAdr;
			Z->length = 2;
			memset(Z->suc,0,10);
			Z->suc[0] = 1;
			Z->dir = prev_dir;
			Z->Prev = Curr_PATH_Node;

			for(int i = 0;i<MAX_ROUTE;i++){
				if(Sw_Nodes[i] == NULL){
					printf("%i",i);
					Sw_Nodes[i] = Z;
					break;
				}
			}
			//printf("\n");
			Curr_PATH_Node = Z;
		}
		else if(Curr_PATH_Node && Link_cmp(Curr_PATH_Node->adr,NAdr)){ //If current Node exists
			//printf("Success [%i][%i]",Curr_PATH_Node->suc[0],Curr_PATH_Node->suc[1]);
			for(int i = 0;i<((msswitch *)NAdr.ptr)->length;i++){
				if(Curr_PATH_Node->suc[i] == 0){
					//printf("\tTry next");
					Curr_PATH_Node->suc[i] = 1;
				}else if(Curr_PATH_Node->suc[i] == -1 && (i+1) == ((msswitch *)NAdr.ptr)->length){
					//printf("\tAll failed");
					Curr_PATH_Node = Curr_PATH_Node->Prev;
					NAdr = Curr_PATH_Node->adr;
					if(NAdr.type == 'S' || NAdr.type == 's' || NAdr.type == 'M' || NAdr.type == 'm'){
						goto Next_Switch;
					}else{
						goto Next_Adr;
					}
				}
			}
		}
		if(NAdr.type == 'M'){
			goto Ret_M_Node;
		}else{
			goto Ret_m_Node;
		}
	}

	printf("Return New_S_Node\n");
	return 0;

	NOT_FOUND:{}


	printf("Return NOT_FOUND\n");
	return 0;

	DONE:{}

	printf("All Switches:\n");

	int k = 0;

	for(int i = 0;i<MAX_ROUTE;i++){
		if(Sw_Nodes[i]){
			_Bool A = FALSE;
			for(int j = 0;j<10;j++){
				if(Sw_Nodes[i]->suc[j] == 2){
					A = TRUE;
				}
			}
			if(A){
				printf("\n%i\t%c",k,Sw_Nodes[i]->adr.type);
				(*len)++;
				if(Sw_Nodes[i]->adr.type == 'S' || Sw_Nodes[i]->adr.type == 's'){
					Switch * tSw = (Switch *)Sw_Nodes[i]->adr.ptr;
					printf("%i:%i  \t",tSw->Module,tSw->id);
				}else if(Sw_Nodes[i]->adr.type == 'M' || Sw_Nodes[i]->adr.type == 'm'){
					msswitch * tmsw = (msswitch *)Sw_Nodes[i]->adr.ptr;
					printf("%i:%i  \t",tmsw->Module,tmsw->id);
				}
				for(int j = 0;j<10;j++){
					if(Sw_Nodes[i]->suc[j] == 0){
						break;
					}
					printf("%d\t",Sw_Nodes[i]->suc[j]);
				}

				struct Sw_train_PATH  * Z = (struct Sw_train_PATH  *)malloc(sizeof(struct Sw_train_PATH ));
				Z->adr = Sw_Nodes[i]->adr;
				Z->states == 0;
				for(int l = 0;l<10;l++){
					if(Sw_Nodes[i]->suc[l] == 2){
						Z->suc[Z->states++] = l;
					}
				}
				*OUT_Sw_Nodes++ = Z;
			}
		}else{
			break;
		}
	}
	printf("Len: %i\n",*len);
	return 1;
}
