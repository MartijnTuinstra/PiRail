#include "system.h"
#include "logger.h"

#include "train.h"

#include "module.h"

#include "algorithm.h"

#include "websocket_msg.h"
#include "websocket_control.h"

int unit_len;
Unit ** Units;

struct rail_link EMPTY_BL(){
  struct rail_link A;
  A.module = 0;
  A.id = 0;
  A.type = 'e';
  return A;
}

struct rail_link CAdr(int module, int id, char type){
  struct rail_link A;
  A.module = module;
  A.id = id;
  A.type = type;
  loggerf(INFO, "CAdr DEPRICATED");
  return A;
}

void setup_JSON(int arr[], int arr2[], int size, int size2){
  char setup_data[100];

  setup_data[0] = 2;
  int setup_data_l = 2 + size + size2;

  int i = 2;

  for(i;(i-2)<size;i++){
    setup_data[i] = arr[i-2];
  }

  if(size2 != 0){
    setup_data[1] = size;

    for(i;(i-2-size)<size2;i++){
      setup_data[i] = arr2[i-2-size];
    }
  }
}

void Create_Unit(int Module,int OUT,int IN,char points){
  Unit * Z = _calloc(1, Unit);

  if(Module < unit_len){
    Units[Module] = Z;
  }
  else{
    loggerf(ERROR, "NEED TO EXPAND UNITS");
    return;
  }

  Z->Module = Module;

  struct rail_link ** A = _calloc( IN,struct rail_link);
  struct rail_link ** B = _calloc(OUT,struct rail_link);

  Z->connections_len = points;
  Z->connection = _calloc(points, Unit *);

  Z->block_len = 8;
  Z->B = _calloc(Z->block_len, Block);

  Z->switch_len = 8;
  Z->Sw = _calloc(Z->switch_len, Switch);

  Z->msswitch_len = 8;
  Z->MSSw = _calloc(Z->switch_len, MSSwitch);

  Z->signal_len = 8;
  Z->Sig = _calloc(Z->signal_len, Signal);

  Z->input_regs = (IN/8)+1;
  // Z->In = A;
  Z->output_regs = (OUT/8)+1;
  // Z->Out = B;
  IN--;OUT--;//To make the division round down;
  Z->InRegs    = _calloc(( IN/8)+1, char);
  Z->OutRegs   = _calloc((OUT/8)+1, char);
  Z->BlinkMask = _calloc((OUT/8)+1, char);
  Z->output_link = _calloc(((OUT/8)+1) * 8, gpio_link);
}

void join(struct rail_link Adr, struct rail_link link){
  printf("LINK %c%i:%i => %c%i:%i\t",Adr.type,Adr.module,Adr.id,link.type,link.module,link.id);
  if(Adr.type == 'R' && Units[Adr.module]->B[Adr.id]){
    Units[Adr.module]->B[Adr.id]->prev = link;
    printf("D\n");
  }else if(Adr.type == 'S'){
    Units[Adr.module]->Sw[Adr.id]->app = link;
  }else if(Adr.type == 's'){
    loggerf(ERROR, "FIX block_adrcmp");
    // if(block_adrcmp(Units[Adr.module]->Sw[Adr.id]->div, 0)){
    //   Units[Adr.module]->Sw[Adr.id]->div = link;
    // }else{
    //   Units[Adr.module]->Sw[Adr.id]->str = link;
    // }
  }
}

void init_modules(){
  Units = _calloc(30, Unit *);
  unit_len = 30;
}

void clear_Modules(){

  for(int i = 0;i<unit_len;i++){
    if(Units[i]){
      printf("Clearing module %i\n",i);

      //clear Segments
      loggerf(DEBUG,"Clear segments (%i)",Units[i]->block_len);
      for(int j = 0;j<=Units[i]->block_len;j++){
        if(Units[i]->B[j]){
        printf("- Block %i\n",j);
        free(Units[i]->B[j]);
        Units[i]->B[j] = NULL;
      }
      }
      //clear Switches
      loggerf(DEBUG,"Clear switches (%i)",Units[i]->switch_len);
      for(int j = 0;j<=Units[i]->switch_len;j++){
        if(Units[i]->Sw[j]){
          printf("- Switch %i\n",j);
          free(Units[i]->Sw[j]);
          Units[i]->Sw[j] = NULL;
        }
      }
      //clear Mods
      /*
      for(int j = 0;j<=Units[i]->M_L;j++){
        if(Units[i]->Mod[j]){
        printf("- Mod %i\n",j);
          free(Units[i]->M[j]);
          Units[i]->M[j] = NULL;
        }
      }*/
      //clear Signals
      for(int j = 0;j<=Units[i]->signal_len;j++){
        printf("- Signal %i\n",j);
        free(Units[i]->Sig[j]);
        Units[i]->Sig[j] = NULL;
      }
      //clear Stations
      for(int j = 0;j<=Units[i]->station_len;j++){
        printf("- Station %i\n",j);
        free(Units[i]->St[j]);
        Units[i]->St[j] = NULL;
      }

      printf("- Unit %i\n",i);
      free(Units[i]);
      Units[i] = 0;
      printf("\t Cleared!\n");
    }
  }
}

