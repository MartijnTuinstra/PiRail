#include "mem.h"
#include "logger.h"
#include "system.h"

#include "sim.h"
#include "rail.h"
#include "switch.h"
#include "train.h"
#include "modules.h"

#include "submodule.h"
#include "algorithm.h"
#include "websocket_msg.h"
#include "pathfinding.h"

pthread_mutex_t mutex_lockA;

#define delayA 5000000
#define delayB 5000000
#define OneSec 1000000

#define TRAIN_A_LEN   5 //cm
#define TRAIN_A_SPEED 5 //cm/s

#define TRAIN_B_LEN   5 //cm
#define TRAIN_B_SPEED 5 //cm/s

#define TRAINSIM_INTERVAL_US 50000
#define TRAINSIM_INTERVAL_SEC 0.05

#define JOIN_SIM_INTERVAL 1000

void change_Block(Block * B, enum Rail_states state){
  B->IOchanged = 1;
  B->statechanged = 1;
  Units[B->module]->block_state_changed = 1;
  if (state == BLOCKED)
    B->blocked = 1;
  else
    B->blocked = 0;

  putAlgorQueue(B, 1);
  // process(B, 3);
}

struct train_sim {
  char sim;

  RailTrain * T;
  uint16_t train_length;

  uint8_t dir;
  float posFront;
  float posRear;

  Block * Front;
  uint8_t FrontSpecialCounter;

  uint8_t blocks;
  Block ** B;
};

void train_sim_tick(struct train_sim * t){
  // loggerf(INFO, "train_sim_tick speed %i\t%f\t%f", t->T->speed,t->posFront, t->posRear);

  if(t->posFront <= 0 && t->blocks < 10){
    // Add block
    for(uint8_t i = t->blocks - 1; i >= 0 && i < 10; i--){
      t->B[i + 1] = t->B[i];
    }
    t->blocks++;
    if(t->FrontSpecialCounter){
      t->B[0] = _Next(t->Front, NEXT, ++t->FrontSpecialCounter);
    }
    else if(t->B[1]->Alg.next){
      t->B[0] = t->B[1]->Alg.N[0];
    }
    else{
      SYS_set_state(&SYS->SimA.state, Module_Fail);
      return;
    }
    loggerf(INFO, "%c  Step %02i:%02i", t->sim, t->B[0]->module, t->B[0]->id);
    t->posFront += t->B[0]->length;
    change_Block(t->B[0], BLOCKED);
  }

  if(t->posRear <= 0){
    // Remove block
    t->blocks--;
    change_Block(t->B[t->blocks], PROCEED);
    t->B[t->blocks] = 0;
    t->posRear += t->B[t->blocks - 1]->length;
  }

  // Advance train (km/h -> cm/s) / scale * tick interval (in sec)
  t->posFront -= (t->T->speed / 3.6) * 100 / 160 * TRAINSIM_INTERVAL_SEC;
  t->posRear  -= (t->T->speed / 3.6) * 100 / 160 * TRAINSIM_INTERVAL_SEC;
}

