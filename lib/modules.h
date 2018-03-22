struct Unit_IO{
	char type;
	struct Seg * B;
	struct Swi * S;
	struct Mod * M;
	struct signal * Signals;
};

struct Unit{
	int B_L;  //Length of Block list;
	int S_L;  //Length of Switch list;
	int Si_L; //Length of Signals list;

	char Module;

	_Bool Sig_change;

	char Out_length;
	char In_length;

	struct Unit ** Connect;
	char connect_points;

  struct Rail_link ** Out;
  struct Rail_link ** In;

  uint8_t *BlinkMask;
  uint8_t *OutRegs;
  uint8_t  *InRegs;

  uint8_t  InRegisters;
  uint8_t OutRegisters;

	struct Seg ** B;
	struct Swi ** S;
	struct Mod * M[8];
	struct signal * Signals[32];
  struct Station * St[20];

	char Signal_nr,Station_nr; //Number of elements in array
};

#define END_BL C_AdrT(0,0,0,'e')

extern struct Unit *Units[MAX_Modules];

void clear_Modules();

void LoadModules(int M);

void JoinModules();

void setup_JSON(int arr[], int arr2[], int size, int size2);