void Modules(int m){
  loggerf(CRITICAL, "DEPRICATED");
  return;
  // //Loop Left
  // struct link link;
  // link.Adr3 = EMPTY_BL();
  // link.Adr4 = EMPTY_BL();

  // int Seg_i = 0;
  // int Swi_i = 0;
  // int Sig_i = 0;

  // if(m == 1){
  //   Create_Unit(m,8,8,2);

  //   /*      ,-1--.
  //          |     `--0--
  //   1 0    \-.--2---3--
  //           ||
  //   Switch
  //   0   0--1
  //   1   2--3
  //   */
  //   link.Adr1.Module = m;link.Adr1.Adr = 0;link.Adr1.type = 'R';
  //   link.Adr2.Module = m;link.Adr2.Adr = 3;link.Adr2.type = 'R';
  //   //void Create_Segment(int Unit_Adr, struct adr Adr,struct adr NAdr,struct adr PAdr  ,char max_speed,char state,char dir,char len);
  //   //void Create_Segment(struct SegC Adr,char type, struct SegC Next, struct SegC Prev,char max_speed,char state,char dir,char len);
  //   join(IN.Adr1,CAdr(m,1,'s'));
  //   join(IN.Adr2,CAdr(m,0,'s'));

  //   Create_Segment(0,CAdr(m,0,'R'),CAdr(m,1,'R'),EMPTY_BL(),     speed_A,0,0,100);
  //   Create_Segment(1,CAdr(m,1,'R'),CAdr(m,1,'S'),CAdr(m,0,'R'),speed_A,0,2,100);
  //   Create_Segment(2,CAdr(m,2,'R'),CAdr(m,0,'S'),CAdr(m,3,'R'),speed_A,0,1,100);
  //   Create_Segment(3,CAdr(m,3,'R'),CAdr(m,2,'R'),EMPTY_BL(),     speed_A,0,1,100);

  //   //LINK
  //   Create_Switch(CAdr(m,1,2),CAdr(m,1,'R'),CAdr(m,0,'s'),IN.Adr1,(int [2]){2,3},1);
  //   Create_Switch(CAdr(m,0,2),CAdr(m,2,'R'),CAdr(m,1,'s'),IN.Adr2,(int [2]){0,1},0);

  //   Units[m]->S[0]->Detection_Block = Units[m]->B[2];
  //   Units[m]->S[1]->Detection_Block = Units[m]->B[1];
  // }
  // else if(m == 2){
  //   Create_Unit(m,8,8,2);

  //   /*      ,--1-.
  //     --0--'     |
  //     --3---2-.-/         0 1
  //             ||
  //   */
  //   link.Adr1.Module = m;link.Adr1.Adr = 1;link.Adr1.type = 's';
  //   link.Adr2.Module = m;link.Adr2.Adr = 0;link.Adr2.type = 's';
  //   //void Create_Segment(int Unit_Adr, struct adr Adr,struct adr NAdr,struct adr PAdr  ,char max_speed,char state,char dir,char len);
  //   //void Create_Segment(struct SegC Adr,char type, struct SegC Next, struct SegC Prev,char max_speed,char state,char dir,char len);

  //   join(IN.Adr1,CAdr(m,0,'R'));
  //   join(IN.Adr2,CAdr(m,3,'R'));

  //   Create_Segment(0,CAdr(m,0,'R'),IN.Adr1,CAdr(m,1,'R'),speed_A,0,0,100);
  //   Create_Segment(1,CAdr(m,1,'R'),CAdr(m,0,'R'),CAdr(m,1,'S'),speed_A,0,2,100);
  //   Create_Segment(2,CAdr(m,2,'R'),CAdr(m,3,'R'),CAdr(m,0,'S'),speed_A,0,1,100);
  //   Create_Segment(3,CAdr(m,3,'R'),IN.Adr2,CAdr(m,2,'R'),speed_A,0,1,100);

  //   Create_Switch(CAdr(m,0,2),CAdr(m,2,'R'),CAdr(m,1,'s'),EMPTY_BL(),(int [2]){0,1},1);
  //   Create_Switch(CAdr(m,1,2),CAdr(m,1,'R'),CAdr(m,0,'s'),EMPTY_BL(),(int [2]){2,3},1);

  //   Units[m]->S[0]->Detection_Block = Units[m]->B[2];
  //   Units[m]->S[1]->Detection_Block = Units[m]->B[1];
  // }
  // else if(m == 3){//Station 4 bakken
  //   Create_Unit(m,8,8,2);
  //   /*
  //   Blocks address (Octal)
  //              /-- -10-11- --\
  //   --0-- -\1-'--- -12-13- ---`-30-/- -31-
  //   --2-- -3\-,--- -14-15- ---,-32-'- -33-
  //              \-- -16-17- --/
  //               \- -20-21- -/
  //   Block numbers (Decimal)
  //              /-- --2--3- --\
  //   --0-- -\1-'--- --4--5- ---`--6-/- --7-
  //   --8-- -9\-,--- -10-11- ---,-16-'- -17-
  //              \-- -12-13- --/
  //               \- -14-15- -/
  //   Switches numbers (Decimal)
  //       0 1                   5 6
  //        2 3                 7 8
  //           4               9
  //   Switches addresses (octal)
  //   0  0--1     5 20-21
  //   1  2--3     6 22-23
  //   2  4--5     7 24-25
  //   3  6--7     8 26-27
  //   4 10-11     9 30-31
  //   */
  //   link.Adr1.Module = m;link.Adr1.Adr =  7;link.Adr1.type = 'R';
  //   link.Adr2.Module = m;link.Adr2.Adr = 17;link.Adr2.type = 'R';
  //   //void Create_Segment(int Unit_Adr, struct adr Adr,struct adr NAdr,struct adr PAdr  ,char max_speed,char state,char dir,char len);
  //   //void Create_Segment(struct SegC Adr,char type, struct SegC Next, struct SegC Prev,char max_speed,char state,char dir,char len);

  //   join(IN.Adr1,CAdr(m,0,'R'));
  //   join(IN.Adr2,CAdr(m,8,'R'));

  //   Create_Switch(CAdr(m,0,2),CAdr(m,0,'R'),CAdr(m,2,'s') ,CAdr(m,1,'S') ,(int [2]){000,001},0); //Switch 0
  //   Create_Switch(CAdr(m,1,2),CAdr(m,0,'s'),CAdr(m,2,'R') ,CAdr(m,4,'R') ,(int [2]){002,003},1);      //Switch 1
  //   Create_Switch(CAdr(m,2,2),CAdr(m,3,'S'),CAdr(m,0,'s') ,CAdr(m,8,'R') ,(int [2]){004,005},0);  //Switch 2
  //   Create_Switch(CAdr(m,3,2),CAdr(m,2,'S'),CAdr(m,4,'S') ,CAdr(m,10,'R'),(int [2]){006,007},1); //Switch 3
  //   Create_Switch(CAdr(m,4,2),CAdr(m,3,'s'),CAdr(m,12,'R'),CAdr(m,14,'R'),(int [2]){010,011},1);    //Switch 4
  //   Create_Switch(CAdr(m,5,2),CAdr(m,6,'s'),CAdr(m,3,'R') ,CAdr(m,5,'R') ,(int [2]){020,021},1);        //Switch 010
  //   Create_Switch(CAdr(m,6,2),CAdr(m,7,'R'),CAdr(m,8,'s') ,CAdr(m,5,'S') ,(int [2]){022,023},0); //Switch 011
  //   Create_Switch(CAdr(m,7,2),CAdr(m,8,'S'),CAdr(m,9,'S') ,CAdr(m,11,'R'),(int [2]){024,025},1); //Switch 013
  //   Create_Switch(CAdr(m,8,2),CAdr(m,7,'S'),CAdr(m,6,'s') ,CAdr(m,17,'R'),(int [2]){026,027},0); //Switch 014
  //   Create_Switch(CAdr(m,9,2),CAdr(m,7,'s'),CAdr(m,13,'R'),CAdr(m,15,'R'),(int [2]){030,031},1);     //Switch 012

  //   Create_Segment(0,CAdr(m,0,'R'),IN.Adr1,CAdr(m,0,'S'),speed_B,0,0,100);
  //   Create_Segment(1,CAdr(m,1,'T'),EMPTY_BL(),EMPTY_BL(),speed_B,0,0,100);

  //   Create_Segment(010,CAdr(m,2,'R'),CAdr(m,1,'s'),CAdr(m,3,'R')  ,speed_C,0,0,50);
  //   Create_Segment(011,CAdr(m,3,'R'),CAdr(m,2,'R'),CAdr(m,5,'s'),speed_C,0,0,50);
  //   Create_Segment(012,CAdr(m,4,'R'),CAdr(m,1,'s'),CAdr(m,5,'R'),  speed_C,0,0,50);
  //   Create_Segment(013,CAdr(m,5,'R'),CAdr(m,4,'R'),CAdr(m,5,'s'),speed_C,0,0,50);

  //   Create_Segment(030,CAdr(m,6,'T'),EMPTY_BL(),EMPTY_BL(),speed_B,0,0,50);
  //   Create_Segment(031,CAdr(m,7,'R'),CAdr(m,6,'S'),EMPTY_BL(),speed_B,0,0,100);


  //   Create_Segment(2,CAdr(m,8,'R'),EMPTY_BL(),CAdr(m,2,'s'),speed_B,0,1,50);
  //   Create_Segment(3,CAdr(m,9,'T'),EMPTY_BL(),EMPTY_BL(),speed_B,0,1,50);

  //   Create_Segment(014,CAdr(m,10,'R'),CAdr(m,3,'s') ,CAdr(m,11,'R'),speed_C,0,0,50);
  //   Create_Segment(015,CAdr(m,11,'R'),CAdr(m,10,'R'),CAdr(m,7,'s') ,speed_C,0,0,50);
  //   Create_Segment(016,CAdr(m,12,'R'),CAdr(m,4,'s') ,CAdr(m,13,'R'),speed_C,0,0,50);
  //   Create_Segment(017,CAdr(m,13,'R'),CAdr(m,12,'R'),CAdr(m,9,'s') ,speed_C,0,0,50);
  //   Create_Segment(020,CAdr(m,14,'R'),CAdr(m,4,'s') ,CAdr(m,15,'R'),speed_C,0,0,50);
  //   Create_Segment(021,CAdr(m,15,'R'),CAdr(m,14,'R'),CAdr(m,9,'s') ,speed_C,0,0,50);

  //   Create_Segment(032,CAdr(m,16,'T'),EMPTY_BL(),EMPTY_BL(),speed_B,0,0,50);
  //   Create_Segment(033,CAdr(m,17,'R'),CAdr(m,8,'s'),EMPTY_BL(),speed_B,0,1,50);
  // }
  // else if(m == 4){//Station 4 bakken
  //   Create_Unit(m,8,32,2);
  //   /*
  //   Blocks addresses (Octal)
  //              /-- -10-11- -30-31- --\
  //   --0-- -\1-'--- -12-13- -32-33- ---`-50-/- -51-
  //   --2-- -3\-,--- -14-15- -34-35- ---,-52-'- -53-
  //              \-- -16-17- -36-37- --/
  //               \- -20-21- -40-41- -/
  //   Blocks addresses (Decimal)
  //               /-- --2--3- --4--5- --\
  //   --0-- --\1-'--- --6--7- --8--9- ---`-10-/- -11-
  //   -12-- -13\-,--- -14-15- -16-17- ---,-26'-- -27-
  //               \-- -18-19- -20-21- --/
  //                \- -22-23- -24-25- -/
  //   Switches numbers (Decimal)
  //       0 1                   5 6
  //        2 3                 7 8
  //           4               9
  //   Switches addresses (octal)
  //   0  0--1     5 20-21
  //   1  2--3     6 22-23
  //   2  4--5     7 24-25
  //   3  6--7     8 26-27
  //   4 10-11     9 30-31
  //   */
  //   link.Adr1.Module = m;link.Adr1.Adr = 11;link.Adr1.type = 'R';
  //   link.Adr2.Module = m;link.Adr2.Adr = 27;link.Adr2.type = 'R';
  //   //void Create_Segment(int Unit_Adr, struct adr Adr,struct adr NAdr,struct adr PAdr  ,char max_speed,char state,char dir,char len);
  //   //void Create_Segment(struct SegC Adr,char type, struct SegC Next, struct SegC Prev,char max_speed,char state,char dir,char len);

  //   join(IN.Adr1,CAdr(m,0,'R'));
  //   join(IN.Adr2,CAdr(m,12,'R'));

  //   Create_Switch(CAdr(m,0,2),CAdr(m,0,'R'),CAdr(m,2,'s') ,CAdr(m,1,'S') ,(int [2]){000,001},0); //Switch 0
  //   Create_Switch(CAdr(m,1,2),CAdr(m,0,'s'),CAdr(m,2,'R') ,CAdr(m,6,'R') ,(int [2]){002,003},1);      //Switch 1
  //   Create_Switch(CAdr(m,2,2),CAdr(m,3,'S'),CAdr(m,0,'s') ,CAdr(m,12,'R'),(int [2]){004,005},0);  //Switch 2
  //   Create_Switch(CAdr(m,3,2),CAdr(m,2,'S'),CAdr(m,4,'S') ,CAdr(m,14,'R'),(int [2]){006,007},1); //Switch 3
  //   Create_Switch(CAdr(m,4,2),CAdr(m,3,'s'),CAdr(m,18,'R'),CAdr(m,22,'R'),(int [2]){010,011},1);    //Switch 4
  //   Create_Switch(CAdr(m,5,2),CAdr(m,6,'s'),CAdr(m,5,'R') ,CAdr(m,9,'R') ,(int [2]){020,021},1);        //Switch 010
  //   Create_Switch(CAdr(m,6,2),CAdr(m,11,'R'),CAdr(m,8,'s'),CAdr(m,5,'S') ,(int [2]){022,023},0); //Switch 011
  //   Create_Switch(CAdr(m,7,2),CAdr(m,8,'S'),CAdr(m,9,'S') ,CAdr(m,17,'R'),(int [2]){024,025},1); //Switch 013
  //   Create_Switch(CAdr(m,8,2),CAdr(m,7,'S'),CAdr(m,6,'s') ,CAdr(m,27,'R'),(int [2]){026,027},0); //Switch 014
  //   Create_Switch(CAdr(m,9,2),CAdr(m,7,'s'),CAdr(m,21,'R'),CAdr(m,25,'R'),(int [2]){030,031},1);     //Switch 012

  //   Create_Segment(0,CAdr(m,0,'R'),IN.Adr1,CAdr(m,0,'S'),speed_B,0,0,100);
  //   Create_Segment(1,CAdr(m,1,'T'),EMPTY_BL(),EMPTY_BL(),speed_B,0,0,100);

  //   Create_Segment(010,CAdr(m,2,'S'),CAdr(m,1,'s'),CAdr(m,3,'R'),speed_C,0,0,50);
  //   Create_Segment(011,CAdr(m,3,'S'),CAdr(m,2,'R'),CAdr(m,4,'R'),speed_C,0,0,50);
  //   Create_Segment(030,CAdr(m,4,'S'),CAdr(m,3,'R'),CAdr(m,5,'R'),speed_C,0,0,50);
  //   Create_Segment(031,CAdr(m,5,'S'),CAdr(m,4,'R'),CAdr(m,5,'s'),speed_C,0,0,50);

  //   Create_Segment(012,CAdr(m,6,'S'),CAdr(m,1,'s'),CAdr(m,7,'R'),speed_C,0,0,50);
  //   Create_Segment(013,CAdr(m,7,'S'),CAdr(m,6,'R'),CAdr(m,8,'R'),speed_C,0,0,50);
  //   Create_Segment(032,CAdr(m,8,'S'),CAdr(m,7,'R'),CAdr(m,9,'R'),speed_C,0,0,50);
  //   Create_Segment(033,CAdr(m,9,'S'),CAdr(m,8,'R'),CAdr(m,5,'s'),speed_C,0,0,50);

  //   Create_Segment(050,CAdr(m,10,'T'),EMPTY_BL(),EMPTY_BL(),speed_B,0,0,50);
  //   Create_Segment(051,CAdr(m,11,'R'),CAdr(m,6,'S'),EMPTY_BL(),speed_B,0,0,100);


  //   Create_Segment(2,CAdr(m,12,'R'),IN.Adr2,CAdr(m,2,'s'),speed_B,0,1,50);
  //   Create_Segment(3,CAdr(m,13,'T'),EMPTY_BL(),EMPTY_BL(),speed_B,0,1,50);

  //   Create_Segment(014,CAdr(m,14,'S'),CAdr(m,3,'s') ,CAdr(m,15,'R'),speed_C,0,1,50);
  //   Create_Segment(015,CAdr(m,15,'S'),CAdr(m,14,'R'),CAdr(m,16,'R'),speed_C,0,1,50);
  //   Create_Segment(034,CAdr(m,16,'S'),CAdr(m,15,'R'),CAdr(m,17,'R'),speed_C,0,1,50);
  //   Create_Segment(035,CAdr(m,17,'S'),CAdr(m,16,'R'),CAdr(m,7 ,'s'),speed_C,0,1,50);

  //   Create_Segment(016,CAdr(m,18,'S'),CAdr(m,4,'s') ,CAdr(m,19,'R'),speed_C,0,1,50);
  //   Create_Segment(017,CAdr(m,19,'S'),CAdr(m,18,'R'),CAdr(m,20,'R'),speed_C,0,1,50);
  //   Create_Segment(036,CAdr(m,20,'S'),CAdr(m,19,'R'),CAdr(m,21,'R'),speed_C,0,1,50);
  //   Create_Segment(037,CAdr(m,21,'S'),CAdr(m,20,'R'),CAdr(m,9,'s') ,speed_C,0,1,50);

  //   Create_Segment(020,CAdr(m,22,'S'),CAdr(m,4,'s'),CAdr(m,23,'R'),speed_C,0,1,50);
  //   Create_Segment(021,CAdr(m,23,'S'),CAdr(m,22,'R'),CAdr(m,24,'R'),speed_C,0,1,50);
  //   Create_Segment(040,CAdr(m,24,'S'),CAdr(m,23,'R'),CAdr(m,25,'R'),speed_C,0,1,50);
  //   Create_Segment(041,CAdr(m,25,'S'),CAdr(m,24,'R'),CAdr(m,9,'s'),speed_C,0,1,50);

  //   Create_Segment(052,CAdr(m,26,'T'),EMPTY_BL(),EMPTY_BL(),speed_B,0,1,50);
  //   Create_Segment(053,CAdr(m,27,'R'),CAdr(m,8,'s'),EMPTY_BL(),speed_B,0,1,50);

  //   Units[m]->S[0]->Detection_Block = Units[m]->B[1];
  //   Units[m]->S[1]->Detection_Block = Units[m]->B[1];
  //   Units[m]->S[2]->Detection_Block = Units[m]->B[13];
  //   Units[m]->S[3]->Detection_Block = Units[m]->B[13];
  //   Units[m]->S[4]->Detection_Block = Units[m]->B[13];

  //   Units[m]->S[5]->Detection_Block = Units[m]->B[10];
  //   Units[m]->S[6]->Detection_Block = Units[m]->B[10];
  //   Units[m]->S[7]->Detection_Block = Units[m]->B[26];
  //   Units[m]->S[8]->Detection_Block = Units[m]->B[26];
  //   Units[m]->S[9]->Detection_Block = Units[m]->B[26];

  //   /*Linking Switches*/
  //    struct L_Swi_t * B_Swi = (struct L_Swi_t*)malloc(sizeof(struct L_Swi_t));
  //    struct SegC ADR = CAdr(m,0,'S');
  //    B_Swi->Adr = ADR;
  //    B_Swi->states[0] = 0;
  //    B_Swi->states[1] = 1;
  //    Units[m]->S[2]->L_Swi[0] = B_Swi;
  //    Units[m]->S[6]->L_Swi[0] = B_Swi;
  //    Units[m]->S[8]->L_Swi[0] = B_Swi;

  //    B_Swi = (struct L_Swi_t*)malloc(sizeof(struct L_Swi_t));

  //    ADR.Adr = 2;
  //    B_Swi->Adr = ADR;
  //    B_Swi->states[0] = 0;
  //    B_Swi->states[1] = 1;
  //    Units[m]->S[0]->L_Swi[0] = B_Swi;
  //    Units[m]->S[6]->L_Swi[1] = B_Swi;
  //    Units[m]->S[8]->L_Swi[1] = B_Swi;

  //    B_Swi = (struct L_Swi_t*)malloc(sizeof(struct L_Swi_t));

  //    ADR.Adr = 6;
  //    B_Swi->Adr = ADR;
  //    B_Swi->states[0] = 0;
  //    B_Swi->states[1] = 1;
  //    Units[m]->S[0]->L_Swi[1] = B_Swi;
  //    Units[m]->S[2]->L_Swi[1] = B_Swi;
  //    Units[m]->S[8]->L_Swi[2] = B_Swi;

  //    B_Swi = (struct L_Swi_t*)malloc(sizeof(struct L_Swi_t));

  //    ADR.Adr = 8;
  //    B_Swi->Adr = ADR;
  //    B_Swi->states[0] = 0;
  //    B_Swi->states[1] = 1;
  //    Units[m]->S[0]->L_Swi[2] = B_Swi;
  //    Units[m]->S[2]->L_Swi[2] = B_Swi;
  //    Units[m]->S[6]->L_Swi[2] = B_Swi;
  //   /**/
  //   //
  //   /*Setting Switch preferences*/
  //     struct P_Swi_t * P = (struct P_Swi_t *)malloc(sizeof(struct P_Swi_t));
  //     P->type = 0;    //Always
  //     P->state = 0;   //Straigth when approaching switch
  //     Units[m]->S[8]->pref[0] = P;
  //     P = (struct P_Swi_t *)malloc(sizeof(struct P_Swi_t));
  //     P->type = 0;    //Always
  //     P->state = 1;   //Diverging when approaching switch
  //     Units[m]->S[2]->pref[0] = P;
  //   //
  //   /*Stations*/
  //     Create_Station(m,"Spoor 1",1,4,(int [4]){2,3,4,5});
  //     Create_Station(m,"Spoor 2",1,4,(int [4]){6,7,8,9});
  //     Create_Station(m,"Spoor 3",1,4,(int [4]){14,15,16,17});
  //     Create_Station(m,"Spoor 4",1,4,(int [4]){18,19,20,21});
  //     Create_Station(m,"Spoor 5",1,4,(int [4]){22,23,24,25});
  //   //
  //   /*Signals*/
  //     //All type 2 signals
  //     //NSignals

  //     uint8_t adr[3] = {0,1,2};
  //     struct Seg ** block_id = Units[m]->B;
  //     char b[BLOCK_STATES] = {1,2,4}; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
  //     char c[BLOCK_STATES] = {1,0,0};

  //     create_signal2(block_id[2],3,adr,b,c,0);

  //     memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
  //     adr[0] = 3;adr[1] = 4;adr[2] = 5;
  //     b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
  //     c[0] = 1;c[1] = 0;c[2] = 0;

  //     create_signal2(block_id[6],3,adr,b,c,0);


  //     memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
  //     adr[0] = 16;adr[1] = 17;adr[2] = 18;
  //     b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
  //     c[0] = 1;c[1] = 0;c[2] = 0;

  //     create_signal2(block_id[17],3,adr,b,c,0);

  //     memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
  //     adr[0] = 19;adr[1] = 20;adr[2] = 21;
  //     b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
  //     c[0] = 1;c[1] = 0;c[2] = 0;

  //     create_signal2(block_id[21],3,adr,b,c,0);

  //     memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
  //     adr[0] = 22;adr[1] = 23;adr[2] = 24;
  //     b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
  //     c[0] = 1;c[1] = 0;c[2] = 0;

  //     create_signal2(block_id[25],3,adr,b,c,0);

  //     //PSignals

  //     memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
  //     adr[0] = 7;adr[1] = 8;adr[2] = 9;
  //     b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
  //     c[0] = 0;c[1] = 0;c[2] = 0;

  //     create_signal2(block_id[14],3,adr,b,c,1);

  //     memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
  //     adr[0] = 10;adr[1] = 11;adr[2] = 12;
  //     b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
  //     c[0] = 0;c[1] = 0;c[2] = 0;

  //     create_signal2(block_id[18],3,adr,b,c,1);

  //     memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
  //     adr[0] = 13;adr[1] = 14;adr[2] = 15;
  //     b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
  //     c[0] = 0;c[1] = 0;c[2] = 0;

  //     create_signal2(block_id[22],3,adr,b,c,1);
  //   //
  //   /*One Way*/
  //     Units[m]->B[0]->oneWay = TRUE;
  //     Units[m]->B[11]->oneWay = TRUE;
  //     Units[m]->B[12]->oneWay = TRUE;
  //     Units[m]->B[27]->oneWay = TRUE;
  //   //

  //   Units[m]->B[1]->dir  = 0;
  //   Units[m]->B[10]->dir = 0;
  //   Units[m]->B[13]->dir  = 1;
  //   Units[m]->B[26]->dir = 1;

  // }/*
  // else if(m == 5){//T piece
  //   Create_Unit___(m,8,8,3);
  //   join(IN.Adr1,C_Adr(m,1,1));
  //   Create_Segment(Seg_i++,C_Adr(m,1,1),IN.Adr1,END_BL,speed_A,0,0,100);
  //   C_Seg(Seg_i++,C_Adr(m,2,0),0);
  //   C_Seg(Seg_i++,C_Adr(m,5,0),0);
  //   join(IN.Adr2,C_AdrT(m,2,1,'S'));
  //   Create_Switch(Swi_i++,C_Adr(m,2,1),IN.Adr2,C_Adr(m,3,1),C_AdrT(m,5,1,'s'),1);
  //   Create_Switch(Swi_i++,C_Adr(m,5,1),END_BL,C_Adr(m,4,1),C_AdrT(m,2,1,'s'),1);
  //   Create_Segment(Seg_i++,C_Adr(m,3,1),C_AdrT(m,2,1,'s'),END_BL,speed_A,0,1,50);
  //   Create_Segment(Seg_i++,C_Adr(m,4,1),C_AdrT(m,5,1,'s'),END_BL,speed_A,0,2,50);
  //   blocks[m][2][0]->dir  = 1;
  //   blocks[m][5][0]->dir = 1;
  //   link.Adr1 = C_Adr(m,1,1);
  //   link.Adr2 = C_AdrT(m,5,1,'S');
  //   link.Adr3 = C_AdrT(m,4,1,'R');
  //   link.Adr4 = C_AdrT(m,3,1,'R');
  // }
  // else if(m == 6){//Rangeer Brug
  //   Create_Unit___(m);
  //   join(IN.Adr1,C_Adr(m,1,1));
  //   Create_Segment(Seg_i++,C_Adr(m,1,1),IN.Adr1,C_AdrT(m,3,1,'s'),speed_A,0,0,90);
  //   join(IN.Adr2,C_Adr(m,2,1));
  //   Create_Segment(Seg_i++,C_Adr(m,2,1),IN.Adr2,C_AdrT(m,3,1,'s'),speed_A,0,1,90);
  //   C_Seg(Seg_i++,C_Adr(m,3,0),0);
  //   Create_Switch(Swi_i++,C_Adr(m,3,1),END_BL,C_Adr(m,2,1),C_Adr(m,1,1),1);
  //   blocks[m][1][1]->oneWay = TRUE;
  //   blocks[m][2][1]->oneWay = TRUE;
  //   link.Adr1 = C_AdrT(m,3,1,'S');
  //   link.Adr2 = END_BL;
  // }
  // else if(m == 7){//Rangeer
  //   Create_Unit___(m,8,8,1);
  //   C_Seg(Seg_i++,C_Adr(m,1,0),0);
  //   Create_Switch(Swi_i++,C_Adr(m,1,1),C_AdrT(m,1,2,'s'),C_Adr(m,3,1),C_Adr(m,2,1),1);
  //   Create_Switch(Swi_i++,C_Adr(m,1,2),C_AdrT(m,1,3,'m'),C_Adr(m,4,1),C_AdrT(m,1,1,'S'),1);
  //   join(IN.Adr1,C_AdrT(m,1,3,'M'));
  //   struct adr A[10] = {{m,1,2,'S'},{m,5,1,'R'},{m,1,4,'S'},END_BL};
  //   struct adr B[10] = {link.Adr1,link.Adr1,link.Adr1,END_BL};
  //   Create_Moduls(Swi_i++,C_Adr(m,1,3),A,B,3);
  //   //Create_Switch(Swi_i++,C_Adr(m,1,3),link.Adr1,C_AdrT(m,1,2,'S'),C_AdrT(m,1,4,'S'),1);
  //   //Create_Switch(Swi_i++,C_Adr(m,1,4),C_AdrT(m,1,3,'s'),C_AdrT(m,1,5,'S'),C_Adr(m,5,1),1);
  //   Create_Switch(Swi_i++,C_Adr(m,1,4),C_AdrT(m,1,3,'m'),C_Adr(m,6,1),C_AdrT(m,1,5,'S'),1);
  //   Create_Switch(Swi_i++,C_Adr(m,1,5),C_AdrT(m,1,4,'s'),C_Adr(m,7,1),C_Adr(m,8,1),1);
  //   Create_Segment(Seg_i++,C_Adr(m,2,1),C_AdrT(m,1,1,'s'),C_Adr(m,2,2),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,2,2),C_Adr(m,2,1),C_Adr(m,2,3),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,2,3),C_Adr(m,2,2),C_Adr(m,2,4),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,2,4),C_Adr(m,2,3),C_Adr(m,2,5),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,2,5),C_Adr(m,2,4),C_Adr(m,2,6),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,2,6),C_Adr(m,2,5),END_BL,speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,3,1),C_AdrT(m,1,1,'s'),C_Adr(m,3,2),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,3,2),C_Adr(m,3,1),C_Adr(m,3,3),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,3,3),C_Adr(m,3,2),C_Adr(m,3,4),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,3,4),C_Adr(m,3,3),C_Adr(m,3,5),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,3,5),C_Adr(m,3,4),C_Adr(m,3,6),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,3,6),C_Adr(m,3,5),END_BL,speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,4,1),C_AdrT(m,1,2,'s'),C_Adr(m,4,2),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,4,2),C_Adr(m,4,1),C_Adr(m,4,3),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,4,3),C_Adr(m,4,2),C_Adr(m,4,4),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,4,4),C_Adr(m,4,3),C_Adr(m,4,5),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,4,5),C_Adr(m,4,4),C_Adr(m,4,6),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,4,6),C_Adr(m,4,5),END_BL,speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,5,1),C_AdrT(m,1,3,'m'),C_Adr(m,5,2),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,5,2),C_Adr(m,5,1),C_Adr(m,5,3),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,5,3),C_Adr(m,5,2),C_Adr(m,5,4),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,5,4),C_Adr(m,5,3),C_Adr(m,5,5),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,5,5),C_Adr(m,5,4),C_Adr(m,5,6),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,5,6),C_Adr(m,5,5),END_BL,speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,6,1),C_AdrT(m,1,4,'s'),C_Adr(m,6,2),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,6,2),C_Adr(m,6,1),C_Adr(m,6,3),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,6,3),C_Adr(m,6,2),C_Adr(m,6,4),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,6,4),C_Adr(m,6,3),C_Adr(m,6,5),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,6,5),C_Adr(m,6,4),C_Adr(m,6,6),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,6,6),C_Adr(m,6,5),END_BL,speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,7,1),C_AdrT(m,1,5,'s'),C_Adr(m,7,2),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,7,2),C_Adr(m,7,1),C_Adr(m,7,3),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,7,3),C_Adr(m,7,2),C_Adr(m,7,4),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,7,4),C_Adr(m,7,3),C_Adr(m,7,5),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,7,5),C_Adr(m,7,4),C_Adr(m,7,6),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,7,6),C_Adr(m,7,5),END_BL,speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,8,1),C_AdrT(m,1,5,'s'),C_Adr(m,8,2),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,8,2),C_Adr(m,8,1),C_Adr(m,8,3),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,8,3),C_Adr(m,8,2),C_Adr(m,8,4),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,8,4),C_Adr(m,8,3),C_Adr(m,8,5),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,8,5),C_Adr(m,8,4),C_Adr(m,8,6),speed_A,0,0,50);
  //   Create_Segment(Seg_i++,C_Adr(m,8,6),C_Adr(m,8,5),END_BL,speed_A,0,0,50);
  //   link.Adr1 = END_BL;
  //   link.Adr2 = END_BL;
  // }*/
  // else if(m == 8 || m == 9 || m == 10 || m == 12 || m == 13 || m == 14){//Stad
  //   Create_Unit(m,8,8,2);
  //   /*
  //   Block addresses (Octal)
  //   --00--02--010--012--
  //   --01--03--011--013--
  //   Block numbers (Decimal)
  //   --0--1--2--3--
  //   --4--5--6--7--
  //   */
  //   link.Adr1.Module = m;link.Adr1.Adr = 3;link.Adr1.type = 'R';
  //   link.Adr2.Module = m;link.Adr2.Adr = 7;link.Adr2.type = 'R';
  //   //void Create_Segment(int Unit_Adr, struct adr Adr,struct adr NAdr,struct adr PAdr  ,char max_speed,char state,char dir,char len);
  //   //void Create_Segment(struct SegC Adr,char type, struct SegC Next, struct SegC Prev,char max_speed,char state,char dir,char len);

  //   join(IN.Adr1,CAdr(m,0,'R'));
  //   join(IN.Adr2,CAdr(m,4,'R'));

  //   Create_Segment(0  ,CAdr(m,0,'R'),IN.Adr1      ,CAdr(m,1,'R'),speed_A,0,0,100);
  //   Create_Segment(02 ,CAdr(m,1,'R'),CAdr(m,0,'R'),CAdr(m,2,'R'),speed_A,0,0,100);
  //   Create_Segment(010,CAdr(m,2,'R'),CAdr(m,1,'R'),CAdr(m,3,'R'),speed_A,0,0,100);
  //   Create_Segment(012,CAdr(m,3,'R'),CAdr(m,2,'R'),EMPTY_BL()   ,speed_A,0,0,100);

  //   Create_Segment(1  ,CAdr(m,4,'R'),IN.Adr2      ,CAdr(m,5,'R'),speed_A,0,1,100);
  //   Create_Segment(03 ,CAdr(m,5,'R'),CAdr(m,4,'R'),CAdr(m,6,'R'),speed_A,0,1,100);
  //   Create_Segment(011,CAdr(m,6,'R'),CAdr(m,5,'R'),CAdr(m,7,'R'),speed_A,0,1,100);
  //   Create_Segment(013,CAdr(m,7,'R'),CAdr(m,6,'R'),EMPTY_BL()   ,speed_A,0,1,100);
  // }
  // else if(m == 11){//Korte Bocht
  //   Create_Unit(m,8,8,2);

  //   link.Adr1.Module = m;link.Adr1.Adr = 0;link.Adr1.type = 'R';
  //   link.Adr2.Module = m;link.Adr2.Adr = 1;link.Adr2.type = 'R';

  //   join(IN.Adr1,CAdr(m,0,'R'));
  //   join(IN.Adr2,CAdr(m,1,'R'));

  //   Create_Segment(0,CAdr(m,0,'R'),IN.Adr1,EMPTY_BL(),speed_A,0,0,100);
  //   Create_Segment(1,CAdr(m,1,'R'),IN.Adr2,EMPTY_BL(),speed_A,0,1,100);
  // }
  // return link;
}

