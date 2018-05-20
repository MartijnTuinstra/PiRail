
#ifndef _INCLUDE_SIGNALS_H
	#define _INCLUDE_SIGNALS_H

	/**///Signal passing speed
	//Flashing Red speed
	#define RED_F_SPEED 20
	//Flashing Amber speed
	#define AMBER_F_SPEED 30
	//Amber speed
	#define AMBER_SPEED 40



	struct signal{
	  int id;
	  int MAdr;
	  int UAdr;
	  int state;
	  int type; // 0 = 2-state / 1 = 4-state / 2 = 8-state
	  uint8_t length;

	  short adr[6];
	  char states[BLOCK_STATES];
	  char flash[BLOCK_STATES];
	};

	struct Seg;

	struct Unit;

	void create_signal2(struct Seg * B,char adr_nr, uint8_t addresses[adr_nr], char state[BLOCK_STATES], char flash[BLOCK_STATES], char side);

	void set_signal(struct signal *Si,int state);

	#define SIG_GREEN      0
	#define SIG_AMBER      1
	#define SIG_RED        2
	#define SIG_RESTRICTED 3
	#define SIG_CAUTION    4
#endif