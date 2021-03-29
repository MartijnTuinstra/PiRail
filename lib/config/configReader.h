#ifndef INCLUDE_CONFIG_CONFIGREADER_H
#define INCLUDE_CONFIG_CONFIGREADER_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "utils/mem.h"

void Config_read_uint8_t_uint8_t(uint8_t * put, uint8_t ** get);
void Config_read_uint8_t_uint16_t(uint16_t * put, uint8_t ** get);
void Config_read_uint16_t_uint16_t(uint16_t * put, uint8_t ** get);
void Config_read_uint16_t_uint8_t(uint8_t * put, uint8_t ** get);

void Config_read_BitField(uint8_t * put, uint8_t get, uint8_t bitOffset, uint8_t bitSize);
void Config_read_BitField(uint8_t * put, uint16_t get, uint8_t bitOffset, uint8_t bitSize);
void Config_read_BitField(uint16_t * put, uint16_t get, uint8_t bitOffset, uint8_t bitSize);
void Config_read_BitField(uint32_t * put, uint32_t get, uint8_t bitOffset, uint8_t bitSize);

void Config_write_uint8_t(uint8_t * get, uint8_t ** put);
void Config_write_uint16_t(uint16_t * get, uint8_t ** put);

void Config_write_BitField(uint8_t get, uint8_t * put, uint8_t bitOffset, uint8_t bitSize);
void Config_write_BitField(uint8_t  get, uint16_t * put, uint8_t bitOffset, uint8_t bitSize);
void Config_write_BitField(uint16_t get, uint16_t * put, uint8_t bitOffset, uint8_t bitSize);
void Config_write_BitField(uint32_t get, uint32_t * put, uint8_t bitOffset, uint8_t bitSize);

#endif