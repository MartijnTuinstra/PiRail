#define MAX_SW 8
#define MAX_SWITCH_LINK 5
#define MAX_SWITCH_PREFFERENCE 5

struct L_Swi_t{
	struct SegC Adr;
	int states[5];
};

struct P_Swi_t{
	char type;
	char state;
};

struct Swi{
	int Module;
	int id;

	struct Rail_link div;
	struct Rail_link str;
	struct Rail_link app;

	struct SegC DivC;
	struct SegC StrC;
	struct SegC AppC;

	char state;	//0 = Straight, 1 = Diverging / 0x80 is change bit
	char default_state;
	char len;

	char UAdr; //Unit Address
	char Out[5]; //Output Addresses

	struct Seg * Detection_Block;

	struct L_Swi_t * L_Swi[MAX_SWITCH_LINK]; //Linked switches

	struct P_Swi_t * pref[MAX_SWITCH_PREFFERENCE];//Switch preference
};

struct Mod{
	int Module;
	int id;
	struct adr Adr;

	struct Seg * Detection_Block;

	struct adr mAdr[10];
	struct adr MAdr[10];

	struct Rail_link m_Adr[10];
	struct Rail_link M_Adr[10];

	struct SegC m_AdrC[10];
	struct SegC M_AdrC[10];

	char length;
	char s_length;
	char state;
};

struct train;

int throw_switch(struct Swi * S);

int set_switch(struct Swi * S,char state);

int throw_ms_switch(struct Mod * M, char c);

void Create_Switch(struct SegC Adr,struct SegC App,struct SegC Div,struct SegC Str,int * adr,char state);

void Create_Moduls(int Unit_Adr, struct adr Adr,struct adr mAdr[10],struct adr MAdr[10],char length);

int check_Switch_state(struct Rail_link NAdr);

int check_Switch(struct Seg * B, int direct, _Bool incl_pref);

int free_Switch(struct Seg *B, int direct);

int free_Route_Switch(struct Seg *B, int direct, struct train * T);