#include "./rail.h"

#ifndef _INCLUDE_TRAINS_H
	#define _INCLUDE_TRAINS_H

	#include <signal.h>

	#define TRAIN_COMPS_CONF "./trains/train_comp.conf"
	#define CARS_CONF "./trains/cars.conf"
	#define ENGINES_CONF "./trains/engines.conf"
	#define CONF_VERSION 1

	struct train_comp {
		char type;
		void * p;
		uint16_t id;
	};

	struct __attribute__((__packed__)) train_comp_ws {
		char type;
		uint16_t ID;
	};

	struct train_composition {
		char * name;
		char nr_stock;
		struct train_comp * composition; // One block memory
	};

	struct __attribute__((__packed__)) train_comp_conf {
		char name_len;
		char nr_stock;
		char check;
	};

	typedef struct engine {
		uint16_t DCC_ID;
		char type:3;
		char control:2;
		char dir:1;
		char halt:2;
		uint16_t cur_spd;
		uint16_t max_spd;
		uint16_t length;		//in mm		
		char * name;
		char * img_path;
		char * icon_path;
	} Engines;

	struct __attribute__((__packed__)) engine_conf {
		uint16_t DCC_ID;
		uint16_t max_spd;
		uint16_t length;
		uint8_t type;		//in mm		
		uint8_t name_len;
		uint8_t img_path_len;
		uint8_t icon_path_len;
		uint8_t check;
	};

	typedef struct car {
		int nr;
		char type:3;
		char control:2;
		char dir:1;
		char halt:1;
		int max_spd;
		int length;		//in mm
		char * name;
		char * img_path;
		char * icon_path;
	} Cars;

	struct __attribute__((__packed__)) car_conf {
		uint16_t nr;
		uint16_t length;
		uint8_t type;		//in mm		
		uint8_t name_len;
		uint8_t img_path_len;
		uint8_t icon_path_len;
		uint8_t check;
	};

	typedef struct trains {
		char * name;

		char nr_engines;
		Engines ** engines;

		char nr_stock;
		struct train_comp * composition; //One block memory for all nr_stocks

		int length; //in mm

		int max_spd;
		int cur_spd;

		char type:2;
		char in_use:1;
		char control:2;
		char dir:1;
		char halt:2;

		char onRoute;
		int routeLength;
		struct RoutePath ** Route;
		struct Station * Destination;

		block * Block;

		char timer;
		int timer_id;
	} Trains;

	extern Trains ** trains;
	extern int trains_len;
	extern Engines ** engines;
	extern int engines_len;
	extern Cars ** cars;
	extern int cars_len;
	extern struct train_composition ** trains_comp;
	extern int trains_comp_len;

	#define MAX_TRAINS 30

	extern Trains *train_link[MAX_TRAINS];

	void init_trains();
	void alloc_trains();
	void free_trains();

	int create_train(char * name, int nr_stock, struct train_comp_ws * comps);

	int create_engine(char * name,int DCC,char * img, char * icon, int max, char type, int length);

	int create_car(char * name,int nr,char * img, char * icon, char type, int length);

	int train_read_confs();

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

	extern struct train *trains2[MAX_TRAINS];
	extern struct train *DCC_train[9999];
	extern int iTrain; //Counter for trains in library
	extern int bTrain; //Counter for trains on layout

	int add_train2(int DCC,int speed,char name[],char type);

	int create_train2(int DCC,int speed,char name[],char type);

	void init_trains2();

	int link_train(char link, char train, char type);

	void unlink_train2(char link);

	void *train_timer(void *threadArg);

	void *clear_train_timers();

	void train_speed(block * B, Trains * T, char speed);

	void train_set_speed(Trains *T, char speed);

	void train_set_dir(Trains *T, char dir);

	void train_set_route(Trains *T, struct Station * S);

	void train_stop(Trains * T);

	void train_signal(block * B, Trains * T, int type);

	void train_block_timer();
#endif
