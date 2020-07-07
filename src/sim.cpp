#include "mem.h"
#include "logger.h"
#include "system.h"

#include "sim.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "train.h"
#include "modules.h"

#include "submodule.h"
#include "algorithm.h"
#include "websocket/stc.h"
#include "pathfinding.h"

extern pthread_mutex_t mutex_lockA;

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
      t->B[0] = t->Front->_Next(NEXT, ++t->FrontSpecialCounter);
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

void *TRAIN_SIMA(void * args){
  while(SYS->LC.state != Module_Run){
    usleep(10000);
    if(SYS->LC.state == Module_Fail || SYS->LC.state == Module_STOP){
      SYS_set_state(&SYS->SimA.state, Module_Fail);
      return NULL;
    }
  }

  usleep(100000);

  Block *B = Units[25]->B[3];

  struct train_sim train;
  train.B = (Block **)_calloc(10, Block *);
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

  while(!B->train->p.p){
    usleep(10000);
  }

  train.T = B->train;

  if(train.T->type == RAILTRAIN_ENGINE_TYPE){
    //Engine only
    train.train_length = train.T->p.E->length / 10;
  }
  else{
    //Train
    train.train_length = train.T->p.T->length / 10;
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

void *TRAIN_SIMB(void * args){
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
  train.B = (Block **)_calloc(10, Block *);
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

  while(!B->train->p.p){
    usleep(10000);
  }

  train.T = B->train;

  if(train.T->type == RAILTRAIN_ENGINE_TYPE){
    //Engine only
    train.train_length = train.T->p.E->length / 10;
  }
  else{
    //Train
    train.train_length = train.T->p.T->length / 10;
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
          struct rail_link ** temp = (struct rail_link **)_calloc(List->list_index+8, struct rail_link *);
          for(int q = 0;q < List->list_index;q++){
            temp[q] = List->R_L[q];
          }
          _free(List->R_L);
          List->R_L = temp;
          List->list_index += 8;
        }
        // printf("write index: %i\n",List->length);
        List->R_L[List->length] = (struct rail_link *)_calloc(1, struct rail_link);
        List->R_L[List->length]->type  = (enum link_types)'R';
        List->R_L[List->length++]->p.B = Units[i]->B[j];
      }
    }

    for(int j = 0;j < Units[i]->switch_len; j++){
      if(!Units[i]->Sw[j])
        continue;

      if(Units[i]->Sw[j]->app.type == RAIL_LINK_C || Units[i]->Sw[j]->div.type == RAIL_LINK_C || Units[i]->Sw[j]->str.type == RAIL_LINK_C){
        printf("module %i, switch %i\n",i,j);
        if(List->list_index <= List->length + 1){
          struct rail_link ** temp = (struct rail_link **)_calloc(List->list_index+8, struct rail_link *);
          for(int q = 0;q < List->list_index;q++){
            temp[q] = List->R_L[q];
          }
          _free(List->R_L);
          List->R_L = temp;
          List->list_index += 8;
        }
        List->R_L[List->length] = (struct rail_link *)_calloc(1, struct rail_link);
        List->R_L[List->length]->type   = (enum link_types)'S';
        List->R_L[List->length++]->p.Sw = Units[i]->Sw[j];
      }
    }

    return_value += Units[i]->connections_len;
  }
  return return_value;
}

