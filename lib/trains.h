

#ifndef _INCLUDE_TRAINS_H
	#define _INCLUDE_TRAINS_H
	#define MAX_TRAINS 30
	#define MAX_ROUTE 200
	#define MAX_SWITCH_PREFFERENCE 5

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

	extern struct train *trains[MAX_TRAINS];
	extern struct train *DCC_train[9999];
	extern struct train *train_link[MAX_TRAINS];
	extern int iTrain; //Counter for trains in library
	extern int bTrain; //Counter for trains on layout

	struct Station;

	int add_train(int DCC,int speed,char name[],char type);

	int create_train(int DCC,int speed,char name[],char type);

	void init_trains();

	int link_train(char link,int train);

	void unlink_train(char link);

	void *train_timer(void *threadArg);

	void *clear_train_timers();

	void train_speed(struct Seg * B,struct train * T,char speed);

	void train_set_speed(struct train *T,char speed);

	void train_set_dir(struct train *T,char dir);

	void train_set_route(struct train *T,struct Station * S);

	void train_stop(struct train * T);

	void train_signal(struct Seg * B,struct train * T,int type);

	void train_block_timer();
#endif