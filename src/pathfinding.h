#define H_path

struct Sw_A_PATH {
	struct Rail_link adr;				 //Switch address
	char length;                 //Nr. of states
	signed char suc[10];					//Successfull with state (-1 == Fail, 0 == to be tested, 1 == testing, 2 == Successfull)
	int state[10];			        	//State
	int dir;				              //Direction
	struct Sw_A_PATH * Prev;  		//Previous/parent switches
	struct Sw_A_PATH * Next[20];	//Next switches
};

struct Sw_PATH {
	struct Rail_link adr;				//Switch address
	signed char suc[10];				//Successfull with state (-1 == Fail, 0 == to be tested, 1 == testing, 2 == Successfull)
};

struct Sw_train_PATH{
	struct Rail_link adr;				//Switch address
	char states;								//Number of Successfull states
	signed char suc[10];				//The number of the states that are Successfull
};

int pathFinding(struct Seg * Begin, struct Seg * End, struct Sw_train_PATH *(OUT_Sw_Nodes)[MAX_ROUTE], int * len);