bool find_and_connect(uint8_t ModuleA, char anchor_A, uint8_t ModuleB, char anchor_B){
  //Node shouldn't be connected to the same Module
  if(ModuleA == ModuleB){return FALSE;}

  char typeA = 0;
  char typeB = 0;

  bool connected = FALSE;

  printf("find_and_connect: %i:%i\t\t%i:%i\n",ModuleA,anchor_A,ModuleB,anchor_B);

  for(int Rail = 1;Rail<3;Rail++){
    struct rail_link A;A.p.p = 0;
    struct rail_link B;B.p.p = 0;

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
            A.p.B = Bl;
            printf("\n");
            break;
          }
          else if(Bl->next.type == RAIL_LINK_C && Bl->next.module == anchor_A && Bl->next.id == Rail){
            // printf("++++++\n");
            A.type = RAIL_LINK_R;
            typeA  = 'N';
            A.p.B = Bl;
            printf("\n");
            break;
          }
        }
      }
    // - Find a switch
      if(!A.p.p){
        for(int k = 0;k<Units[ModuleA]->switch_len;k++){
          if(Units[ModuleA]->Sw[k]){
            Switch * Sw = Units[ModuleA]->Sw[k];
              // printf(" - A Switch %i:%i A%x:%x:%x S%x:%x:%x D%x:%x:%x\n",ModuleA,k, Sw->app.type, Sw->app.module, Sw->app.id, Sw->str.type, Sw->str.module, Sw->str.id, Sw->div.type, Sw->div.module, Sw->div.id);
            if(Sw->app.type == RAIL_LINK_C){
              if(Sw->app.module == anchor_A && Sw->app.id == Rail){
                // printf("++++++\n");
                A.type = RAIL_LINK_S;
                typeA  = 'A';
                A.p.Sw = Sw;
                break;
              }
              printf("\n");
            }
            else if(Sw->str.type == RAIL_LINK_C){
              if(Sw->str.module == anchor_A && Sw->str.id == Rail){
                // printf("++++++\n");
                A.type = RAIL_LINK_S;
                typeA  = 'S';
                A.p.Sw = Sw;
                break;//dsafas
              }
              printf("\n");
            }
            else if(Sw->div.type == RAIL_LINK_C){
              if(Sw->div.module == anchor_A && Sw->div.id == Rail){
                // printf("++++++\n");
                A.type = RAIL_LINK_S;
                typeA  = 'D';
                A.p.Sw = Units[ModuleA]->Sw[k];
                break;
              }
              printf("\n");
            }
          }
        }
      }
    // - Find a msswitch
      if(!A.p.p){
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
            B.p.B = Units[ModuleB]->B[k];
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
            B.p.B = Units[ModuleB]->B[k];
            break;
          }
          printf("\n");
          }
        }
      }
    // - Find a Switch
      if(!B.p.p){
        for(int k = 0;k<Units[ModuleB]->switch_len;k++){
          if(Units[ModuleB]->Sw[k]){
            if(Units[ModuleB]->Sw[k]->app.type == RAIL_LINK_C){
              printf(" - B switch App %i:%i",ModuleB,k);
              if(Units[ModuleB]->Sw[k]->app.module == anchor_B && Units[ModuleB]->Sw[k]->app.id == Rail){
              printf("++++++\n");
              B.type = RAIL_LINK_S;
              typeB  = 'A';
              B.p.Sw = Units[ModuleB]->Sw[k];
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
              B.p.Sw = Units[ModuleB]->Sw[k];
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
              B.p.Sw = Units[ModuleB]->Sw[k];
              break;
              }
              printf("\n");
            }
          }
        }
      }
    // - Find a MSwitch
      if(!B.p.p){
        printf("Mayby a MSwitch, but not IMPLEMENTED!!\n");
        continue;
      }

    if(A.type == RAIL_LINK_R && B.type == RAIL_LINK_R){
      printf("Connecting R %i:%i <==> %i:%i R\n",A.p.B->module,A.p.B->id,B.p.B->module,B.p.B->id);
    }
    else if(A.type == RAIL_LINK_S && B.type == RAIL_LINK_R){
      printf("Connecting S %i:%i <==> %i:%i R\n",A.p.Sw->module,A.p.Sw->id,B.p.B->module,B.p.B->id);
    }
    else if(A.type == RAIL_LINK_R && B.type == RAIL_LINK_S){
      printf("Connecting R %i:%i <==> %i:%i S\n",A.p.B->module,A.p.B->id,B.p.Sw->module,B.p.Sw->id);
    }

    connected = TRUE;

    //Connect
    if(A.type == RAIL_LINK_R && B.type == RAIL_LINK_R){
      if(typeA == 'P'){
        A.p.B->prev.module = B.p.B->module;
        A.p.B->prev.id    = B.p.B->id;
        A.p.B->prev.type   = RAIL_LINK_R;
        B.p.B->next.module = A.p.B->module;
        B.p.B->next.id    = A.p.B->id;
        B.p.B->next.type   = RAIL_LINK_R;
      }
      else{
        A.p.B->next.module = B.p.B->module;
        A.p.B->next.id    = B.p.B->id;
        A.p.B->next.type   = RAIL_LINK_R;
        B.p.B->prev.module = A.p.B->module;
        B.p.B->prev.id    = A.p.B->id;
        B.p.B->prev.type   = RAIL_LINK_R;
      }
    }
    else if(A.type == RAIL_LINK_S && B.type == RAIL_LINK_R){
      if(typeB == 'N'){
        B.p.B->next.module = A.p.Sw->module;
        B.p.B->next.id    = A.p.Sw->id;
        B.p.B->next.type   = (typeA == 'A') ? RAIL_LINK_S : RAIL_LINK_s;
      }
      else{
        B.p.B->prev.module = A.p.Sw->module;
        B.p.B->prev.id    = A.p.Sw->id;
        B.p.B->prev.type   = (typeA == 'A') ? RAIL_LINK_S : RAIL_LINK_s;
      }

      if(typeA == 'A'){
        A.p.Sw->app.module = B.p.B->module;
        A.p.Sw->app.id    = B.p.B->id;
        A.p.Sw->app.type   = RAIL_LINK_R;
      }
      else if(typeA == 'S'){
        A.p.Sw->str.module = B.p.B->module;
        A.p.Sw->str.id    = B.p.B->id;
        A.p.Sw->str.type   = RAIL_LINK_R;
      }
      else if(typeA == 'D'){
        A.p.Sw->div.module = B.p.B->module;
        A.p.Sw->div.id    = B.p.B->id;
        A.p.Sw->div.type   = RAIL_LINK_R;
      }
    }
    else if(A.type == RAIL_LINK_R && B.type == RAIL_LINK_S){
      if(typeA == 'N'){
        A.p.B->next.module = B.p.Sw->module;
        A.p.B->next.id    = B.p.Sw->id;
        A.p.B->next.type   = (typeB == 'A') ? RAIL_LINK_S : RAIL_LINK_s;
      }
      else{
        A.p.B->prev.module = B.p.Sw->module;
        A.p.B->prev.id    = B.p.Sw->id;
        A.p.B->prev.type   = (typeB == 'A') ? RAIL_LINK_S : RAIL_LINK_s;
      }

      if(typeB == 'A'){
        B.p.Sw->app.module = A.p.B->module;
        B.p.Sw->app.id    = A.p.B->id;
        B.p.Sw->app.type   = RAIL_LINK_R;
      }
      else if(typeB == 'S'){
        B.p.Sw->str.module = A.p.B->module;
        B.p.Sw->str.id    = A.p.B->id;
        B.p.Sw->str.type   = RAIL_LINK_R;
      }
      else if(typeB == 'D'){
        B.p.Sw->div.module = A.p.B->module;
        B.p.Sw->div.id    = A.p.B->id;
        B.p.Sw->div.type   = RAIL_LINK_R;
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
    if(!List->R_L[i]->p.p)
      continue;

    if(List->R_L[i]->type == 'R'){
      if(List->R_L[i]->p.B->next.type != RAIL_LINK_C && List->R_L[i]->p.B->prev.type != RAIL_LINK_C) {
        value++;
        continue;
      }
      if(List->R_L[i]->p.B->blocked){
        printf("Found block %i:%i %i\t", List->R_L[i]->p.B->module, List->R_L[i]->p.B->id, List->R_L[i]->p.B->blocked);
        //Blocked block
        if(!R)
          R = List->R_L[i];
        else
        {
          bool connected = FALSE;
          char anchor_A = 0;
          char anchor_B = 0;

          int ModuleA = 0;
          int ModuleB = 0;
          if(R->type == 'R'){
            ModuleA = R->p.B->module;
            ModuleB = List->R_L[i]->p.B->module;

            if(R->p.B->next.type == RAIL_LINK_C){
              anchor_A = R->p.B->next.module;
              anchor_B = List->R_L[i]->p.B->prev.module;
            }
            else if(R->p.B->prev.type == RAIL_LINK_C){
              anchor_A = R->p.B->prev.module;
              anchor_B = List->R_L[i]->p.B->next.module;
            }

            connected = find_and_connect(ModuleA,anchor_A,ModuleB,anchor_B);
          }
          else if(R->type == 'S'){
            ModuleA = R->p.Sw->module;
            ModuleB = List->R_L[i]->p.B->module;
            if(List->R_L[i]->p.B->next.type == RAIL_LINK_C){
              anchor_B = List->R_L[i]->p.B->next.module;
            }
            else{
              anchor_B = List->R_L[i]->p.B->prev.module;
            }

            if(R->p.Sw->app.type == RAIL_LINK_C){
              anchor_A = R->p.Sw->app.module;
            }
            else if(R->p.Sw->str.type == RAIL_LINK_C){
              anchor_A = R->p.Sw->str.module;
            }
            else if(R->p.Sw->div.type == RAIL_LINK_C){
              anchor_A = R->p.Sw->div.module;
            } //End Switch approach type

            connected = find_and_connect(ModuleA,anchor_A,ModuleB,anchor_B);
          } // End Switch type

          if(connected){
            WS_stc_Partial_Layout(ModuleA);
            WS_stc_Partial_Layout(ModuleB);
            connected = FALSE;
          }
        }
      }
    }
    else if(List->R_L[i]->type == 'S' && List->R_L[i]->p.Sw->Detection->blocked){
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


void SIM_JoinModules(){
  Units[10]->on_layout = 1;
  Units[20]->on_layout = 1;
  Units[21]->on_layout = 1;
  Units[22]->on_layout = 1;
  Units[23]->on_layout = 1;
  Units[25]->on_layout = 1;

  WS_stc_Track_Layout(0);
  printf("Ready to join modules\n");

  struct ConnectList List;
  List.length = 0;
  List.list_index = 8;
  List.R_L = (struct rail_link **)_calloc(8, struct rail_link *);


  int i = 0;
  int x = 0;
  int max_j = init_connect_Algor(&List);
  int cur_j = max_j;
  int prev_j = max_j;
  while(SYS->modules_linked == 0){
    WS_stc_trackUpdate(0);
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
      WSServer->send_all(data, k, 0x10);
    }
    i++;
    usleep(JOIN_SIM_INTERVAL);
    prev_j = cur_j;

    if(i == 1){
    usleep(50000);
    if(x == 1){
      Units[20]->block_state_changed = 1;
      Units[20]->B[5]->IOchanged = 1;
      Units[25]->block_state_changed = 1;
      Units[25]->B[0]->IOchanged = 1;

      Units[20]->B[5]->blocked = 1;
      Units[25]->B[0]->blocked = 1;
      Units[20]->B[5]->state = BLOCKED;
      Units[25]->B[0]->state = BLOCKED;
      printf("\n1\n");
    }else if(x == 2){
      Units[20]->block_state_changed = 1;
      Units[20]->B[5]->IOchanged = 1;
      Units[25]->block_state_changed = 1;
      Units[25]->B[0]->IOchanged = 1;
      
      Units[25]->block_state_changed = 1;
      Units[25]->B[3]->IOchanged = 1;
      Units[22]->block_state_changed = 1;
      Units[22]->B[0]->IOchanged = 1;

      Units[25]->B[3]->blocked = 1;
      Units[22]->B[0]->blocked = 1;
      Units[25]->B[3]->state = BLOCKED;
      Units[22]->B[0]->state = BLOCKED;

      Units[20]->B[5]->blocked = 0;
      Units[25]->B[0]->blocked = 0;
      Units[20]->B[5]->state = PROCEED;
      Units[25]->B[0]->state = PROCEED;
      printf("\n2\n");
    }else if(x == 3){
      Units[25]->block_state_changed = 1;
      Units[25]->B[3]->IOchanged = 1;
      Units[22]->block_state_changed = 1;
      Units[22]->B[0]->IOchanged = 1;

      Units[22]->block_state_changed = 1;
      Units[22]->B[1]->IOchanged = 1;
      Units[10]->block_state_changed = 1;
      Units[10]->B[0]->IOchanged = 1;

      Units[22]->B[1]->blocked = 1;
      Units[10]->B[0]->blocked = 1;
      Units[22]->B[1]->state = BLOCKED;
      Units[10]->B[0]->state = BLOCKED;

      Units[25]->B[3]->blocked = 0;
      Units[22]->B[0]->blocked = 0;
      Units[25]->B[3]->state = PROCEED;
      Units[22]->B[0]->state = PROCEED;
      printf("\n3\n");
    }else if(x == 4){
      Units[22]->block_state_changed = 1;
      Units[22]->B[1]->IOchanged = 1;
      Units[10]->block_state_changed = 1;
      Units[10]->B[0]->IOchanged = 1;

      Units[10]->block_state_changed = 1;
      Units[10]->B[3]->IOchanged = 1;
      Units[21]->block_state_changed = 1;
      Units[21]->B[0]->IOchanged = 1;

      Units[10]->B[3]->blocked = 1;
      Units[21]->B[0]->blocked = 1;
      Units[10]->B[3]->state = BLOCKED;
      Units[21]->B[0]->state = BLOCKED;

      Units[22]->B[1]->blocked = 0;
      Units[10]->B[0]->blocked = 0;
      Units[22]->B[1]->state = PROCEED;
      Units[10]->B[0]->state = PROCEED;
      printf("\n4\n");
    }else if(x == 5){
      Units[10]->block_state_changed = 1;
      Units[10]->B[3]->IOchanged = 1;
      Units[21]->block_state_changed = 1;
      Units[21]->B[0]->IOchanged = 1;
      
      Units[21]->block_state_changed = 1;
      Units[21]->B[3]->IOchanged = 1;
      Units[23]->block_state_changed = 1;
      Units[23]->B[0]->IOchanged = 1;

      Units[21]->B[3]->blocked = 1;
      Units[23]->B[0]->blocked = 1;
      Units[21]->B[3]->state = BLOCKED;
      Units[23]->B[0]->state = BLOCKED;

      Units[10]->B[3]->blocked = 0;
      Units[21]->B[0]->blocked = 0;
      Units[10]->B[3]->state = PROCEED;
      Units[21]->B[0]->state = PROCEED;
      printf("\n5\n");
    }else if(x == 6){
      Units[21]->block_state_changed = 1;
      Units[21]->B[3]->IOchanged = 1;
      Units[23]->block_state_changed = 1;
      Units[23]->B[0]->IOchanged = 1;

      Units[23]->block_state_changed = 1;
      Units[23]->B[1]->IOchanged = 1;
      Units[20]->block_state_changed = 1;
      Units[20]->B[0]->IOchanged = 1;

      Units[23]->B[1]->blocked = 1;
      Units[20]->B[0]->blocked = 1;
      Units[23]->B[1]->state = BLOCKED;
      Units[20]->B[0]->state = BLOCKED;

      Units[21]->B[3]->blocked = 0;
      Units[23]->B[0]->blocked = 0;
      Units[21]->B[3]->state = PROCEED;
      Units[23]->B[0]->state = PROCEED;
      printf("\n6\n");
    }else if(x == 7){
      Units[23]->block_state_changed = 1;
      Units[23]->B[1]->IOchanged = 1;
      Units[20]->block_state_changed = 1;
      Units[20]->B[0]->IOchanged = 1;

      Units[23]->B[1]->blocked = 0;
      Units[20]->B[0]->blocked = 0;
      Units[23]->B[1]->state = PROCEED;
      Units[20]->B[0]->state = PROCEED;
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

  // WS_stc_Track_Layout();
}

void SIM_Connect_Rail_links(){
  // add pointer to the rail_link
  pathlist.clear();

  for(int m = 0; m<unit_len; m++){
    if(!Units[m]){
      continue;
    }

    loggerf(INFO, "LINKING UNIT %i", m);

    Unit * tU = Units[m];

    link_all_blocks(tU);
    link_all_switches(tU);
    link_all_msswitches(tU);
  }

  pathlist_find();
}

void SIM_Client_Connect_cb(){
  // SimA_start();
  // SimB_start();
  Algor_start();
  // if(SYS->LC.state == Module_STOP){
  //   Algor_start();
  //   while(SYS->LC.state != Module_Run){}
  //   struct paths return_value = pathfinding(U_B(20,8), U_B(20,14));
  //   if(return_value.forward || return_value.reverse)
  //     printf("CHEERS");
  //   // pathfinding_print(instr);
  //   free_pathinstructions(return_value.forward);
  //   free_pathinstructions(return_value.reverse);
  // }
  loggerf(INFO, "Done SIM_Client_Connect_cb");
}