void *TRAIN_SIMA(){
  while(SYS->LC.state != Module_Run){
    usleep(10000);
    if(SYS->LC.state == Module_Fail || SYS->LC.state == Module_STOP){
      SYS_set_state(&SYS->SimA.state, Module_Fail);
      return NULL;
    }
  }

  usleep(10000000);

  Block *B = Units[25]->B[3];

  struct train_sim train;
  train.B = _calloc(10, void *);
  train.sim = 'A';
  train.posFront = 0;
  train.posRear = B->length;
  train.FrontSpecialCounter = 0;
  train.Front = 0;

  train.blocks = 0;
  // train.B[0] = B;

  while(!B->Alg.N[0] || !B->Alg.P[0]){}
  while(B->Alg.N[0]->blocked || B->blocked || B->Alg.P[0]->blocked){} // Wait for space

  change_Block(B, BLOCKED);

  usleep(100000);

  while(!B->train){
      usleep(10000);
  }

  B->train->control = TRAIN_SEMI_AUTO;

  while(!B->train->p){
    usleep(10000);
  }

  train.T = B->train;

  if(train.T->type == TRAIN_ENGINE_TYPE){
    //Engine only
    train.train_length = ((Engines *)train.T->p)->length / 10;
  }
  else{
    //Train
    train.train_length = ((Trains *)train.T->p)->length / 10;
  }
  loggerf(INFO, "train length %icm", train.train_length);

  int32_t len = train.train_length;
  while(len > 0){
    len -= B->length;

    for(uint8_t i = train.blocks - 1; i >= 0 && i < 10; i--){
      train.B[i + 1] = train.B[i];
    }
    train.blocks++;

    change_Block(B, BLOCKED);
    loggerf(INFO, "Add block %i (%02i:%02i)", train.blocks, B->module, B->id);

    if(B->Alg.next){
      train.B[0] = B;
      B = B->Alg.N[0];
    }
  }

  train.posFront -= len;

  SYS_set_state(&SYS->SimA.state, Module_Run);

  while(SYS->SimA.state & Module_Run){
    train_sim_tick(&train);
    usleep(TRAINSIM_INTERVAL_US);
  }

  _free(train.B);
  SYS_set_state(&SYS->SimA.state, Module_STOP);

  return 0;
}

void *TRAIN_SIMB(){
  while(SYS->LC.state != Module_Run){
    usleep(10000);
    if(SYS->LC.state == Module_Fail || SYS->LC.state == Module_STOP){
      SYS_set_state(&SYS->SimB.state, Module_Fail);
      return NULL;
    }
  }

  usleep(11000000);

  Block *B = Units[25]->B[3];

  struct train_sim train;
  train.B = _calloc(10, void *);
  train.sim = 'B';
  train.posFront = 0;
  train.posRear = B->length;
  train.FrontSpecialCounter = 0;
  train.Front = 0;

  train.blocks = 0;
  // train.B[0] = B;

  while(!B->Alg.N[0] || !B->Alg.P[0]){}
  while(B->Alg.N[0]->blocked || B->blocked || B->Alg.P[0]->blocked){} // Wait for space

  change_Block(B, BLOCKED);

  usleep(100000);

  while(!B->train){
      usleep(10000);
  }

  B->train->control = TRAIN_SEMI_AUTO;

  while(!B->train->p){
    usleep(10000);
  }

  train.T = B->train;

  if(train.T->type == TRAIN_ENGINE_TYPE){
    //Engine only
    train.train_length = ((Engines *)train.T->p)->length / 10;
  }
  else{
    //Train
    train.train_length = ((Trains *)train.T->p)->length / 10;
  }
  loggerf(INFO, "train length %icm", train.train_length);

  int32_t len = train.train_length;
  while(len > 0){
    len -= B->length;

    for(uint8_t i = train.blocks - 1; i >= 0 && i < 10; i--){
      train.B[i + 1] = train.B[i];
    }
    train.blocks++;

    change_Block(B, BLOCKED);
    loggerf(INFO, "Add block %i (%02i:%02i)", train.blocks, B->module, B->id);

    if(B->Alg.next){
      train.B[0] = B;
      B = B->Alg.N[0];
    }
  }

  train.posFront -= len;

  SYS_set_state(&SYS->SimB.state, Module_Run);

  while(SYS->SimB.state & Module_Run){
    train_sim_tick(&train);
    usleep(TRAINSIM_INTERVAL_US);
  }

  _free(train.B);

  return 0;
}