// Binary structured files
// #define MAXBUFLEN 1000000
// struct test_struct {
//  char a;
//  char b;
//  char c;
//  char d;
//  char e;
//  char f;
// };

// char source[MAXBUFLEN + 1];
// memset(source,0,MAXBUFLEN+1);

// FILE *fp = fopen("test_bin.bin", "r");
// if (fp != NULL) {
//     size_t newLen = fread(source, sizeof(char), MAXBUFLEN, fp);
//     if ( ferror( fp ) != 0 ) {
//         fputs("Error reading file", stderr);
//     }

//     fclose(fp);
// }

//  struct test_struct * test = &source[0];

void free_modules(){
  logger("FREE_MODULES IMPLEMENT",CRITICAL);
  clear_Modules();
}

void LoadModuleFromConfig(int M){

}

void LoadModules(int M){
  loggerf(ERROR, "Going to be depricated: use LoadModuleFromConfig");

  if(M == 0){
    return;
  }

  // printf("Load module %i\n",M);

  if(M != 4 && M != 8 && M != 1 && M != 2 && M != 5 && M != 6 && M != 10 && M != 11 && M != 20 && M != 21 && M != 22 && M != 23){
    loggerf(WARNING, "Module not ready\n");
    return; //Function is not ready
  }

  //Try to open file
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  char folder[]   = "./configs/units/OLD_TODO_REMOVE/";
  char filename[] = "/prop.txt";
  char file[50] = "";

  sprintf(file, "%s%d%s", folder, M, filename);

  fp = fopen(file, "r");
  if (fp == NULL){
    loggerf(CRITICAL, "Failed to open File: %s\n",file);
    return;
  }

  //return; //STOP

  int ModuleID;

  while ((read = getline(&line, &len, fp)) != -1) {
    if(line[0] == '\'')
      continue;

          //printf("\nRetrieved line of length %02zu : ", read);

    char * p = strtok(line,"\t\r\n");
    char * parts[20];
    char i = 0;

    while(p != NULL){
      // printf("%s  ",p);
      parts[i++] = p;
      p = strtok(NULL, "\t\r\n");
    }
    // printf("\n");

    if(parts[0][0] == 'C'){
      if(strcmp(parts[0],"CU") == 0){ //Create Unit
        //Set Module ID for this file and Create Module
        ModuleID = atoi(parts[1]);
        if(ModuleID != M){
          printf("MODULE ID in file is not consistent with FolderNr\n");
          return;
        }
        // printf("Module ID: %i\n",ModuleID);
        Create_Unit(ModuleID,atoi(parts[2]),atoi(parts[3]),atoi(parts[4]));

      }else if(strcmp(parts[0],"CB") == 0){ //Create Block
        //Create a Segment with all given data from the file

        struct rail_link Adr,NAdr,PAdr;
        Adr.module = ModuleID;
        Adr.id = atoi(parts[2]);
        Adr.type = parts[3][0];
        // printf("New block in module %i",ModuleID);
        //Next Block
        if(parts[4][0] == 'E'){ //End of line / Empty
          NAdr.module = 0;
          NAdr.id = 0;
          NAdr.type = 'e';
        }
        else{
          if(parts[4][0] == 'C'){
            NAdr = CAdr(atoi(parts[5]),atoi(parts[6]),'C');
          }else if(parts[4][0] == 'X'){
            NAdr = CAdr(ModuleID,atoi(parts[5]),parts[6][0]);
          }else {
            NAdr = CAdr(atoi(parts[4]),atoi(parts[5]),parts[6][0]);
          }
        }

        //Prev Block
        if(parts[7][0] == 'E'){
          PAdr = EMPTY_BL();
        }
        else{
          if(parts[7][0] == 'C'){
            PAdr = CAdr(atoi(parts[8]),atoi(parts[9]),'C');
          }else if(parts[7][0] == 'X'){
            // printf("Prev block is in same module %i \n",ModuleID);
            PAdr = CAdr(ModuleID,atoi(parts[8]),parts[9][0]);
          }else {
            PAdr = CAdr(atoi(parts[7]),atoi(parts[8]),parts[9][0]);
          }
        }
        //Create_Segment(IO_Adr        ,Adr,Next,Prev,mspd,           state,dir,            len);
        struct block_connect tmp;
        tmp.module = Adr.module;
        tmp.id = Adr.id;
        tmp.type = MAIN;
        tmp.next = NAdr;
        tmp.prev = PAdr;
        Create_Segment(strtol(parts[1],NULL,8),tmp,atoi(parts[10]),atoi(parts[11]),atoi(parts[12]));
        //Set oneway
        if(parts[13][0] == 'Y'){
          Units[ModuleID]->B[Adr.id]->oneWay = TRUE;
        }
      }else if(strcmp(parts[0],"CSw") == 0){//Create Switch
        //Create a Switch with all given data from the file

        struct rail_link Adr,AAdr,SAdr,DAdr;
        Adr = CAdr(ModuleID,atoi(parts[1]),atoi(parts[12]));

        //Approach Block
        if(parts[3][0] == 'E'){
          AAdr = EMPTY_BL();
        }
        else{
          if(parts[3][0] == 'X'){
            AAdr = CAdr(ModuleID,atoi(parts[4]),parts[5][0]);
          }else if(parts[3][0] == 'C'){
            AAdr = CAdr(atoi(parts[4]),atoi(parts[5]),'C');
          }else{
            AAdr = CAdr(atoi(parts[3]),atoi(parts[4]),parts[5][0]);
          }
        }

        //Diverging Block 9+
        if(parts[9][0] == 'E'){
          DAdr = EMPTY_BL();
        }
        else{
          if(parts[9][0] == 'X'){
            DAdr = CAdr(ModuleID,atoi(parts[10]),parts[11][0]);
          }else if(parts[9][0] == 'C'){
            DAdr = CAdr(atoi(parts[10]),atoi(parts[11]),'C');
          }else{
            DAdr = CAdr(atoi(parts[9]),atoi(parts[10]),parts[11][0]);
          }
        }

        //Straigth Block 6
        if(parts[6][0] == 'E'){
          SAdr = EMPTY_BL();
        }
        else{
          if(parts[6][0] == 'X'){
            SAdr = CAdr(ModuleID,atoi(parts[7]),parts[8][0]);
          }else if(parts[6][0] == 'C'){
            SAdr = CAdr(atoi(parts[7]),atoi(parts[8]),'C');
          }else{
            SAdr = CAdr(atoi(parts[6]),atoi(parts[7]),parts[8][0]);
          }
        }

        int IOAddress[20];
        char * q;
        i = 0;
        q = strtok(parts[12], " ");

        while(q != NULL){
          IOAddress[i++] = atoi(q);
          q = strtok(NULL, " ");
        }

        int StateSpeed[20];
        i = 0;
        q = strtok(parts[13], " ");

        while(q != NULL){
          StateSpeed[i++] = atoi(q);
          q = strtok(NULL, " ");
        }

        struct switch_connect tmp;
        tmp.module = ModuleID;
        tmp.id = Adr.id;
        tmp.app = AAdr;
        tmp.div = DAdr;
        tmp.str = SAdr;

        char * A = _calloc(2, char);
        A[0] = 03;
        A[1] = 04;

        _Bool ** B = _calloc(2, _Bool *);
        B[0] = _calloc(2, _Bool);
        B[1] = _calloc(2, _Bool);
        B[0][0] = 1; //State 0  - Output 0
        B[0][1] = 0; //State 0  - Output 1
        B[1][0] = 0; //State 1  - Output 0
        B[1][1] = 1; //State 1  - Output 1

        Create_Switch(tmp,atoi(parts[2]), 2, A, B);

        //Units[ModuleID]->S[Adr.id]->Detection_Block = atoi(parts[2]);
      }else if(strcmp(parts[0],"CSi") == 0){//Create Signal
        printf("Create Signals - Not Supported");
      }else if(strcmp(parts[0],"CSt") == 0){//Create Station
        // printf("Create Station/Stop");

        char name[30];
        strcpy(name,parts[1]);

        char type = atoi(parts[2]);
        char NrBlocks = atoi(parts[3]);

        int Blocks[10];
        char * q;
        i = 0;
        q = strtok(parts[4], " ");

        while(q != NULL){
          Blocks[i++] = atoi(q);
          q = strtok(NULL, " ");
        }

        Block ** blocks = _calloc(i, Block *);

        for(char j = 0; j < i; j++){
          blocks[j] = Units[ModuleID]->B[Blocks[j]];
        }

        Create_Station(ModuleID, 0, name, strlen(name)+2, PERSON, 100, blocks);
      }
    }
    else if(parts[0][0] == 'S'){
      if(strcmp(parts[0],"Sdet") == 0){ //Switch detection block
        Units[ModuleID]->Sw[atoi(parts[1])]->Detection = Units[ModuleID]->B[atoi(parts[2])];
      }
    }
  }

  fclose(fp);
  if (line)
    free(line);
}

