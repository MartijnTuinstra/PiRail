#define H_Modules

#ifndef t_Unit_IO
  #define t_Unit_IO
  struct Unit_IO{
  	char type;
  	struct Seg * B;
  	struct Swi * S;
  	struct Mod * M;
  	struct signal * Signals;
  };
#endif

#ifndef t_Unit
  #define t_Unit
  struct Unit{
  	int B_L;  //Length of Block list;
  	int S_L;  //Length of Switch list;
  	int Si_L; //Length of Signals list;

  	_Bool Sig_change;

  	char Out_length;
  	char In_length;

    struct Rail_link ** Out;
    struct Rail_link ** In;

    uint8_t *BlinkMask;
    uint8_t *OutRegs;
    uint8_t  *InRegs;

    uint8_t  InRegisters;
    uint8_t OutRegisters;

  	struct Seg ** B;
  	struct Swi ** S;
  	struct Mod * M[MAX_Blocks*MAX_Segments];
  	struct signal * Signals[MAX_Blocks*MAX_Segments];
    struct Station * St[20];

  	char B_nr, Swi_nr, Mod_nr, Signal_nr,Station_nr; //Number of elements in array
  };

  struct Unit *Units[MAX_Modules];

  void clear_Modules();
#endif
