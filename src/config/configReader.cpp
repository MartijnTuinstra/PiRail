#include "config/configReader.h"

void Config_read_uint8_t_uint8_t(uint8_t * put, uint8_t ** get){
	#ifdef DEBUG
	printf("r u8u8   %8x: %i\n", (unsigned int)*get, *get[0]);
	#endif
	*put = (*get)[0];
	*get += sizeof(uint8_t);
}

void Config_read_uint8_t_uint16_t(uint16_t * put, uint8_t ** get){
	#ifdef DEBUG
	printf("r u8u16  %8x: %i\n", (unsigned int)*get, *get[0]);
	#endif
	*put = (*get)[0];
	*get += sizeof(uint8_t);
}

void Config_read_uint16_t_uint16_t(uint16_t * put, uint8_t ** get){
	#ifdef DEBUG
	printf("r u16u16 %8x: %i\n", (unsigned int)*get, ((uint16_t *)*get)[0]);
	#endif
	*put = ((uint16_t *)*get)[0];
	*get += sizeof(uint16_t);
}

void Config_read_uint16_t_uint8_t(uint8_t * put, uint8_t ** get){
	#ifdef DEBUG
	printf("r u16u8 %8x: %i\n", (unsigned int)*get, ((uint16_t *)*get)[0]);
	#endif
	uint16_t tmp = ((uint16_t *)*get)[0];
	*put = tmp & 0xFF;
	*get += sizeof(uint16_t);
}

void Config_read_BitField(uint8_t * put, uint8_t get, uint8_t bitOffset, uint8_t bitSize){
	uint8_t mask = 0;

	for(uint8_t i = 0; i < (bitSize - 1); i++){
		mask |= 1;
		mask <<= 1;
	}
	mask |= 1;

	mask <<= bitOffset;

	#ifdef DEBUG
	printf("r bitu8          : (%i & %x) >> %i -> %i\n", get, mask, bitOffset, (get & mask) >> bitOffset);
	#endif

	*put = (get & mask) >> bitOffset;
}

void Config_read_BitField(uint16_t * put, uint16_t get, uint8_t bitOffset, uint8_t bitSize){
}

void Config_read_BitField(uint32_t * put, uint32_t get, uint8_t bitOffset, uint8_t bitSize){

}

void Config_write_uint8_t(uint8_t * get, uint8_t ** put){
	#ifdef DEBUG
	printf("w u8u8   %8x: %i\n", (unsigned int)*put, *get);
	#endif
	(*put)[0] = *get;
	*put += sizeof(uint8_t);
}

void Config_write_uint16_t(uint16_t * get, uint8_t ** put){
	#ifdef DEBUG
	printf("w u16u16 %8x: %i\n", (unsigned int)*put, *get);
	#endif
	*(uint16_t *)*put = *get;
	*put += sizeof(uint16_t);
}

void Config_write_BitField(uint8_t get, uint8_t * put, uint8_t bitOffset, uint8_t bitSize){
	uint8_t mask = 0;

	for(uint8_t i = 0; i < (bitSize - 1); i++){
		mask |= 1;
		mask <<= 1;
	}
	mask |= 1;

	#ifdef DEBUG
	printf("w bitu8          : (%i & %x) << %i -> %i | %i\n", get, mask, bitOffset, *put, (get & mask) << bitOffset);
	#endif

	*put = *put | ((get & mask) << bitOffset);
}
void Config_write_BitField(uint16_t * get, uint16_t put, uint8_t bitOffset, uint8_t bitSize);
void Config_write_BitField(uint32_t * get, uint32_t put, uint8_t bitOffset, uint8_t bitSize);

