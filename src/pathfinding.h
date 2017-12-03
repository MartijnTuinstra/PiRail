struct Sw_A_PATH {
	struct adr adr;				//Switch address
	char length;
	signed char suc[10];					//Successfull with state (-1 == Fail, 0 == to be tested, 1 == testing, 2 == Successfull)
	int state[10];				//State
	int dir;							//Direction
	struct Sw_A_PATH * Prev;  		//Previous/parent switches
	struct Sw_A_PATH * Next[20];	//Next switches
};

struct Sw_PATH {
	struct adr adr;				//Switch address
	signed char suc[10];					//Successfull with state (-1 == Fail, 0 == to be tested, 1 == testing, 2 == Successfull)
};

int pathFinding(struct adr Begin, struct adr End, struct Sw_PATH * OUT_Sw_Nodes[MAX_ROUTE]);
