#ifndef t_Swi
  struct Swi;
#endif

#ifndef t_signal
  struct signal;
#endif

#ifndef t_Rail_link
  #define t_Rail_link
  struct Rail_link {
    char type;

    struct Seg * B;
    struct Swi * Sw;
    struct Mod * M;
    struct signal * Si;
  };

  struct Station_link{
    struct Station * St;
    char type;
  };
#endif

struct SegC {
  long Adr;
  char Module;
  char type;
};

#ifndef t_Seg
  #define t_Seg
  struct Seg{
    int Module;
    int id;
    char type;

    struct Rail_link Next;
    struct Rail_link Prev;

    struct SegC NextC;
    struct SegC PrevC;

    struct Station * Station;

  	char max_speed;		// 5 times the speed (25 => 125km/h )
  	char state;
  	char dir;		//0xAABB, A = Current travel direction, B = Travel direction
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
  };
#endif

#ifndef t_Station
  #define t_Station
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
#endif

struct link{
	struct SegC Adr1;
	struct SegC Adr2;
	struct SegC Adr3;
	struct SegC Adr4;
};

void Create_Segment(int IO_Adr,struct SegC Adr ,struct SegC Next, struct SegC Prev,char max_speed,char state,char dir,char len);

`char DeviceList[20];