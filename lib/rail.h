
#ifndef _INCLUDE_RAIL_H
  #define _INCLUDE_RAIL_H

  #define MAX_Segments 8
  #define MAX_Blocks 16
  #define MAX_Modules 26

  //Block Speed (Cat. A)
  #define speed_A 250
  //Block Speed (Cat. B)
  #define speed_B 140
  //Block Speed (Cat. C)
  #define speed_C 90
  //Block Speed (Cat. D)
  #define speed_D 60

  extern struct Station * stations[MAX_Modules*MAX_Blocks];
  extern int St_list_i;

  struct Swi;

  struct signal;

  struct Rail_link {
    char type;
    void * ptr;
      // struct Seg * B;
      // struct Swi * Sw;
      // struct Mod * M;
      // struct signal * Si;
  };

  struct Station_link{
    struct Station * St;
    char type;
  };

  struct SegC {
    long Adr;
    char Module;
    char type;
  };

  typedef struct Seg{
    int Module;
    int id;
    char type:4;
    char dir:4;

    struct Rail_link Next;
    struct Rail_link Prev;

    struct SegC NextC;
    struct SegC PrevC;

    struct Station * Station;

  	char max_speed;		// 5 times the speed (25 => 125km/h )
  	char state;
  	char length;
  	char train;		 //0x00 <-> 0xFF
  	_Bool blocked;
  	_Bool change; //If block changed state;
  	_Bool oneWay;

  	//Rules
  	//Speed Rules
  	//Train Type preference

  	//Signals
  	struct signal * NSi; // Signal in Next block direction
  	struct signal * PSi; // Signal in Prev block direction
  	//char signals; // AAAA BBBB, A = Prev Signal, B = Next Signal
  } block;

  struct Switch_state_link{
    char type;
    char state;
    struct Swi * Sw;
    struct Mod * MSw;
  };

  struct Station{
    int Module; //Module
    int id;   //Module ID
    int UnID; //Universal id
    char Name[20];
  	struct Seg * Blocks[8];
  	char type;//type of stop (0 == All, 1 == Passenger-only, 2 == Cargo-only)

    char Switches;
    struct Switch_state_link ** Sw;
  };

  struct link{
  	struct SegC Adr1;
  	struct SegC Adr2;
  	struct SegC Adr3;
  	struct SegC Adr4;
  };

  char DeviceList[20];

  struct SegC EMPTY_BL();

  int dir_Comp(struct Seg *A,struct Seg *B);

  int Adr_Comp2(struct SegC A,struct SegC B);

  struct SegC CAdr(int M,int B,char type);

  void Create_Segment(int IO_Adr,struct SegC Adr ,struct SegC Next, struct SegC Prev,char max_speed,char state,char dir,char len);

  void Create_Segment2(int IO_Adr,char M,int ID,char Type,struct SegC Next, struct SegC Prev,char max_speed,char state,char dir,char len);

  int Block_cmp(struct Seg * A,struct Seg * B);

  int Link_cmp(struct Rail_link A, struct Rail_link B);

  void Connect_Segments();

  int Create_Station(char Module,char * Name,char type,char nr,int Blocks[]);

  void Station_Add_Switch(struct Station * St,struct Swi * Sw,char state);

  void Station_Add_MSwitch(struct Station * St,struct Mod * MSw, char state);


  struct Seg * Next2(struct Seg * B,int i);

  struct Seg * Prev2(struct Seg * B,int i);

  struct Rail_link NADR2(struct Seg * B);

  struct Rail_link PADR2(struct Seg * B);
#endif