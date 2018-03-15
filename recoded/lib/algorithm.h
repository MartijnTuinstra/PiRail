struct procces_block {
	_Bool blocked;
	char length;
	struct Seg * B[5];
};

void change_block_state2(struct procces_block * A,int State);

void scan_All();

void * scan_All_continiously();

void procces(struct Seg * B,int debug);

void procces_accessoire();

struct ConnectList {
  int length;
  struct Seg * Block;
  struct Swi * Switch;
};

int init_connect_Algor(struct ConnectList * List);

int connect_Algor(struct ConnectList * List);