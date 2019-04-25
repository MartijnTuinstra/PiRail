#include "logger.h"
#include "mem.h"
#include "train.h"
#include "Z21.h"

void Z21_Set_Loco_Drive_Engine(Engines * E){
	uint8_t * data = _calloc(11, 1);
	data[0] = 0x0A;
	data[2] = 0x40;
	data[4] = 0xE4;
	data[5] = 0x10; // Speed speed_step_type
	if(E->speed_step_type == TRAIN_28_FAHR_STUFEN){ // 28 steps
		data[5] |= 2;
	}
	else if(E->speed_step_type == TRAIN_128_FAHR_STUFEN){ // 128 steps
		data[5] |= 3;
	}
	data[6] = 0xC0 | ((E->DCC_ID & 0x3F00) >> 8);
	data[7] = E->DCC_ID & 0xFF;
	data[8] = ((E->dir & 1) << 7) | (E->speed & 0x7F);
	data[9] = data[4] ^ data[5] ^ data[6] ^ data[7] ^ data[8];

	Z21_send_data(data, 10);
	_free(data);
}

void Z21_Set_Loco_Drive_Train(Trains * T){
	uint8_t * data = _calloc(T->nr_engines * 10 + 2, 1);
	uint16_t i = 0;

	for(int e = 0; e < T->nr_engines; e++){
		Engines * E = T->engines[e];

		data[i] = 0x0A;
		data[i+2] = 0x40;
		data[i+4] = 0xE4;
		data[i+5] = 0x10;
		if(E->speed_step_type == TRAIN_28_FAHR_STUFEN){ // 28 steps
			data[i+5] |= 2;
		}
		else if(E->speed_step_type == TRAIN_128_FAHR_STUFEN){ // 128 steps
			data[i+5] |= 3;
		}
		data[i+6] = 0xC0 | ((E->DCC_ID & 0x3F00) >> 8);
		data[i+7] = E->DCC_ID & 0xFF;
		data[i+8] = ((E->dir & 1) << 7) | (E->speed & 0x7F);
		data[i+9] = data[i+4] ^ data[i+5] ^ data[i+6] ^ data[i+7] ^ data[i+8];
		i += 10;
	}

	Z21_send_data(data, i);
	_free(data);
}

void Z21_LAN_X_LOCO_INFO(uint8_t length, char * data){
	uint16_t DCC_ID = ((data[0] & 0x3F) << 8) + data[1];
	// uint8_t xbusRegeler = (data[2] & 0x8) >> 3;
	// uint8_t stufen = (data[2] & 0x7);
	uint8_t dir = (data[3] & 0x80) >> 7;
	uint8_t speed = (data[3] & 0x7F);

	//Functions ....

	Engines * E = DCC_train[DCC_ID];

	if(!E){
		loggerf(ERROR, "No train with DCC address %i\n", DCC_ID);
		return;
	}

	E->speed = speed;
	E->dir = dir;

	engine_calc_real_speed(E);

	if(E->train){
		loggerf(INFO, "Engine part of train");

		train_set_speed(E->train, E->cur_speed);

		WS_UpdateTrain(E->train, TRAIN_TRAIN_TYPE);
	}
	else{
		WS_UpdateTrain(E, TRAIN_ENGINE_TYPE);
	}
}