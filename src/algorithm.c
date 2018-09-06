#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#include "system.h"

#include "algorithm.h"
#include "logger.h"

#include "rail.h"
#include "train.h"
#include "switch.h"
#include "signals.h"

#include "module.h"
#include "com.h"
#include "websocket_msg.h"

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
    for(int i = 0;i<unit_len;i++){
      if(Units[i]){
        for(int j = 0;j<=Units[i]->block_len;j++){
          if(Units[i]->B[j]){
            //printf("%i:%i\n",i,j);
            // process(Units[i]->B[j],0);
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

    // printf("\n");
  }
  return 0;
}

void scan_All(){
  pthread_mutex_lock(&mutex_lockA);
  for(int i = 0;i<unit_len;i++){
    if(Units[i]){
      for(int j = 0;j<=Units[i]->block_len;j++){
        if(Units[i]->B[j]){
          //printf("%i:%i\n",i,j);
          process(Units[i]->B[j],2);
        }
      }
    }
  }

  WS_trackUpdate(0);
  pthread_mutex_unlock(&mutex_lockA);
}

void change_block_state(Algor_Block * A, enum Rail_states state){
  if(!A->blocked){
    for(int i = 0;i<A->length;i++){
      if(A->B[i]->state > state){
        printf("change_block_state: %i:%i    %i => %i\n", A->B[i]->module, A->B[i]->id, A->B[i]->state, state);
        A->B[i]->changed = 1;
        A->B[i]->state = state;
      }
    }
  }else{
    for(int i = 0;i<A->length;i++){
      printf("change_block_state: %i:%i    %i => %i\n", A->B[i]->module, A->B[i]->id, A->B[i]->state, state);
      if(A->B[i]->blocked){
        break;
      }
      if(A->B[i]->state > state){
        A->B[i]->changed = 1;
        A->B[i]->state = state;
      }
    }
  }
}

void process(Block * B,int flags){
  int debug = (flags & 1);
  int force = (flags & 2);

  if(!B->blocked && B->train == 0 && !force){
    return;
  }
  // if(B->changed == 0){
  //   return;
  // }

  B->changed &= ~(IO_Changed);
  B->changed |= State_Changed;


  //init_Algor_Blocks and clear
  Algor_Block BPPP,BPP,BP,BN,BNN,BNNN;

  //Clear pointer
  BPPP.B[0] = NULL;BPPP.B[1] = NULL;BPPP.B[2] = NULL;BPPP.B[3] = NULL;BPPP.B[4] = NULL;
  BPP.B[0]  = NULL;BPP.B[1]  = NULL;BPP.B[2]  = NULL;BPP.B[3]  = NULL;BPP.B[4]  = NULL;
  BP.B[0]   = NULL;BP.B[1]   = NULL;BP.B[2]   = NULL;BP.B[3]   = NULL;BP.B[4]   = NULL;
  BN.B[0]   = NULL;BN.B[1]   = NULL;BN.B[2]   = NULL;BN.B[3]   = NULL;BN.B[4]   = NULL;
  BNN.B[0]  = NULL;BNN.B[1]  = NULL;BNN.B[2]  = NULL;BNN.B[3]  = NULL;BNN.B[4]  = NULL;
  BNNN.B[0] = NULL;BNNN.B[1] = NULL;BNNN.B[2] = NULL;BNNN.B[3] = NULL;BNNN.B[4] = NULL;
  //Clear data
  BPPP.blocked = 0;BPP.blocked = 0;BP.blocked = 0;BN.blocked = 0;BNN.blocked = 0;BNNN.blocked = 0;
  BPPP.blocks  = 0;BPP.blocks  = 0;BP.blocks  = 0;BN.blocks  = 0;BNN.blocks  = 0;BNNN.blocks  = 0;
  BPPP.length  = 0;BPP.length  = 0;BP.length  = 0;BN.length  = 0;BNN.length  = 0;BNNN.length  = 0;
  // Link algor_blocks
  struct algor_blocks AllBlocks;
  AllBlocks.BPPP = &BPPP;
  AllBlocks.BPP  = &BPP;
  AllBlocks.BP   = &BP;
  AllBlocks.B    = B;
  AllBlocks.BN   = &BN;
  AllBlocks.BNN  = &BNN;
  AllBlocks.BNNN = &BNNN;

  
  //Find all surrounding blocks. Can be speeded up by storing this into the block. Update only if a (MS)switch changes or the direciton changes
  Algor_search_Blocks(&AllBlocks, debug);

  Algor_print_block_debug(AllBlocks);

  
  //Follow the train arround the layout
  Algor_train_following(AllBlocks, debug);
  if (B->changed & IO_Changed)
    return;

  //Set oncomming switch to correct state
  Algor_Switch_Checker(AllBlocks, debug);
  if (B->changed & IO_Changed)
    return;

  //Apply block stating
  Algor_rail_state(AllBlocks, debug);

  //Check Switch

  // Print all found blocks to stdout
  Algor_print_block_debug(AllBlocks);

  Algor_signal_state(AllBlocks, debug);

  //Train Control

  if(0){
    return;

    //------------------------------------------------------------------------------------------ roadmap: more efficient scanning of the block. skip the blocks that are not changed/blocked and there neighbours are also not blocked
    //SPEEDUP function / if all blocks are not blocked skip!!
    /*if(k > 0 && !BA.blocked && !BN.blocked){
      if(p > 0 && !BP.blocked){
        return;
      }else if(p == 0){
        return;
      }
    }*/
    /**/
    // /**/
    // /*Check switch*/
    //   //
    //   int New_Switch = 0;
    //   struct rail_link NAdr,NNAdr, bTraindr;

      
    //   if(k > 0){
    //     // NNAdr = Next_link(BN.B[BN.length - 1]);
    //   }

    //   if((NNAdr.type == 's' || NNAdr.type == 'S' || NNAdr.type == 'm' || NNAdr.type == 'M') && BA.blocked){
    //     //There is a switch after the next block
    //     if(BA.B[0]->train != 0 && train_link[BA.B[0]->train] && train_link[BA.B[0]->train]->route.Destination){
    //       //If train has a route
    //       if(check_Switch_State(NNAdr)){
    //         //Switch is free to use
    //         New_Switch = 2;
    //         printf("Free switch ahead (OnRoute %i:%i)\n",BA.B[0]->module,BA.B[0]->id);
    //         if(!free_Route_Switch(BN.B[BN.length - 1],0,train_link[BA.B[0]->train])){
    //           printf("FAILED to set switch according Route\n");
    //           New_Switch = 0;
    //         }
    //       }
    //     }else{
    //       //No route
    //       if(check_Switch_State(NNAdr)){
    //         //Switch is free to use
    //         New_Switch = 2;
    //         printf("Free switch ahead %i:%i\n",BA.B[0]->module,BA.B[0]->id);
    //         if(!BN.B[BN.length - 1]){
    //           printf("Check_switch but no block R\n");
    //         }
    //         if(!check_Switch(Next_link(BN.B[BN.length - 1]),TRUE)){
    //           //The switch is in the wrong state / position
    //           printf("Check Switch\n");
    //           if(!free_Switch(BN.B[BN.length - 1],0)){
    //             New_Switch = 0;
    //     }}}}
    //   }else if((NAdr.type == 's' || NAdr.type == 'S' || NAdr.type == 'm' || NAdr.type == 'M') && BA.blocked){
    //     //There is a switch after the current block
    //     if(BA.B[0]->train != 0 && train_link[BA.B[0]->train] && train_link[BA.B[0]->train]->route.Destination){
    //       //If train has a route
    //       if(check_Switch_State(NAdr)){
    //         //Switch is free to use
    //         New_Switch = 1;
    //         printf("Free switch ahead (OnRoute %i:%i)\n",BA.B[0]->module,BA.B[0]->id);
    //         if(!free_Route_Switch(BA.B[BA.length - 1],0,train_link[BA.B[0]->train])){
    //           New_Switch = 0;
    //         }
    //       }
    //     }else{
    //       //No route
    //       if(check_Switch_State(NNAdr)){
    //         New_Switch = 1;
    //         //Switch is free to use
    //         printf("Free switch ahead %i:%i\n",BA.B[0]->module,BA.B[0]->id);
    //         if(!check_Switch(Next_link(BN.B[BN.length - 1]),TRUE)){
    //           //The switch is in the wrong state / position
    //           printf("Check Switch\n");
    //           if(!free_Switch(BN.B[BN.length - 1],0)){
    //             printf("FAILED to set switch according Route\tSTOPPING TRAIN\n");
    //             New_Switch = 0;
    //     }}}}
    //   }

    //   //Extend if switches are thrown
    //   if(New_Switch > 0){
    //     printf("Switch thrown");
    //     if(k < 1){
    //       printf("BN  NEEDED\t");
    //       BN.B[0] = Next(BA.B[0],0,1+BN.length);
    //       if(BN.B[0]){
    //         BN.length++;
    //         BN.blocked |= BN.B[0]->blocked;
    //         if(BN.B[0]->type == 'T'){
    //           BN.B[1] = Next(BA.B[0],0,1+BN.length);
    //           if(BN.B[1]->type == 'T'){
    //             BN.length++;
    //             BN.blocked |= BN.B[1]->blocked;
    //           }else{
    //             BN.B[1] = NULL;
    //           }
    //         }
    //         k++;
    //       }
    //     }
    //     if(k < 2){
    //       printf("BNN  NEEDED\t");
    //       BNN.B[0] = Next(BA.B[0],0,1+BN.length+BNN.length);
    //       if(BNN.B[0]){
    //         BNN.length++;
    //         BNN.blocked |= BNN.B[0]->blocked;
    //         if(BNN.B[0]->type == 'T'){
    //           BNN.B[1] = Next(BA.B[0],0,1+BN.length+BNN.length);
    //           if(BNN.B[1]->type == 'T'){
    //             BNN.length++;
    //             BNN.blocked |= BNN.B[1]->blocked;
    //           }else{
    //             BNN.B[1] = NULL;
    //           }
    //         }
    //         k++;
    //       }
    //     }
    //     if(k < 3){
    //       BNNN.B[0] = Next(BA.B[0],0,1+BN.length+BNN.length+BNNN.length);
    //       if(BNNN.B[0]){
    //         BNNN.length++;
    //         BNNN.blocked |= BNNN.B[0]->blocked;
    //         if(BNNN.B[0]->type == 'T'){
    //           BNNN.B[1] = Next(BA.B[0],0,1+BN.length+BNN.length+BNNN.length);
    //           if(BNNN.B[1]->type == 'T'){
    //             BNNN.length++;
    //             BNNN.blocked |= BNNN.B[1]->blocked;
    //           }else{
    //             BNNN.B[1] = NULL;
    //           }
    //         }
    //         k++;
    //       }
    //     }
    //   }

    //   if(New_Switch == 1){
    //     change_block_state(&BN,RESERVED_SWITCH);
    //   }else if(New_Switch == 2){
    //     change_block_state(&BNN,RESERVED_SWITCH);
    //   }

    //   /*
    //     if((NAdr.type == 's' || NAdr.type == 'S' || NAdr.type == 'm' || NAdr.type == 'M') && BA.blocked){
    //       if(((NAdr.type == 's' || NAdr.type == 'S') && NAdr.Sw->Detection && NAdr.Sw->Detection->state != RESERVED) ||
    //          ((NAdr.type == 'm' || NAdr.type == 'M') &&  NAdr.M->Detection &&  NAdr.M->Detection->state != RESERVED)){
    //         if(!check_Switch(BN.B[BN.length - 1],0,TRUE)){
    //           //The switch is not reserved and is in the wrong position
    //           printf("Check Switch\n");
    //           if(free_Switch2(BN.B[BN.length - 1],0)){
    //             printf("Freed");
    //             printf("BNN RESERVED\n");
    //             change_block_state2(&BNN,RESERVED);
    //           }
    //         }
    //         else{
    //           //If switch is in correct position but is not reserved
    //           change_block_state2(&BNN,RESERVED);
    //         }
    //       }
    //     }
    //   }*/
    // /**/
    // /**/
    // /*Reverse block after one or two zero-blocks*/
    //   //If the next block is reversed, and not blocked
    //   if(i > 0 && BA.blocked && BN.B[0] && BN.B[0]->type != 'T' && !BN.blocked && !dircmp(BA.B[0],BN.B[0])){
    //     BN.B[0]->dir ^= 0b100;
    //   }

    //   //Reverse one block in advance
    //   if(i > 1 && BA.blocked && BNN.B[0] && BNN.B[0]->type != 'T' && !dircmp(BA.B[0],BNN.B[0]) &&
    //           (BN.B[0]->type == 'T' || !(dircmp(BA.B[0],BN.B[0]) == dircmp(BN.B[0],BNN.B[0])))){

    //     printf("Reverse in advance 1\n");
    //     if(BNN.B[0]->type == 'S'){ //Reverse whole platform if it is one
    //       printf("Whole platform\n");
    //       for(int a = 0;a<8;a++){
    //         if(BNN.B[0]->station->blocks[a]){
    //           if(BNN.B[0]->station->blocks[a]->blocked){
    //             break;
    //           }
    //           BNN.B[0]->station->blocks[a]->dir ^= 0b100;
    //         }
    //       }
    //     }else{
    //       printf("Block %x\n",BNN.B[0]);
    //       BNN.B[0]->dir ^= 0b100;
    //     }
    //   }
    // /**/
    // /**/
    // /*State coloring*/
    //   //Block behind train (blocked) becomes RED
    //   //Second block behind trin becomes AMBER
    //   //After that GREEN

    //   //Double 0-block counts as one block

    //   //If current block is blocked and previous block is free
    //   if(BA.B[0]->module == 5 && (BA.B[0]->id == 2 || BA.B[0]->id == 3)){
    //     int karamba = 0;
    //   }
    //   if(p > 0 && BA.blocked && !BP.blocked){
    //     change_block_state(&BP, DANGER);
    //     if(p > 2)
    //       change_block_state(&BPPP, PROCEED);
    //     if(p > 1)
    //       change_block_state(&BPP, CAUTION);
    //   }
    //   else if(i > 0 && !BA.blocked && BN.blocked && BN.B[0]->type == 'T'){
    //     change_block_state(&BA, DANGER);
    //     if(p > 0)
    //       change_block_state(&BP, CAUTION);
    //     if(p > 1)
    //       change_block_state(&BPP, PROCEED);
    //   }
    //   else if(p > 1 && k > 1 && !BA.blocked && !BN.blocked && !BP.blocked && !BNN.blocked && !BPP.blocked){
    //     change_block_state(&BA, PROCEED);
    //     if(p > 2 && !BPPP.blocked){
    //       change_block_state(&BP, PROCEED);
    //     }
    //     if(k > 2 && !BNNN.blocked){
    //       change_block_state(&BN,PROCEED);
    //     }
    //   }
    // /**/
    // /**/
    // /*Signals*/
    //   //If a signal is at Next end and BN exists
    //   if(BA.B[0]->NextSignal  && k > 0){
    //     //Wrong Switch
    //     //if current block is in forward and there are blocked switches
    //     // or if the block is in the wrong direction (reverse)
    //     if(((BA.B[0]->dir == 0 || BA.B[0]->dir == 1 || BA.B[0]->dir == 2) && !check_Switch(Next_link(BA.B[0]),TRUE)) || (BA.B[0]->dir == 4 || BA.B[0]->dir == 5 || BA.B[0]->dir == 6)){
    //       set_signal(BA.B[0]->NextSignal, DANGER);
    //     }

    //     if(!(BA.B[0]->dir == 4 || BA.B[0]->dir == 5 || BA.B[0]->dir == 6) && check_Switch(Next_link(BA.B[0]),TRUE) && i > 0){
    //       //Next block is RED/Blocked
    //       if(BN.blocked || BN.B[0]->state == DANGER){
    //         set_signal(BA.B[0]->NextSignal, DANGER);
    //       }else if(BN.B[0]->state == RESTRICTED){
    //         set_signal(BA.B[0]->NextSignal, RESTRICTED); //Flashing RED
    //       }else if(BN.B[0]->state == CAUTION){  //Next block AMBER
    //         set_signal(BA.B[0]->NextSignal, CAUTION);
    //       }else{ // //Next block AMBER  if(BN->state == GREEN)
    //         set_signal(BA.B[0]->NextSignal, PROCEED);
    //       }
    //     }
    //   }
    //   else if(BA.B[0]->NextSignal  && k == 0){ //If the track stops due to switches, set it to RED / DANGER
    //     set_signal(BA.B[0]->NextSignal, DANGER);
    //   }

    //   //If a signal is at Prev side and BP exists
    //   if(BA.B[0]->PrevSignal  && p > 0){
    //     //printf("Signal at %i:%i\n",BA.B[0]->module,BA.B[0]->id);
    //     //printf("check_Switch: %i\n",check_Switch(BA.B[0],0,TRUE));
    //     //printf("Signal at %i:%i:%i\t0x%x\n",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->signals);
    //     //if current block is in reverse and there are blocked switches
    //     // or if the block is in the wrong direction (forward)
    //     if(((BA.B[0]->dir == 4 || BA.B[0]->dir == 5 || BA.B[0]->dir == 6) && !check_Switch(Next_link(BA.B[0]),TRUE)) || (BA.B[0]->dir == 0 || BA.B[0]->dir == 1 || BA.B[0]->dir == 2)){
    //       set_signal(BA.B[0]->PrevSignal, DANGER);
    //       //printf("%i:%i:%i\tRed signal R2\n",BA->Adr.M,BA->Adr.B,BA->Adr.S);
    //     }

    //     if(!(BA.B[0]->dir == 0 || BA.B[0]->dir == 1 || BA.B[0]->dir == 2) && check_Switch(Next_link(BA.B[0]),TRUE) && i > 0){
    //       //Next block is RED/Blocked
    //       if(BN.blocked || BN.B[0]->state == DANGER){
    //         set_signal(BA.B[0]->PrevSignal, DANGER);
    //       }else if(BN.B[0]->state == RESTRICTED){
    //         set_signal(BA.B[0]->PrevSignal, RESTRICTED); //Flashing RED
    //       }else if(BN.B[0]->state == CAUTION){  //Next block AMBER
    //         set_signal(BA.B[0]->PrevSignal, CAUTION);
    //       }else{ // //Next block AMBER  if(BN->state == GREEN)
    //         set_signal(BA.B[0]->PrevSignal, PROCEED);
    //       }
    //     }
    //   }
    //   else if(BA.B[0]->PrevSignal  && p == 0){ //If the track stops due to switches, set it to RED / DANGER
    //     set_signal(BA.B[0]->NextSignal, DANGER);
    //   }
    // /**/
    // /**/
    // /*TRAIN control*/
    //   //Only if track is DCC controled and NOT DC!!
    //   if(BA.blocked && train_link[BA.B[0]->train]){
    //     if(k == 0){
    //       train_link[BA.B[0]->train]->halt = 1;
    //     }else if(k > 0 && train_link[BA.B[0]->train]->halt == 1){
    //       train_link[BA.B[0]->train]->halt = 0;
    //     }
    //   }
    //   if(_SYS->_STATE & STATE_TRACK_DIGITAL){

    //   /*SPEED*/
    //     //Check if current and next block are blocked, and have different trainIDs
    //     if(BA.blocked && BN.B[0]->blocked && BA.B[0]->train != BN.B[0]->train){
    //       if(train_link[BA.B[0]->train]){
    //         //Kill train
    //         printf("COLLISION PREVENTION\n\t");
    //         loggerf(ERROR, "FIX train_stop");
    //         //train_stop(train_link[BA.B[0]->train]);
    //       }else{
    //         //No train coupled
    //         printf("COLLISION PREVENTION\tEM_STOP\n\t");
    //         WS_EmergencyStop(); //WebSocket Emergency Stop
    //       }
    //     }
    //     //Check if next block is a RED block
    //     if(((BA.blocked && !BN.blocked && BN.B[0]->state == DANGER) || (k == 0 && BA.blocked))){
    //       if(train_link[BA.B[0]->train]){
    //         if(train_link[BA.B[0]->train]->timer != 1){
    //           //Fire stop timer
    //           printf("NEXT SIGNAL: RED\n\tSTOP TRAIN:");
    //           loggerf(ERROR, "FIX train_signal");
    //           //train_signal(BA.B[0],train_link[BA.B[0]->train], DANGER);
    //         }
    //       }else{
    //         //No train coupled
    //         printf("STOP TRAIN\tEM_STOP\n\t");
    //         WS_EmergencyStop();
    //       }
    //     }
    //     else if(k > 1 && BN.blocked && !BNN.blocked && BNN.B[0]->state == DANGER){
    //       if(train_link[BN.B[0]->train]){
    //         if(train_link[BN.B[0]->train]->timer != 1){
    //           //Fire stop timer
    //           printf("NEXT SIGNAL: RED\n\tSTOP TRAIN:");
    //           loggerf(ERROR, "FIX train_signal");
    //           //train_signal(BN.B[0],train_link[BN.B[0]->train], DANGER);
    //         }
    //       }else{
    //         //No train coupled
    //         printf("STOP TRAIN\tEM_STOP\n\t");
    //         WS_EmergencyStop();
    //       }
    //     }
    //     //Check if next block is a AMBER block
    //     if(((BA.blocked && !BN.blocked && BN.B[0]->state == CAUTION) || (k == 1 && BA.blocked && !BN.blocked))){
    //       printf("Next AMBER\n");
    //       if(train_link[BA.B[0]->train]){
    //         if(train_link[BA.B[0]->train]->timer != 1){
    //           //Fire slowdown timer
    //           printf("NEXT SIGNAL: AMBER\n\tSLOWDOWN TRAIN:\t");
    //           loggerf(ERROR, "FIX train_signal");
    //           //train_signal(BA.B[0],train_link[BA.B[0]->train], CAUTION);
    //         }
    //       }
    //     }

    //     //If the next 2 blocks are free, accelerate
    //     //If the next block has a higher speed limit than the current
    //     if(k > 0 && !BN.blocked && BA.B[0]->train != 0 && train_link[BA.B[0]->train] && train_link[BA.B[0]->train]->timer != 2 && train_link[BA.B[0]->train]->timer != 1){
    //       if((BN.B[0]->state == PROCEED || BN.B[0]->state == RESERVED) && train_link[BA.B[0]->train]->cur_speed < BA.B[0]->max_speed && BN.B[0]->max_speed >= BA.B[0]->max_speed){
    //         printf("Next block has a higher speed limit (%i > %i)",BN.B[0]->max_speed,BA.B[0]->max_speed);
    //         loggerf(ERROR, "FIX train_speed");
    //         //train_speed(BA.B[0],train_link[BA.B[0]->train],BA.B[0]->max_speed);
    //       }
    //     }

    //     //If the next block has a lower speed limit than the current
    //     if(BA.B[0]->train != 0 && train_link[BA.B[0]->train] && train_link[BA.B[0]->train]->timer != 2){
    //       if(k > 0 && (BN.B[0]->state == PROCEED || BN.B[0]->state == RESERVED) && train_link[BA.B[0]->train]->cur_speed > BN.B[0]->max_speed && BN.B[0]->type != 'T'){
    //         printf("Next block has a lower speed limit");
    //         loggerf(ERROR, "FIX train_speed");
    //         ///train_speed(BN.B[0],train_link[BA.B[0]->train],BN.B[0]->max_speed);
    //       }else if(k > 1 && BN.B[0]->type == 'T' && BNN.B[0]->type != 'T' && (BNN.B[0]->state == PROCEED || BNN.B[0]->state == RESERVED) && train_link[BA.B[0]->train]->cur_speed > BNN.B[0]->max_speed){
    //         printf("Block after Switches has a lower speed limit");
    //         loggerf(ERROR, "FIX train_speed");
    //         //train_speed(BNN.B[0],train_link[BA.B[0]->train],BNN.B[0]->max_speed);
    //       }else if(train_link[BA.B[0]->train]->cur_speed != BN.B[0]->max_speed && BN.B[0]->type != 'T'){
    //         printf("%i <= %i\n",train_link[BA.B[0]->train]->cur_speed,BN.B[0]->max_speed && BN.B[0]->type != 'T');
    //       }
    //     }
    //   //
    //   /*Station / Route*/
    //     //If next block is the destination
    //     if(BA.B[0]->train != 0 && train_link[BA.B[0]->train] && !block_cmp(0,train_link[BA.B[0]->train]->route.Destination)){
    //       if(k > 0 && block_cmp(train_link[BA.B[0]->train]->route.Destination,BN.B[0])){
    //         printf("Destination almost reached\n");
    //         loggerf(ERROR, "FIX train_signal");
    //         //train_signal(BA.B[0],train_link[BA.B[0]->train], CAUTION);
    //       }else if(block_cmp(train_link[BA.B[0]->train]->route.Destination,BA.B[0])){
    //         printf("Destination Reached\n");
    //         loggerf(ERROR, "FIX train_signal");
    //         //train_signal(BA.B[0],train_link[BA.B[0]->train], DANGER);
    //         train_link[BA.B[0]->train]->route.Destination = 0;
    //         train_link[BA.B[0]->train]->halt = TRUE;
    //       }
    //     }

    //   }
    // /**/
    // /**/
  }
}

void Algor_print_block_debug(struct algor_blocks AllBlocks){

  //Unpack AllBlocks
  Algor_Block BPPP = *AllBlocks.BPPP;
  Algor_Block BPP  = *AllBlocks.BPP;
  Algor_Block BP   = *AllBlocks.BP;
  Block * B        =  AllBlocks.B;
  Algor_Block BN   = *AllBlocks.BN;
  Algor_Block BNN  = *AllBlocks.BNN;
  Algor_Block BNNN = *AllBlocks.BNNN;

  char output[200] = "";

    if(BPPP.blocks > 0){
      sprintf(output, "%sPPP%i ", output, BPP.blocks);
      if(BPPP.blocked)
        sprintf(output, "%sB", output);
      else
        sprintf(output, "%s ", output);
      for(int i = 1;i>=0;i--){
        if(BPPP.B[i]){
          sprintf(output, "%s%02i:%02i", output,BPPP.B[i]->module,BPPP.B[i]->id);
          if(BPPP.B[i]->blocked){
            sprintf(output, "%sB  ", output);
          }else{
            sprintf(output, "%s   ", output);
          }
        }else{
          sprintf(output, "%s        ", output);
        }
      }
    }else{
      sprintf(output, "%s                      ", output);
    }
    if(BPP.blocks > 0){
      sprintf(output, "%sPP%i ", output,BPP.blocks);
      if(BPP.blocked)
        sprintf(output, "%sB", output);
      else
        sprintf(output, "%s ", output);
      for(int i = 1;i>=0;i--){
        if(BPP.B[i]){
          sprintf(output, "%s%02i:%02i", output,BPP.B[i]->module,BPP.B[i]->id);
          if(BPP.B[i]->blocked){
            sprintf(output, "%sB  ", output);
          }else{
            sprintf(output, "%s   ", output);
          }
        }else{
          sprintf(output, "%s        ", output);
        }
      }
    }else{
      sprintf(output, "%s                     ", output);
    }
    if(BP.blocks > 0){
      sprintf(output, "%sP%i ", output,BP.blocks);
      if(BP.blocked)
        sprintf(output, "%sB", output);
      else
        sprintf(output, "%s ", output);
      for(int i = 1;i>=0;i--){
        if(BP.B[i]){
          sprintf(output, "%s%02i:%02i", output,BP.B[i]->module,BP.B[i]->id);
          if(BP.B[i]->blocked){
            sprintf(output, "%sB  ", output);
          }else{
            sprintf(output, "%s   ", output);
          }
        }else{
          sprintf(output, "%s        ", output);
        }
      }
    }else{
      sprintf(output, "%s                    ", output);
    }
    sprintf(output, "%sA%3i %2x%02i:%02i;T%-2iD%-2iS%-2i", output,B->length,B->type,B->module,B->id,B->train,B->dir,B->state);
    if(B->blocked){
      sprintf(output, "%sB", output);
    }
    sprintf(output, "%s\t", output);
    if(BN.blocks > 0){
      sprintf(output, "%sN%i ", output,BN.blocks);
      if(BN.blocked)
        sprintf(output, "%sB", output);
      else
        sprintf(output, "%s ", output);
      for(int i = 0;i<2;i++){
        if(BN.B[i]){
          sprintf(output, "%s%02i:%02i", output,BN.B[i]->module,BN.B[i]->id);
          if(BN.B[i]->blocked){
            sprintf(output, "%sB  ", output);
          }else{
            sprintf(output, "%s   ", output);
          }
        }else{
          sprintf(output, "%s        ", output);
        }
      }
    }
    if(BNN.blocks > 0){
      sprintf(output, "%sNN%i ", output,BNN.blocks);
      if(BNN.blocked)
        sprintf(output, "%sB", output);
      else
        sprintf(output, "%s ", output);
      for(int i = 0;i<2;i++){
        if(BNN.B[i]){
          sprintf(output, "%s%02i:%02i", output,BNN.B[i]->module,BNN.B[i]->id);
          if(BNN.B[i]->blocked){
            sprintf(output, "%sB  ", output);
          }else{
            sprintf(output, "%s   ", output);
          }
        }else{
          sprintf(output, "%s        ", output);
        }
      }
    }
    if(BNNN.blocks > 0){
      sprintf(output, "%sNNN%i ", output,BNNN.blocks);
      if(BNNN.blocked)
        sprintf(output, "%sB", output);
      else
        sprintf(output, "%s ", output);
      for(int i = 0;i<2;i++){
        if(BNNN.B[i]){
          sprintf(output, "%s%02i:%02i", output,BNNN.B[i]->module,BNNN.B[i]->id);
          if(BNNN.B[i]->blocked){
            sprintf(output, "%sB  ", output);
          }else{
            sprintf(output, "%s   ", output);
          }
        }else{
          sprintf(output, "%s        ", output);
        }
      }
    }

  loggerf(DEBUG, "%s", output);
}

void Algor_Switch_Checker(struct algor_blocks AllBlocks, int debug){
  //Unpack AllBlocks
  //Algor_Block BPPP = *AllBlocks.BPPP;
  //Algor_Block BPP  = *AllBlocks.BPP;
  //Algor_Block BP   = *AllBlocks.BP;
  Block * B        =  AllBlocks.B;
  Algor_Block BN   = *AllBlocks.BN;
  //Algor_Block BNN  = *AllBlocks.BNN;
  //Algor_Block BNNN = *AllBlocks.BNNN;

  //Check Next 1
  if(B->blocked && BN.blocks > 0 && !BN.B[0]->blocked){
    Block * tmp;
    for(int i = 0; i < BN.blocks; i++){
      if(i == 0){
        tmp = B;
      }
      else{
        tmp = BN.B[i - 1]
      }
      struct rail_link link = Next_link(tmp, NEXT);
      if(link.type == 's' || link.type == 'm' || link.type == 'M'){
        if(!Next_check_Switch(tmp, link, NEXT | SWITCH_CARE)){
          if(link.type == 's'){
            loggerf(DEBUG, "Toggled %i:%i", ((Switch *)link.p)->module, ((Switch *)link.p)->id);
            set_switch(link.p, !(((Switch *)link.p)->state & 0x7F));
            B->changed |= IO_Changed;
            return;
          }
          else if(link.type == 'm'){
            loggerf(WARNING, "Next is msswitch !! %i:%i->%i:%i",
                    tmp->module,
                    tmp->id,
                    ((MSSwitch *)link.p)->module,
                    ((MSSwitch *)link.p)->id);
          }
          else if(link.type == 'M'){
            loggerf(WARNING, "Next is MSswitch !! %i:%i->%i:%i",
                    tmp->module,
                    tmp->id,
                    ((MSSwitch *)link.p)->module,
                    ((MSSwitch *)link.p)->id);
          }
        }
      }

      if(link.type == 'S' || link.type == 's'){
        if(((Switch *)link.p)->Detection->state != BLOCKED &&
           ((Switch *)link.p)->Detection->state != RESERVED &&
           ((Switch *)link.p)->Detection->state != RESERVED_SWITCH){

            ((Switch *)link.p)->Detection->state = RESERVED_SWITCH;
            ((Switch *)link.p)->Detection->changed = State_Changed;
        }
      }
      else if(link.type == 'M' || link.type == 'm'){
        if(((MSSwitch *)link.p)->Detection->state != BLOCKED &&
           ((MSSwitch *)link.p)->Detection->state != RESERVED &&
           ((MSSwitch *)link.p)->Detection->state != RESERVED_SWITCH){

            ((MSSwitch *)link.p)->Detection->state = RESERVED_SWITCH;
            ((MSSwitch *)link.p)->Detection->changed = State_Changed;
        }
      }
    }
  }
}

void Algor_search_Blocks(struct algor_blocks * AllBlocks, int debug){
  loggerf(TRACE, "Algor_search_Blocks");
  Block * next = 0;
  Block * prev = 0;
  Block * B = AllBlocks->B;

  int next_level = 1;
  int prev_level = 1;

  next = Next(B, NEXT | SWITCH_CARE,1);
  prev = Next(B, PREV | SWITCH_CARE,1);

  if(next && next->type == SPECIAL)
    next_level++;

  if(prev && prev->type == SPECIAL)
    prev_level++;

  //Select all surrounding blocks
  if(next){
    loggerf(TRACE, "Search Next");
    for(int i = 0; i < 3; i++){
      Algor_Block * block_p;
      if(i == 0){
        block_p = AllBlocks->BN;
      }
      else if(i == 1){
        block_p = AllBlocks->BNN;
      }
      else if(i == 2){
        block_p = AllBlocks->BNNN;
      }
  
      do{
        if(i == 0 && block_p->blocks == 0){
          block_p->B[block_p->blocks] = next;
        }
        else if(next->type != SPECIAL){
          block_p->B[block_p->blocks] = Next(next, NEXT | SWITCH_CARE, next_level++);
        }
        else{
          block_p->B[block_p->blocks] = Next(B, NEXT | SWITCH_CARE, next_level++);
        }
  
        if(!block_p->B[block_p->blocks]){
          i = 4;
          break;
        }

        if(block_p->B[block_p->blocks]->blocked)
          block_p->blocked = 1;
  
        block_p->length += block_p->B[block_p->blocks]->length;
  
        block_p->blocks += 1;
  
      }
      while(block_p->length < Block_Minimum_Size && block_p->blocks < 5);
    }
  }
  if(prev){
    loggerf(TRACE, "Search Prev");
    for(int i = 0; i < 3; i++){
      Algor_Block * block_p;
      if(i == 0){
        block_p = AllBlocks->BP;
      }
      else if(i == 1){
        block_p = AllBlocks->BPP;
      }
      else if(i == 2){
        block_p = AllBlocks->BPPP;
      }
  
      do{
        if(i == 0 && block_p->blocks == 0){
          block_p->B[block_p->blocks] = prev;
        }
        else if(prev->type != SPECIAL){
          block_p->B[block_p->blocks] = Next(prev, PREV | SWITCH_CARE, prev_level++);
        }
        else{
          block_p->B[block_p->blocks] = Next(B, PREV | SWITCH_CARE, prev_level++);
        }
  
        if(!block_p->B[block_p->blocks]){
          i = 4;
          break;
        }

        if(block_p->B[block_p->blocks]->blocked)
          block_p->blocked = 1;
  
        block_p->length += block_p->B[block_p->blocks]->length;
  
        block_p->blocks += 1;
  
      }
      while(block_p->length < Block_Minimum_Size && block_p->blocks < 5);
    }
  }
}

void Algor_train_following(struct algor_blocks AllBlocks, int debug){
  loggerf(TRACE, "Algor_train_following");
  //Unpack AllBlocks
  // Algor_Block BPPP = *AllBlocks.BPPP;
  // Algor_Block BPP  = *AllBlocks.BPP;
  Algor_Block BP   = *AllBlocks.BP;
  Block * B        =  AllBlocks.B;
  Algor_Block BN   = *AllBlocks.BN;
  // Algor_Block BNN  = *AllBlocks.BNN;
  // Algor_Block BNNN = *AllBlocks.BNNN;

  if(!B->blocked && B->train != 0){
    //Reset
    B->train = 0;
    if(debug) printf("RESET");
  }
  // else if(B->blocked && B->train != 0 && train_link[B->train] && !train_link[B->train]->Block){
  //   // Set block of train
  //   train_link[B->train]->Block = B;
  //   if(debug) printf("SET_BLOCK");
  // }

  if(BP.blocks > 0 && BN.blocks > 0 && B->blocked && !BP.blocked && BN.blocked && BN.B[0]->train && !B->train){
    //REVERSED
    B->dir ^= 0b100;
    loggerf(TRACE, "REVERSE BLOCK %i:%i", B->module, B->id);
    B->changed |= IO_Changed;
    Block_Reverse_To_Next_Switch(B);
    return;
  }
  else if(BP.blocks > 0 && BN.blocks > 0 && B->blocked && B->train == 0 && BN.B[0]->train == 0 && BP.B[0]->train == 0){
    //NEW TRAIN
    // find a new follow id
    // loggerf(ERROR, "FOLLOW ID INCREMENT, bTrain");
    B->train = find_free_index(train_link, train_link_len);

    //Create a message for WebSocket
    WS_NewTrain(B->train, B->module, B->id);
    if(debug) printf("NEW_TRAIN at %i\t", B->train);
  }
  else if(BN.blocks > 0 && BP.blocks > 0 && BN.blocked && BP.blocked && !B->blocked && BN.B[0]->train == BP.B[0]->train){
    //A train has split
    WS_TrainSplit(BN.B[0]->train, BP.B[0]->module,BP.B[0]->id,BN.B[0]->module,BN.B[0]->id);
    if(debug) printf("SPLIT_TRAIN");
  }

  if(BP.blocks > 0 && BP.blocked && B->blocked && B->train == 0 && BP.B[0]->train != 0){
    // Copy train id from previous block
    B->train = BP.B[0]->train;
    // if(train_link[B->train])
    //   train_link[B->train]->Block = B;
    if(debug) printf("COPY_TRAIN");
  }

  if(BN.blocks > 0 && BN.B[0]->type == 'T' && BN.blocked){
    if(BN.B[0]->train == 0 && BN.B[0]->blocked && B->blocked && B->train != 0){
      BN.B[0]->train = B->train;
      if(train_link[BN.B[0]->train])
        train_link[BN.B[0]->train]->Block = BN.B[0];
    }else if(BN.blocks > 1){
      for(int a = 1;a<BN.blocks;a++){
        if(BN.B[a-1]->blocked && BN.B[a]->blocked && BN.B[a]->train == 0 && BN.B[a-1]->train != 0){
          BN.B[a]->train = BN.B[a-1]->train;
          if(train_link[BN.B[a]->train])
            train_link[BN.B[a]->train]->Block = BN.B[a];
          break;
        }
      }
    }
  }
}

void Algor_rail_state(struct algor_blocks AllBlocks, int debug){
  loggerf(TRACE, "Algor_rail_state");
  //Unpack AllBlocks
  Algor_Block BPPP = *AllBlocks.BPPP;
  Algor_Block BPP  = *AllBlocks.BPP;
  Algor_Block BP   = *AllBlocks.BP;
  Block * B        =  AllBlocks.B;
  Algor_Block BN   = *AllBlocks.BN;
  // Algor_Block BNN  = *AllBlocks.BNN;
  // Algor_Block BNNN = *AllBlocks.BNNN;

  if(BN.blocks > 0 && BN.blocked && !B->blocked){
    B->state = DANGER;
    Algor_apply_rail_state(BP, CAUTION);
    Algor_apply_rail_state(BPP, PROCEED);
  }
  else if(B->blocked && !BP.blocked && BP.blocks > 0){
    Algor_apply_rail_state(BP, DANGER);
    Algor_apply_rail_state(BPP, CAUTION);
    Algor_apply_rail_state(BPPP, PROCEED);
  }
  else if(!B->blocked && BN.blocks == 0){
    B->state = CAUTION;
    printf(" End of track %i:%i ",B->module, B->id);
  }
}

void Algor_apply_rail_state(Algor_Block b, enum Rail_states state){
  loggerf(TRACE, "Algor_apply_rail_state");
  for(int i = 0; i < b.blocks; i++){
    b.B[i]->state = state;
    b.B[i]->changed |= State_Changed;
  }
}

void Algor_signal_state(struct algor_blocks AllBlocks, int debug){
  loggerf(TRACE, "Algor_signal_state");
  //Unpack AllBlocks
  Algor_Block BPPP = *AllBlocks.BPPP;
  Algor_Block BPP  = *AllBlocks.BPP;
  Algor_Block BP   = *AllBlocks.BP;
  Block * B        =  AllBlocks.B;
  Algor_Block BN   = *AllBlocks.BN;
  Algor_Block BNN  = *AllBlocks.BNN;
  Algor_Block BNNN = *AllBlocks.BNNN;

  Block * CB;
  Block * NB;
  Block * PB;

  if(B->NextSignal){
    if(BN.blocks > 0){
      NB = BN.B[0];
      set_signal(B->NextSignal, NB->state);
    }
    else
      set_signal(B->NextSignal, DANGER); //No track left
  }
  else if(B->PrevSignal){
    if(BP.blocks > 0){
      PB = BP.B[0];
      set_signal(B->PrevSignal, PB->state);
    }
    // else
    //   set_signal(B->PrevSignal, PB->state);
  }

  for(int group = 0; group < 6; group++){
    Algor_Block * list = 0;
    Block * nextlist = 0;
    Block * prevlist = 0;
    if(group == 0){
      list = &BPPP;
      nextlist = BPP.B[BPP.blocks - 1];
    }
    else if(group == 1){
      prevlist = BPPP.B[0];
      list = &BPP;
      nextlist = BP.B[BP.blocks - 1];
    }
    else if(group == 2){
      prevlist = BPP.B[0];
      list = &BP;
      nextlist = B;
    }
    else if(group == 3){
      prevlist = B;
      list = &BN;
      nextlist = BNN.B[0];
    }
    else if(group == 4){
      prevlist = BN.B[BN.blocks - 1];
      list = &BNN;
      nextlist = BNNN.B[0];
    }
    else if(group == 5){
      prevlist = BNN.B[BNN.blocks - 1];
      list = &BNNN;
    }
    else
      break;

    if(list->blocks > 0){
      for(int i = 0; i < list->blocks; i++){
        CB = list->B[i];
        //Skip if block has no signals at all.
        if(!CB->NextSignal && !CB->PrevSignal)
          continue;

        // printf("G%i-%i %i:%i ", group, i, CB->module, CB->id);

        //Previous block
        if(group != 0 && group >= 3){
          if(i == 0)
            PB = prevlist;
          else
            PB = list->B[i-1];
        }
        else if(group > 0){
          if(i < list->blocks - 1)
            PB = list->B[i+1];
          else
            PB = prevlist;
        }
        //Next block
        if(group != 5 && group < 3){
          if(i == 0)
            NB = nextlist;
          else
            NB = list->B[i-1];
        }
        else if(group >= 3){
          if(i < list->blocks - 1)
            NB = list->B[i+1];
          else
            NB = nextlist;
        }

        if(CB->NextSignal){
          if(NB)
            set_signal(CB->NextSignal, NB->state);
          else
            set_signal(CB->NextSignal, DANGER);
        }
        if(CB->PrevSignal){
          if(PB)
            set_signal(CB->PrevSignal, PB->state);
          else
            set_signal(CB->PrevSignal, DANGER);
        }
      }
    }
  }

  // if(BN.blocks > 0){
  //   for(int i = 0; i < BN.blocks; i++){
  //     CB = BN.B[i]
  //     if(BN.B[i]->NextSignal){
  //       if(i == BN.blocks - 1 && BNN.blocks > 0 && (BN.B[i]->state != BNN.B[0]->state || BNN.B[0]->state != BN.B[i]->NextSignal->state)){
  //         set_signal(BN.B[i]->NextSignal, BNN.B[0]->state);
  //       }
  //       else(i < BN.blocks - 1 && (BN.B[i]->state != BN.B[i+1]->state || BN.B[i+1]->state != BN.B[i]->NextSignal->state)){
  //         set_signal(BN.B[i]->NextSignal, BN.B[i+1]->state);
  //       }
  //     }
  //   }
  //   if(BN.B[0]->NextSignal){
  //     if(BNN.blocks > 0){
  //       if(BNN.B[0]->state == DANGER && BN.B[0]->state != DANGER)
  //         printf("Signal Next red %i:%i ",BN.B[0]->module, BN.B[0]->id);
  //     }
  //     printf("Next block Next signal ");
  //   }
  //   else if(BN.B[0]->PrevSignal){
  //     if(B->state == DANGER && BN.B[0]->state != DANGER){
  //       printf("Signal Prev red %i:%i ",BN.B[0]->module, BN.B[0]->id);
  //     }
  //     printf("Next block Prev signal ");
  //   }
  // }

  // if(BP.blocks > 0){
  //   for(int i = 0; i < BP.blocks; i++){
  //     if(BP.B[i]->NextSignal){
  //       if(i == 0 && (BP.B[i]->state != B->state || B->state != BP.B[i]->NextSignal->state))
  //         set_signal(BP.B[i]->NextSignal, B->state);
  //       else if(i > 0 && (BP.B[i]->state != BP.B[i-1]->state || BP.B[i-1]->state != BP.B[i]->NextSignal->state))
  //         set_signal(BP.B[i]->NextSignal, BP.B[i-1]->state);
  //     }
  //     else if(BP.B[i]->PrevSignal){
  //       if(i == BP.blocks - 1 && BPP.blocks > 0 && (BPP.B[0]->state != BP.B[i]->state || BPP.B[i]->state != BP.B[i]->PrevSignal->state))
  //         set_signal(BP.B[i]->PrevSignal, BPP.B[0]->state);
  //       else if(i < BP.blocks - 1 && (BP.B[i]->state != BP.B[i-1]->state || BP.B[i-1]->state != BP.B[i]->PrevSignal->state))
  //         set_signal(BP.B[i]->PrevSignal, BP.B[i-1]->state);
  //     }
  //   }
  // }
  

}


void procces_accessoire(){
  for(int i = 0;i<unit_len;i++){
    #warning FIX, Output_changed into Node_changed
    /*
    if(Units[i] && Units[i]->output_changed){
      printf("Signals of module %i changed\n",i);
      for(int j = 0;j<Units[i]->signal_len;j++){
        if(Units[i]->Sig[j]){
          printf("Signal id: %i\n",Units[i]->Sig[j]->id);
        }
      }

      Units[i]->output_changed = FALSE;
    }
    */
  }
}

int init_connect_Algor(struct ConnectList * List){
  // printf("init_connect_Algor\n");
  int return_value = 0;
  for(int i = 0;i < unit_len;i++){
    if(!Units[i]){
      continue;
    }
    for(int j = 0;j < Units[i]->block_len; j++){
      if(!Units[i]->B[j]){
        continue;
      }

      if(Units[i]->B[j]->next.type == 'C' || Units[i]->B[j]->prev.type == 'C' || Units[i]->B[j]->next.type == 'c' || Units[i]->B[j]->prev.type == 'c'){
        printf("found block %i:%i\n",i,j);
        if(List->list_index <= List->length + 1){
          struct rail_link ** temp = _calloc(List->list_index+8, struct rail_link *);
          for(int q = 0;q < List->list_index;q++){
            temp[q] = List->R_L[q];
          }
          List->R_L = temp;
          List->list_index += 8;
        }
        // printf("write index: %i\n",List->length);
        List->R_L[List->length] = _calloc(1, struct rail_link);
        List->R_L[List->length]->type = 'R';
        List->R_L[List->length++]->p  = Units[i]->B[j];
      }
    }

    for(int j = 0;j < Units[i]->switch_len; j++){
      if(!Units[i]->Sw[j]){
        continue;
      }

      if(Units[i]->Sw[j]->app.type == 'C' || Units[i]->Sw[j]->div.type == 'C' || Units[i]->Sw[j]->str.type == 'C'){
        printf("module %i, switch %i\n",i,j);
        if(List->list_index <= List->length + 1){
          struct rail_link ** temp = _calloc(List->list_index+8, struct rail_link *);
          for(int q = 0;q < List->list_index;q++){
            temp[q] = List->R_L[q];
          }
          List->R_L = temp;
          List->list_index += 8;
        }
        List->R_L[List->length] = _calloc(1, struct rail_link);
        List->R_L[List->length]->type = 'S';
        List->R_L[List->length++]->p  = Units[i]->Sw[j];
      }
    }

    return_value += Units[i]->connections_len;
  }
  return return_value;
}

_Bool find_and_connect(uint8_t ModuleA, char anchor_A, uint8_t ModuleB, char anchor_B){
  //Node shouldn't be connected to the same Module
  if(ModuleA == ModuleB){return FALSE;}

  char typeA = 0;
  char typeB = 0;

  _Bool connected = FALSE;

  // printf("find_and_connect: %i:%i\t\t%i:%i\n",ModuleA,anchor_A,ModuleB,anchor_B);

  for(int Rail = 1;Rail<3;Rail++){
    struct rail_link A;A.p = 0;
    struct rail_link B;B.p = 0;

    //Find Anchor A
    // - Find a Block
      for(int k = 0;k<Units[ModuleA]->block_len;k++){
        if(Units[ModuleA]->B[k]){
          if(Units[ModuleA]->B[k]->prev.type == 'C'){
            // printf(" - A block Prev %i:%i",ModuleA,k);
            if(Units[ModuleA]->B[k]->prev.module == anchor_A && Units[ModuleA]->B[k]->prev.id == Rail){
              // printf("++++++\n");
              A.type = 'R';
              typeA  = 'P';
              A.p = Units[ModuleA]->B[k];
              break;
            }
            // printf("\n");
          }
          else if(Units[ModuleA]->B[k]->next.type == 'C'){
            // printf(" - A block Next %i:%i",ModuleA,k);
            if(Units[ModuleA]->B[k]->next.module == anchor_A && Units[ModuleA]->B[k]->next.id == Rail){
              // printf("++++++\n");
              A.type = 'R';
              typeA  = 'N';
              A.p = Units[ModuleA]->B[k];
              break;
            }
            // printf("\n");
          }
        }
      }
    // - Find a switch
      if(!A.p){
        for(int k = 0;k<Units[ModuleA]->switch_len;k++){
          if(Units[ModuleA]->Sw[k]){
            if(Units[ModuleA]->Sw[k]->app.type == 'C'){
              // printf(" - A Switch App %i:%i",ModuleA,k);
              if(Units[ModuleA]->Sw[k]->app.module == anchor_A && Units[ModuleA]->Sw[k]->app.id == Rail){
                // printf("++++++\n");
                A.type = 'S';
                typeA  = 'A';
                A.p = Units[ModuleA]->Sw[k];
                break;
              }
              // printf("\n");
            }
            else if(Units[ModuleA]->Sw[k]->str.type == 'C'){
              // printf(" - A Switch Str %i:%i",ModuleA,k);
              if(Units[ModuleA]->Sw[k]->str.module == anchor_A && Units[ModuleA]->Sw[k]->str.id == Rail){
                // printf("++++++\n");
                A.type = 'S';
                typeA  = 'S';
                A.p = Units[ModuleA]->Sw[k];
                break;
              }
              // printf("\n");
            }
            else if(Units[ModuleA]->Sw[k]->div.type == 'C'){
              // printf(" - A Switch Div %i:%i",ModuleA,k);
              if(Units[ModuleA]->Sw[k]->div.module == anchor_A && Units[ModuleA]->Sw[k]->div.id == Rail){
                // printf("++++++\n");
                A.type = 'S';
                typeA  = 'D';
                A.p = Units[ModuleA]->Sw[k];
                break;
              }
              // printf("\n");
            }
          }
        }
      }
    // - Find a msswitch
      if(!A.p){
        printf("Mayby a MSwitch, but not IMPLEMENTED!!\n");
        continue;
      }

    //Find Anchor B
    // - Find a block
      for(int k = 0;k<Units[ModuleB]->block_len;k++){
        if(Units[ModuleB]->B[k]){
          if(Units[ModuleB]->B[k]->next.type == 'C'){
            // printf(" - B block Prev %i:%i",ModuleB,k);
            if(Units[ModuleB]->B[k]->next.module == anchor_B && Units[ModuleB]->B[k]->next.id == Rail){
              // printf("++++++\n");
              B.type = 'R';
              typeB  = 'N';
              B.p = Units[ModuleB]->B[k];
              break;
            }
            // printf("\n");
          }
          else if(Units[ModuleB]->B[k]->prev.type == 'C'){
            // printf(" - B block Prev %i:%i",ModuleB,k);
            if(Units[ModuleB]->B[k]->prev.module == anchor_B && Units[ModuleB]->B[k]->prev.id == Rail){
              // printf("++++++\n");
              B.type = 'R';
              typeB  = 'P';
              B.p = Units[ModuleB]->B[k];
              break;
            }
            // printf("\n");
          }
        }
      }
    // - Find a Switch
      if(!B.p){
        for(int k = 0;k<Units[ModuleB]->switch_len;k++){
          if(Units[ModuleB]->Sw[k]){
            if(Units[ModuleB]->Sw[k]->app.type == 'C'){
              // printf(" - B switch App %i:%i",ModuleB,k);
              if(Units[ModuleB]->Sw[k]->app.module == anchor_B && Units[ModuleB]->Sw[k]->app.id == Rail){
                // printf("++++++\n");
                B.type = 'S';
                typeB  = 'A';
                B.p = Units[ModuleB]->Sw[k];
                break;
              }
              // printf("\n");
            }
            else if(Units[ModuleB]->Sw[k]->str.type == 'C'){
              // printf(" - B switch Str %i:%i",ModuleB,k);
              if(Units[ModuleB]->Sw[k]->str.module == anchor_B && Units[ModuleB]->Sw[k]->str.id == Rail){
                // printf("++++++\n");
                B.type = 'S';
                typeB  = 'S';
                B.p = Units[ModuleB]->Sw[k];
                break;
              }
              // printf("\n");
            }
            else if(Units[ModuleB]->Sw[k]->div.type == 'C'){
              // printf(" - B switch Div %i:%i",ModuleB,k);
              if(Units[ModuleB]->Sw[k]->div.module == anchor_B && Units[ModuleB]->Sw[k]->div.id == Rail){
                // printf("++++++\n");
                B.type = 'S';
                typeB  = 'D';
                B.p = Units[ModuleB]->Sw[k];
                break;
              }
              // printf("\n");
            }
          }
        }
      }
    // - Find a MSwitch
      if(!B.p){
        printf("Mayby a MSwitch, but not IMPLEMENTED!!\n");
        continue;
      }

    if(A.type == 'R' && B.type == 'R'){
      printf("Connecting R %i:%i <==> %i:%i R\n",((Block *)A.p)->module,((Block *)A.p)->id,((Block *)B.p)->module,((Block *)B.p)->id);
    }
    else if(A.type == 'S' && B.type == 'R'){
      printf("Connecting S %i:%i <==> %i:%i R\n",((Switch *)A.p)->module,((Switch *)A.p)->id,((Block *)B.p)->module,((Block *)B.p)->id);  
    }
    else if(A.type == 'R' && B.type == 'S'){
      printf("Connecting R %i:%i <==> %i:%i S\n",((Block *)A.p)->module,((Block *)A.p)->id,((Switch *)B.p)->module,((Switch *)B.p)->id);    
    }

    connected = TRUE;

    //Connect
    if(A.type == 'R' && B.type == 'R'){
      if(typeA == 'P'){
        ((Block *)A.p)->prev.module = ((Block *)B.p)->module;
        ((Block *)A.p)->prev.id    = ((Block *)B.p)->id;
        ((Block *)A.p)->prev.type   = 'R';
        ((Block *)B.p)->next.module = ((Block *)A.p)->module;
        ((Block *)B.p)->next.id    = ((Block *)A.p)->id;
        ((Block *)B.p)->next.type   = 'R';
      }
      else{
        ((Block *)A.p)->next.module = ((Block *)B.p)->module;
        ((Block *)A.p)->next.id    = ((Block *)B.p)->id;
        ((Block *)A.p)->next.type   = 'R';
        ((Block *)B.p)->prev.module = ((Block *)A.p)->module;
        ((Block *)B.p)->prev.id    = ((Block *)A.p)->id;
        ((Block *)B.p)->prev.type   = 'R';
      }
    }
    else if(A.type == 'S' && B.type == 'R'){
      if(typeB == 'N'){
        ((Block *)B.p)->next.module = ((Switch *)A.p)->module;
        ((Block *)B.p)->next.id    = ((Switch *)A.p)->id;
        ((Block *)B.p)->next.type   = (typeA == 'A') ? 'S' : 's';
      }
      else{
        ((Block *)B.p)->prev.module = ((Switch *)A.p)->module;
        ((Block *)B.p)->prev.id    = ((Switch *)A.p)->id;
        ((Block *)B.p)->prev.type   = (typeA == 'A') ? 'S' : 's';
      }

      if(typeA == 'A'){
        ((Switch *)A.p)->app.module = ((Block *)B.p)->module;
        ((Switch *)A.p)->app.id    = ((Block *)B.p)->id;
        ((Switch *)A.p)->app.type   = 'R';
      }
      else if(typeA == 'S'){
        ((Switch *)A.p)->str.module = ((Block *)B.p)->module;
        ((Switch *)A.p)->str.id    = ((Block *)B.p)->id;
        ((Switch *)A.p)->str.type   = 'R';
      }
      else if(typeA == 'D'){
        ((Switch *)A.p)->div.module = ((Block *)B.p)->module;
        ((Switch *)A.p)->div.id    = ((Block *)B.p)->id;
        ((Switch *)A.p)->div.type   = 'R';
      }
    }
    else if(A.type == 'R' && B.type == 'S'){
      if(typeA == 'N'){
        ((Block *)A.p)->next.module = ((Switch *)B.p)->module;
        ((Block *)A.p)->next.id    = ((Switch *)B.p)->id;
        ((Block *)A.p)->next.type   = (typeB == 'A') ? 'S' : 's';
      }
      else{
        ((Block *)A.p)->prev.module = ((Switch *)B.p)->module;
        ((Block *)A.p)->prev.id    = ((Switch *)B.p)->id;
        ((Block *)A.p)->prev.type   = (typeB == 'A') ? 'S' : 's';
      }

      if(typeB == 'A'){
        ((Switch *)B.p)->app.module = ((Block *)A.p)->module;
        ((Switch *)B.p)->app.id    = ((Block *)A.p)->id;
        ((Switch *)B.p)->app.type   = 'R';
      }
      else if(typeB == 'S'){
        ((Switch *)B.p)->str.module = ((Block *)A.p)->module;
        ((Switch *)B.p)->str.id    = ((Block *)A.p)->id;
        ((Switch *)B.p)->str.type   = 'R';
      }
      else if(typeB == 'D'){
        ((Switch *)B.p)->div.module = ((Block *)A.p)->module;
        ((Switch *)B.p)->div.id    = ((Block *)A.p)->id;
        ((Switch *)B.p)->div.type   = 'R';
      }
    }
  }

  if(connected && ModuleA && anchor_A && ModuleB && anchor_B){
    Units[ModuleA]->connection[anchor_A-1] = Units[ModuleB];
    Units[ModuleB]->connection[anchor_B-1] = Units[ModuleA];
  }

  return connected;
}

int connect_Algor(struct ConnectList * List){
  struct rail_link * R = 0;

  int value = 0;

  for(int i = 0;i<List->length;i++){
    if(!List->R_L[i]->p){
      continue;
    }
    if(List->R_L[i]->type == 'R'){
      if(((Block *)List->R_L[i]->p)->next.type != 'C' && ((Block *)List->R_L[i]->p)->prev.type != 'C') {
        value++;
        continue;
      }
      // printf("Found block %i:%i %i\t",((Block*)List->R_L[i]->p)->module,((Block*)List->R_L[i]->p)->id,((Block*)List->R_L[i]->p)->blocked);
      if(((Block *)List->R_L[i]->p)->blocked){
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
            ModuleA = ((Block *)R->p)->module;
            ModuleB = ((Block *)List->R_L[i]->p)->module;

            if(((Block *)R->p)->next.type == 'C'){
              anchor_A = ((Block *)R->p)->next.module;
              anchor_B = ((Block *)List->R_L[i]->p)->prev.module;
            }
            else if(((Block *)R->p)->prev.type == 'C'){
              anchor_A = ((Block *)R->p)->prev.module;
              anchor_B = ((Block *)List->R_L[i]->p)->next.module;
            }

            connected = find_and_connect(ModuleA,anchor_A,ModuleB,anchor_B);
          }
          else if(R->type == 'S'){
            ModuleA = ((Switch *)R->p)->module;
            ModuleB = ((Block *)List->R_L[i]->p)->module;
            if(((Block *)List->R_L[i]->p)->next.type == 'C'){
              anchor_B = ((Block *)List->R_L[i]->p)->next.module;
            }
            else{
              anchor_B = ((Block *)List->R_L[i]->p)->prev.module;
            }

            if(((Switch *)R->p)->app.type == 'C'){
              anchor_A = ((Switch *)R->p)->app.module;
            }
            else if(((Switch *)R->p)->str.type == 'C'){
              anchor_A = ((Switch *)R->p)->str.module;
            }
            else if(((Switch *)R->p)->div.type == 'C'){
              anchor_A = ((Switch *)R->p)->div.module;
            } //End Switch approach type

            connected = find_and_connect(ModuleA,anchor_A,ModuleB,anchor_B);
          } // End Switch type

          if(connected){
            WS_Partial_Layout(ModuleA,ModuleB);
            connected = FALSE;
          }
        }
      }
    }
    else if(List->R_L[i]->type == 'S' && ((Switch *)List->R_L[i]->p)->Detection->blocked){
      //Blocked switch
      if(!R){
        R = List->R_L[i];
      }
    }
  }

  value = 0;
  int total = 0;

  for(int i = 0;i < unit_len;i++){
    if(!Units[i]){
      continue;
    }

    for(int j = 0;j<Units[i]->connections_len;j++){
      total++;
      if(Units[i]->connection[j]){
        value++;
      }
    }
  }
  
  if(value == total){
    _SYS_change(STATE_Modules_Coupled, 1);
  }
  
  return value;
}
