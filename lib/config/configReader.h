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


void configEditor_preview_string(char * buffer, const char * s);
void configEditor_preview_uint8_t(char * buffer, uint8_t i);
void configEditor_preview_uint16_t(char * buffer, uint16_t i);
void configEditor_preview_bool(char * buffer, uint8_t b);
void configEditor_preview_string(char * buffer, char * s);
void configEditor_scan_uint8_t(char * buffer, uint8_t * i);
void configEditor_scan_uint16_t(char * buffer, uint16_t * i);
void configEditor_scan_bool(char * buffer, uint8_t * b);
void configEditor_scan_bool(char * buffer, bool * b);
void configEditor_scan_string(char * buffer, char ** s, uint8_t * l);

#endif