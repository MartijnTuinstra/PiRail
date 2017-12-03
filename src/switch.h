struct L_Swi_t{
	struct adr Adr;
	int states[5];
};

struct P_Swi_t{
	char type;
	int state;
};

struct Swi{
	struct adr Adr;

	struct adr Div;
	struct adr Str;
	struct adr App;
	char state;
	char len;
	char UAdr;

	struct L_Swi_t * L_Swi[MAX_SWITCH_LINK]; //Linked switches

	struct P_Swi_t * pref[MAX_SWITCH_PREFFERENCE];//Switch preference
};

struct Mod{
	struct adr Adr;

	struct adr mAdr[10];
	struct adr MAdr[10];
	char length;
	char s_length;
	char state;
};

struct Swi *Switch[MAX_Modules][MAX_Blocks][MAX_SW] = {};
struct Mod *Moduls[MAX_Modules][MAX_Blocks][MAX_SW/4] = {};

int throw_switch(struct Swi * S);

int throw_ms_switch(struct Mod * M, char c);

void Create_Switch(int Unit_Adr, struct adr Adr,struct adr  App,struct adr Div,struct adr Str,char state);

void Create_Moduls(int Unit_Adr, struct adr Adr,struct adr mAdr[10],struct adr MAdr[10],char length);
/*
void Switch_Link(struct Seg * S,int M,int B,int S,){

}*/
