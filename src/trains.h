#define H_train
struct train{
	int DCC_ID;						//DCC address of the train
	int ID;							//Train ID
	char type;						//Type of train (C = Cargo, P = Passenger, H = High speed)
	char name[21];  				//Name of train
	char cur_speed; 				//Current speed of train
	long max_speed; 				//Maximum speed of train
	char accelerate_speed;	//divide by 50
	char break_speed; 			//divide by 50
	char use;
	char control;					//Is the computer in control? (0 = Manual, 1 = Semi-Automatic, 2 = Automatic)
	_Bool halt;							//If train is stopped at a station
	_Bool dir;						//0 = Forward, 1 = Reverse

	struct Sw_train_PATH * Route[MAX_ROUTE];
	int Sw_len;
	struct Seg * Destination;

	struct Seg * Cur_Block;

	char timer;
	int timer_id;
};

struct train_timer_th_data{
   int  thread_id;
	 int  Flag;
	 int speed;
	 struct train * T;
	 struct Seg * B;
};

struct train *trains[MAX_TRAINS] = {};
struct train *DCC_train[9999] = {};
struct train *train_link[MAX_TRAINS];
int iTrain = 0; //Counter for trains in library
int bTrain = 0; //Counter for trains on layout

int add_train(int DCC,int speed,char name[],char type);

int create_train(int DCC,int speed,char name[],char type);

void init_trains();

int link_train(char link,int train);

void unlink_train(char link);

void *train_timer(void *threadArg);

void train_speed(struct Seg * B,struct train * T,char speed);

void train_set_speed(struct train *T,char speed);

void train_set_dir(struct train *T,char dir);

void train_set_route(struct train *T,struct Station * S);

void train_stop(struct train * T);

void train_signal(struct Seg * B,struct train * T,int type);

void train_block_timer();