void JoinModules(){
  if((_SYS->_STATE & STATE_Modules_Loaded) == 0){
    //No track loaded
    return;
  }
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
  while((_SYS->_STATE & STATE_Modules_Coupled) == 0){
    cur_j = connect_Algor(&List);
    if(i > 30){
      printf(" (%02i/%02i)\n",cur_j,max_j);
      i = 0;
      x++;
    }
    if(prev_j == cur_j){
      printf(".");
    }else{
      printf("+");

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
    usleep(10000);
    prev_j = cur_j;

    if(i == 15){
    if(x == 1){
      Units[20]->B[5]->blocked = 1;
      Units[22]->B[0]->blocked = 1;
      printf("\n1\n");
    }else if(x == 2){
      Units[21]->B[3]->blocked = 1;
      Units[23]->B[0]->blocked = 1;

      Units[20]->B[5]->blocked = 0;
      Units[22]->B[0]->blocked = 0;
      printf("\n2\n");
    }else if(x == 3){
      Units[20]->B[0]->blocked = 1;
      Units[23]->B[1]->blocked = 1;

      Units[21]->B[3]->blocked = 0;
      Units[23]->B[0]->blocked = 0;
      printf("\n3\n");
    }else if(x == 4){
      Units[21]->B[0]->blocked = 1;
      Units[22]->B[1]->blocked = 1;

      Units[20]->B[0]->blocked = 0;
      Units[23]->B[1]->blocked = 0;
      printf("\n4\n");
    }else if(x == 5){
      Units[21]->B[0]->blocked = 0;
      Units[22]->B[1]->blocked = 0;
      printf("\n41\n");
    }else if(x == 6){
      printf("\nend\n");
    }
    else if(x == 10){
      // _SYS_change(STATE_Modules_Coupled,1);
    }
    }
    //IF ALL JOINED
    //BREAK
  }

  for(int i = 0;i<List.length;i++){
    if(List.R_L[i]->type == 'S'){
      if(((Switch *)List.R_L[i]->p)->app.type == 'C'){
        ((Switch *)List.R_L[i]->p)->app.type = 0;
      }else if(((Switch *)List.R_L[i]->p)->str.type == 'C'){
        ((Switch *)List.R_L[i]->p)->str.type = 0;
      }else if(((Switch *)List.R_L[i]->p)->div.type == 'C'){
        ((Switch *)List.R_L[i]->p)->div.type = 0;
      }
    }else if(((Block *)List.R_L[i]->p)){
      if(((Block *)List.R_L[i]->p)->next.type == 'C'){
        ((Block *)List.R_L[i]->p)->next.type = 0;
      }else if(((Block *)List.R_L[i]->p)->prev.type == 'C'){
        ((Block *)List.R_L[i]->p)->prev.type = 0;
      }
    }

    free(List.R_L[i]);
  }

  WS_Track_Layout();

}

void ConnectModulePoints(Block * A,Block * B){
  char anchor_A,rail_A,anchor_B,rail_B;
  if(A->next.type == 'C'){
    anchor_A = A->next.module;
    rail_A   = A->next.id;
  }else{
    anchor_A = A->next.module;
    rail_A   = A->next.id;
  }


}