int init_connect_Algor(struct ConnectList * List){
  // printf("init_connect_Algor\n");
  int return_value = 0;
  for(int i = 0;i < unit_len;i++){
  if(!Units[i])
    continue;

  for(int j = 0;j < Units[i]->block_len; j++){
    if(!Units[i]->B[j])
      continue;

    if(Units[i]->B[j]->next.type == RAIL_LINK_C || Units[i]->B[j]->prev.type == RAIL_LINK_C){
      printf("found block %i:%i\n",i,j);
      if(List->list_index <= List->length + 1){
        struct rail_link ** temp = _calloc(List->list_index+8, struct rail_link *);
        for(int q = 0;q < List->list_index;q++){
          temp[q] = List->R_L[q];
        }
        _free(List->R_L);
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
    if(!Units[i]->Sw[j])
      continue;

    if(Units[i]->Sw[j]->app.type == RAIL_LINK_C || Units[i]->Sw[j]->div.type == RAIL_LINK_C || Units[i]->Sw[j]->str.type == RAIL_LINK_C){
      printf("module %i, switch %i\n",i,j);
      if(List->list_index <= List->length + 1){
        struct rail_link ** temp = _calloc(List->list_index+8, struct rail_link *);
        for(int q = 0;q < List->list_index;q++){
          temp[q] = List->R_L[q];
        }
        _free(List->R_L);
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

  printf("find_and_connect: %i:%i\t\t%i:%i\n",ModuleA,anchor_A,ModuleB,anchor_B);

  for(int Rail = 1;Rail<3;Rail++){
    struct rail_link A;A.p = 0;
    struct rail_link B;B.p = 0;

    //Find Anchor A
    // - Find a Block
      for(int k = 0;k<Units[ModuleA]->block_len;k++){
      if(Units[ModuleA]->B[k]){
        Block * Bl = Units[ModuleA]->B[k];
          // printf(" - A block %i:%i p%x:%x:%x n%x:%x:%x\n",ModuleA,k, Bl->prev.type, Bl->prev.module, Bl->prev.id, Bl->next.type, Bl->next.module, Bl->next.id);
          if(Bl->prev.type == RAIL_LINK_C && Bl->prev.module == anchor_A && Bl->prev.id == Rail){
            printf("++++++\n");
            A.type = RAIL_LINK_R;
            typeA  = 'P';
            A.p = Bl;
            printf("\n");
            break;
          }
          else if(Bl->next.type == RAIL_LINK_C && Bl->next.module == anchor_A && Bl->next.id == Rail){
            // printf("++++++\n");
            A.type = RAIL_LINK_R;
            typeA  = 'N';
            A.p = Bl;
            printf("\n");
            break;
          }
        }
      }
    // - Find a switch
      if(!A.p){
        for(int k = 0;k<Units[ModuleA]->switch_len;k++){
          if(Units[ModuleA]->Sw[k]){
            Switch * Sw = Units[ModuleA]->Sw[k];
              // printf(" - A Switch %i:%i A%x:%x:%x S%x:%x:%x D%x:%x:%x\n",ModuleA,k, Sw->app.type, Sw->app.module, Sw->app.id, Sw->str.type, Sw->str.module, Sw->str.id, Sw->div.type, Sw->div.module, Sw->div.id);
            if(Sw->app.type == RAIL_LINK_C){
              if(Sw->app.module == anchor_A && Sw->app.id == Rail){
                // printf("++++++\n");
                A.type = RAIL_LINK_S;
                typeA  = 'A';
                A.p = Sw;
                break;
              }
              printf("\n");
            }
            else if(Sw->str.type == RAIL_LINK_C){
              if(Sw->str.module == anchor_A && Sw->str.id == Rail){
                // printf("++++++\n");
                A.type = RAIL_LINK_S;
                typeA  = 'S';
                A.p = Sw;
                break;//dsafas
              }
              printf("\n");
            }
            else if(Sw->div.type == RAIL_LINK_C){
              if(Sw->div.module == anchor_A && Sw->div.id == Rail){
                // printf("++++++\n");
                A.type = RAIL_LINK_S;
                typeA  = 'D';
                A.p = Units[ModuleA]->Sw[k];
                break;
              }
              printf("\n");
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
          if(Units[ModuleB]->B[k]->next.type == RAIL_LINK_C){
          printf(" - B block Prev %i:%i",ModuleB,k);
          if(Units[ModuleB]->B[k]->next.module == anchor_B && Units[ModuleB]->B[k]->next.id == Rail){
            printf("++++++\n");
            B.type = RAIL_LINK_R;
            typeB  = 'N';
            B.p = Units[ModuleB]->B[k];
            break;
          }
          printf("\n");
          }
          else if(Units[ModuleB]->B[k]->prev.type == RAIL_LINK_C){
          printf(" - B block Prev %i:%i",ModuleB,k);
          if(Units[ModuleB]->B[k]->prev.module == anchor_B && Units[ModuleB]->B[k]->prev.id == Rail){
            printf("++++++\n");
            B.type = RAIL_LINK_R;
            typeB  = 'P';
            B.p = Units[ModuleB]->B[k];
            break;
          }
          printf("\n");
          }
        }
      }
    // - Find a Switch
      if(!B.p){
        for(int k = 0;k<Units[ModuleB]->switch_len;k++){
          if(Units[ModuleB]->Sw[k]){
            if(Units[ModuleB]->Sw[k]->app.type == RAIL_LINK_C){
              printf(" - B switch App %i:%i",ModuleB,k);
              if(Units[ModuleB]->Sw[k]->app.module == anchor_B && Units[ModuleB]->Sw[k]->app.id == Rail){
              printf("++++++\n");
              B.type = RAIL_LINK_S;
              typeB  = 'A';
              B.p = Units[ModuleB]->Sw[k];
              break;
              }
              printf("\n");
            }
            else if(Units[ModuleB]->Sw[k]->str.type == RAIL_LINK_C){
              printf(" - B switch Str %i:%i",ModuleB,k);
              if(Units[ModuleB]->Sw[k]->str.module == anchor_B && Units[ModuleB]->Sw[k]->str.id == Rail){
              printf("++++++\n");
              B.type = RAIL_LINK_S;
              typeB  = 'S';
              B.p = Units[ModuleB]->Sw[k];
              break;
              }
              printf("\n");
            }
            else if(Units[ModuleB]->Sw[k]->div.type == RAIL_LINK_C){
              printf(" - B switch Div %i:%i",ModuleB,k);
              if(Units[ModuleB]->Sw[k]->div.module == anchor_B && Units[ModuleB]->Sw[k]->div.id == Rail){
              printf("++++++\n");
              B.type = RAIL_LINK_S;
              typeB  = 'D';
              B.p = Units[ModuleB]->Sw[k];
              break;
              }
              printf("\n");
            }
          }
        }
      }
    // - Find a MSwitch
      if(!B.p){
        printf("Mayby a MSwitch, but not IMPLEMENTED!!\n");
        continue;
      }

    if(A.type == RAIL_LINK_R && B.type == RAIL_LINK_R){
      printf("Connecting R %i:%i <==> %i:%i R\n",((Block *)A.p)->module,((Block *)A.p)->id,((Block *)B.p)->module,((Block *)B.p)->id);
    }
    else if(A.type == RAIL_LINK_S && B.type == RAIL_LINK_R){
      printf("Connecting S %i:%i <==> %i:%i R\n",((Switch *)A.p)->module,((Switch *)A.p)->id,((Block *)B.p)->module,((Block *)B.p)->id);
    }
    else if(A.type == RAIL_LINK_R && B.type == RAIL_LINK_S){
      printf("Connecting R %i:%i <==> %i:%i S\n",((Block *)A.p)->module,((Block *)A.p)->id,((Switch *)B.p)->module,((Switch *)B.p)->id);
    }

    connected = TRUE;

    //Connect
    if(A.type == RAIL_LINK_R && B.type == RAIL_LINK_R){
      if(typeA == 'P'){
        ((Block *)A.p)->prev.module = ((Block *)B.p)->module;
        ((Block *)A.p)->prev.id    = ((Block *)B.p)->id;
        ((Block *)A.p)->prev.type   = RAIL_LINK_R;
        ((Block *)B.p)->next.module = ((Block *)A.p)->module;
        ((Block *)B.p)->next.id    = ((Block *)A.p)->id;
        ((Block *)B.p)->next.type   = RAIL_LINK_R;
      }
      else{
        ((Block *)A.p)->next.module = ((Block *)B.p)->module;
        ((Block *)A.p)->next.id    = ((Block *)B.p)->id;
        ((Block *)A.p)->next.type   = RAIL_LINK_R;
        ((Block *)B.p)->prev.module = ((Block *)A.p)->module;
        ((Block *)B.p)->prev.id    = ((Block *)A.p)->id;
        ((Block *)B.p)->prev.type   = RAIL_LINK_R;
      }
    }
    else if(A.type == RAIL_LINK_S && B.type == RAIL_LINK_R){
      if(typeB == 'N'){
        ((Block *)B.p)->next.module = ((Switch *)A.p)->module;
        ((Block *)B.p)->next.id    = ((Switch *)A.p)->id;
        ((Block *)B.p)->next.type   = (typeA == 'A') ? RAIL_LINK_S : RAIL_LINK_s;
      }
      else{
        ((Block *)B.p)->prev.module = ((Switch *)A.p)->module;
        ((Block *)B.p)->prev.id    = ((Switch *)A.p)->id;
        ((Block *)B.p)->prev.type   = (typeA == 'A') ? RAIL_LINK_S : RAIL_LINK_s;
      }

      if(typeA == 'A'){
        ((Switch *)A.p)->app.module = ((Block *)B.p)->module;
        ((Switch *)A.p)->app.id    = ((Block *)B.p)->id;
        ((Switch *)A.p)->app.type   = RAIL_LINK_R;
      }
      else if(typeA == 'S'){
        ((Switch *)A.p)->str.module = ((Block *)B.p)->module;
        ((Switch *)A.p)->str.id    = ((Block *)B.p)->id;
        ((Switch *)A.p)->str.type   = RAIL_LINK_R;
      }
      else if(typeA == 'D'){
        ((Switch *)A.p)->div.module = ((Block *)B.p)->module;
        ((Switch *)A.p)->div.id    = ((Block *)B.p)->id;
        ((Switch *)A.p)->div.type   = RAIL_LINK_R;
      }
    }
    else if(A.type == RAIL_LINK_R && B.type == RAIL_LINK_S){
      if(typeA == 'N'){
        ((Block *)A.p)->next.module = ((Switch *)B.p)->module;
        ((Block *)A.p)->next.id    = ((Switch *)B.p)->id;
        ((Block *)A.p)->next.type   = (typeB == 'A') ? RAIL_LINK_S : RAIL_LINK_s;
      }
      else{
        ((Block *)A.p)->prev.module = ((Switch *)B.p)->module;
        ((Block *)A.p)->prev.id    = ((Switch *)B.p)->id;
        ((Block *)A.p)->prev.type   = (typeB == 'A') ? RAIL_LINK_S : RAIL_LINK_s;
      }

      if(typeB == 'A'){
        ((Switch *)B.p)->app.module = ((Block *)A.p)->module;
        ((Switch *)B.p)->app.id    = ((Block *)A.p)->id;
        ((Switch *)B.p)->app.type   = RAIL_LINK_R;
      }
      else if(typeB == 'S'){
        ((Switch *)B.p)->str.module = ((Block *)A.p)->module;
        ((Switch *)B.p)->str.id    = ((Block *)A.p)->id;
        ((Switch *)B.p)->str.type   = RAIL_LINK_R;
      }
      else if(typeB == 'D'){
        ((Switch *)B.p)->div.module = ((Block *)A.p)->module;
        ((Switch *)B.p)->div.id    = ((Block *)A.p)->id;
        ((Switch *)B.p)->div.type   = RAIL_LINK_R;
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
    if(!List->R_L[i]->p)
      continue;

    if(List->R_L[i]->type == 'R'){
      if(((Block *)List->R_L[i]->p)->next.type != RAIL_LINK_C && ((Block *)List->R_L[i]->p)->prev.type != RAIL_LINK_C) {
        value++;
        continue;
      }
      if(((Block *)List->R_L[i]->p)->blocked){
        printf("Found block %i:%i %i\t",((Block*)List->R_L[i]->p)->module,((Block*)List->R_L[i]->p)->id,((Block*)List->R_L[i]->p)->blocked);
        //Blocked block
        if(!R)
          R = List->R_L[i];
        else
        {
          _Bool connected = FALSE;
          char anchor_A = 0;
          char anchor_B = 0;

          int ModuleA = 0;
          int ModuleB = 0;
          if(R->type == 'R'){
            ModuleA = ((Block *)R->p)->module;
            ModuleB = ((Block *)List->R_L[i]->p)->module;

            if(((Block *)R->p)->next.type == RAIL_LINK_C){
              anchor_A = ((Block *)R->p)->next.module;
              anchor_B = ((Block *)List->R_L[i]->p)->prev.module;
            }
            else if(((Block *)R->p)->prev.type == RAIL_LINK_C){
              anchor_A = ((Block *)R->p)->prev.module;
              anchor_B = ((Block *)List->R_L[i]->p)->next.module;
            }

            connected = find_and_connect(ModuleA,anchor_A,ModuleB,anchor_B);
          }
          else if(R->type == 'S'){
            ModuleA = ((Switch *)R->p)->module;
            ModuleB = ((Block *)List->R_L[i]->p)->module;
            if(((Block *)List->R_L[i]->p)->next.type == RAIL_LINK_C){
              anchor_B = ((Block *)List->R_L[i]->p)->next.module;
            }
            else{
              anchor_B = ((Block *)List->R_L[i]->p)->prev.module;
            }

            if(((Switch *)R->p)->app.type == RAIL_LINK_C){
              anchor_A = ((Switch *)R->p)->app.module;
            }
            else if(((Switch *)R->p)->str.type == RAIL_LINK_C){
              anchor_A = ((Switch *)R->p)->str.module;
            }
            else if(((Switch *)R->p)->div.type == RAIL_LINK_C){
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
    if(!Units[i])
      continue;

    for(int j = 0;j<Units[i]->connections_len;j++){
      total++;
      if(Units[i]->connection[j]){
      value++;
      }
    }
  }

  if(value == total){
    SYS->modules_linked = 1;
  }

  return value;
}


// int _connect_(struct ConnectList * List){
//   uint8_t Atype = RAIL_LINK_E;
//   void * Ap;
//   uint8_t Btype = RAIL_LINK_E;
//   void * Bp;
//   int iA = -1, iB = -1;

//   for(uint8_t i = 0; i < List->length; i++){
//     if(!List->R_L[i]->p)
//       continue;

//     if(Atype != RAIL_LINK_E && Btype != RAIL_LINK_E)
//       break;

//     if(List->R_L[i]->type == 'R'){
//       Block * B = List->R_L[i]->p;
//       if(B->blocked){
//         printf("Found block %i:%i %i\n",B->module,B->id,B->blocked);
//         if(Atype == RAIL_LINK_E){
//           Atype = RAIL_LINK_R;
//           Ap = B;
//           iA = i;
//         }
//         else{
//           Btype = RAIL_LINK_R;
//           Bp = B;
//           iB = i;
//         }
//       }
//     }
//     else if(List->R_L[i]->type == 'S'){
//       Switch * Sw = List->R_L[i]->p;
//       if(Sw->Detection->blocked){
//         printf("Found Switch %i:%i\n",Sw->module,Sw->id);
//         if(Atype == RAIL_LINK_E){
//           Atype = RAIL_LINK_S;
//           Ap = Sw;
//           iA = i;
//         }
//         else{
//           Btype = RAIL_LINK_S;
//           Bp = Sw;
//           iB = i;
//         }
//       }
//     }
//   }

//   printf("Connect %x => %x", (unsigned int)Ap, (unsigned int)Bp);

//   // Connect
//   find_and_connect(Ap->module, Ap->next.type == RAIL_LINK_C ? Ap->next.id : Ap->prev.id, Bp->module, Bp->next.type == RAIL_LINK_C ? Bp->next.id : Bp->prev.id);

//   // Purge from list
// }

void * rail_link_pointer(struct rail_link link){
  if(!Units[link.module])
    return 0;
  Unit * U = Units[link.module];
  //if(link.module == 0 || Units[link.module] == 0){
  //	  return 0;
  //}
  if(link.type == RAIL_LINK_R && U->B[link.id]){
    return U->B[link.id];
  }
  else if((link.type == RAIL_LINK_S || link.type == RAIL_LINK_s) && U->Sw[link.id]){
    return U->Sw[link.id];
  }
  else if((link.type >= RAIL_LINK_MA && link.type <= RAIL_LINK_mb) && U->MSSw[link.id]){
    return U->MSSw[link.id];
  }
  return 0;
}


void SIM_JoinModules(){
  printf("Ready to join modules\n");

  struct ConnectList List;
  List.length = 0;
  List.list_index = 8;
  List.R_L = _calloc(8, struct rail_link *);


  int i = 0;
  int x = 0;
  int max_j = init_connect_Algor(&List);
  int cur_j = max_j;
  int prev_j = max_j;
  while(SYS->modules_linked == 0){
    cur_j = connect_Algor(&List);
    printf("?\n");
    if(i > 1){
      printf(" (%02i/%02i)\n",cur_j,max_j);
      i = 0;
      x++;
    }
    if(prev_j != cur_j){

      char data[20];
      data[0] = 0x82;
      data[1] = cur_j;
      data[2] = max_j;
      int k = 3;
      for(int j = 0;j<unit_len;j++){
        if(Units[j]){
          data[k++] = j;
        }
      }
      ws_send_all(data,k,0x10);
    }
    i++;
    usleep(JOIN_SIM_INTERVAL);
    prev_j = cur_j;

    if(i == 1){
    usleep(5000);
    if(x == 1){
      Units[20]->B[5]->blocked = 1;
      Units[25]->B[0]->blocked = 1;
      printf("\n1\n");
    }else if(x == 2){
      Units[25]->B[3]->blocked = 1;
      Units[22]->B[0]->blocked = 1;

      Units[20]->B[5]->blocked = 0;
      Units[25]->B[0]->blocked = 0;
      printf("\n2\n");
    }else if(x == 3){
      Units[22]->B[1]->blocked = 1;
      Units[10]->B[0]->blocked = 1;

      Units[25]->B[3]->blocked = 1;
      Units[22]->B[0]->blocked = 1;
      printf("\n3\n");
    }else if(x == 4){
      Units[10]->B[3]->blocked = 1;
      Units[21]->B[0]->blocked = 1;

      Units[22]->B[1]->blocked = 0;
      Units[10]->B[0]->blocked = 0;
      printf("\n4\n");
    }else if(x == 5){
      Units[21]->B[3]->blocked = 1;
      Units[23]->B[0]->blocked = 1;

      Units[10]->B[3]->blocked = 0;
      Units[21]->B[0]->blocked = 0;
      printf("\n5\n");
    }else if(x == 6){
      Units[23]->B[1]->blocked = 1;
      Units[20]->B[0]->blocked = 1;

      Units[21]->B[3]->blocked = 0;
      Units[23]->B[0]->blocked = 0;
      printf("\n6\n");
    }else if(x == 7){
      Units[23]->B[1]->blocked = 0;
      Units[20]->B[0]->blocked = 0;
      printf("\n7\n");
    }else if(x == 6){
      printf("\nend\n");
    }
    else if(x == 10){
      // _SYS_change(STATE_Modules_Coupled,1);
      SYS->modules_linked = 1;
    }
    }
    //IF ALL JOINED
    //BREAK
  }
  
  Units[21]->B[0]->blocked = 0;
  Units[22]->B[1]->blocked = 0;

  for(int i = 0;i<List.length;i++){
    // if(List.R_L[i]->type == 'S'){
    //   if(((Switch *)List.R_L[i]->p)->app.type == RAIL_LINK_C){
    //     ((Switch *)List.R_L[i]->p)->app.type = 0;
    //   }else if(((Switch *)List.R_L[i]->p)->str.type == RAIL_LINK_C){
    //     ((Switch *)List.R_L[i]->p)->str.type = 0;
    //   }else if(((Switch *)List.R_L[i]->p)->div.type == RAIL_LINK_C){
    //     ((Switch *)List.R_L[i]->p)->div.type = 0;
    //   }
    // }else if(((Block *)List.R_L[i]->p)){
    //   if(((Block *)List.R_L[i]->p)->next.type == RAIL_LINK_C){
    //     ((Block *)List.R_L[i]->p)->next.type = 0;
    //   }else if(((Block *)List.R_L[i]->p)->prev.type == RAIL_LINK_C){
    //     ((Block *)List.R_L[i]->p)->prev.type = 0;
    //   }
    // }

    _free(List.R_L[i]);
  }
  _free(List.R_L);

  for(uint8_t i = 0; i < unit_len; i++){
    if(!Units[i])
      continue;

    for(uint8_t j = 0; j < Units[i]->block_len; j++){
      if(Units[i]->B[j]){
        Units[i]->B[j]->blocked = 0;
      }
    }
  }

  // WS_Track_Layout();
}

void SIM_Connect_Rail_links(){
  // add pointer to the rail_link
  for(int m = 0; m<unit_len; m++){
    if(!Units[m]){
      continue;
    }

    loggerf(INFO, "LINKING UNIT %i", m);

    Unit * tU = Units[m];

    for(int i = 0; i<tU->block_len; i++){
      if(!tU->B[i]){
        continue;
      }

      Block * tB = tU->B[i];

      tB->next.p = rail_link_pointer(tB->next);
      tB->prev.p = rail_link_pointer(tB->prev);
    }

    for(int i = 0; i<tU->switch_len; i++){
      if(!tU->Sw[i]){
        continue;
      }

      Switch * tSw = tU->Sw[i];

      tSw->app.p = rail_link_pointer(tSw->app);
      tSw->str.p = rail_link_pointer(tSw->str);
      tSw->div.p = rail_link_pointer(tSw->div);
    }

    for(int i = 0; i<tU->msswitch_len; i++){
      if(!tU->MSSw[i]){
        continue;
      }

      MSSwitch * tMSSw = tU->MSSw[i];

      for(int s = 0; s < tMSSw->state_len; s++){
        tMSSw->sideA[s].p = rail_link_pointer(tMSSw->sideA[s]);
        tMSSw->sideB[s].p = rail_link_pointer(tMSSw->sideB[s]);
      }
    }
  }
}

void SIM_Client_Connect_cb(){
  // SimA_start();
  // SimB_start();
  
  Algor_start();
  while(SYS->LC.state != Module_Run){}
  struct paths return_value = pathfinding(U_B(20,8), U_B(20,14));
  if(return_value.forward || return_value.reverse)
    printf("CHEERS");
  // pathfinding_print(instr);
  free_pathinstructions(return_value.forward);
  free_pathinstructions(return_value.reverse);
}
