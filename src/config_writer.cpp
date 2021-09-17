#include "config/configReader.h"
#include "config/LayoutStructure.h"

void hexdump(void * data, int length){
  printf("HEXDUMP (%x) %i:\n", (unsigned long)data, length);
  char text[2000];
  char * ptr = text;

  for(int i = 0; i < length; i++){
    ptr += sprintf(ptr, "%02x ", ((uint8_t *)data)[i]);
    if((i % 16) == 15)
      ptr += sprintf(ptr, "\n");
  }

  // f(INFO, file, line, (const char *)text);
  printf("%s", text);
}

int main(int argc, char ** argf){


// struct configStruct_RailLink
// {
//   uint8_t Module;
//   uint16_t ID;
//   uint8_t Type;
// };

// struct configStruct_IOport
// {
//   uint8_t Node;
//   uint16_t Port;
// };

// struct configStruct_Node
// {
//   uint8_t Node;
//   uint8_t size;
//   uint8_t * data;
// };

struct configStruct_Unit Unit = {1, 2, 3, 4, 5, 6, 7, 8};

// struct configStruct_Unit
// {
//   uint8_t Module;
//   uint8_t Connections;
//   uint8_t IO_Nodes;
//   uint16_t Blocks;
//   uint16_t Switches;
//   uint16_t MSSwitches;
//   uint16_t Signals;
//   uint16_t Stations;
// };
struct configStruct_Block Block0 = {9, 10, {11,12,255}, {13,14,255}, {15,16}, {17,18}, 255, 1000, 0xA8};
// struct configStruct_Block
// {
//   uint8_t id;
//   uint8_t type;
//   struct configStruct_RailLink next;
//   struct configStruct_RailLink prev;
//   struct configStruct_IOport IO_In;
//   struct configStruct_IOport IO_Out;
//   uint8_t speed;
//   uint16_t length;
//   uint8_t fl;
// };
struct configStruct_Switch Switch0 = {19, 20, {21,22,255}, {23,24,255}, {25,26,255}, 0, 14, 255, 10, 0, 0, 0, 0, 0};
// struct configStruct_Switch
// {
//   uint8_t id;
//   uint8_t det_block;
//   struct configStruct_RailLink App;
//   struct configStruct_RailLink Str;
//   struct configStruct_RailLink Div;
//   uint8_t IO_length;
//   uint8_t IO_type;
//   uint8_t speed_Str;
//   uint8_t speed_Div;
//   uint8_t feedback_len;
//   struct configStruct_IOport * IO_Ports;
//   uint8_t * IO_Event;
//   struct configStruct_IOport * FB_Ports;
//   uint8_t * FB_Event;
// };
struct configStruct_MSSwitchState MSSwitch0States_[2] = {{{31,32,255}, {33,34,255}, 100, 35, 1}, {{36,37,255}, {38,39,255}, 100, 40, 2}};
struct configStruct_MSSwitchState * MSSwitch0States = (struct configStruct_MSSwitchState *)_calloc(2, struct configStruct_MSSwitchState);
memcpy(MSSwitch0States, MSSwitch0States_, 2 * sizeof(struct configStruct_MSSwitchState));
// struct configStruct_MSSwitchState
// {
//   struct configStruct_RailLink sideA;
//   struct configStruct_RailLink sideB;
//   uint16_t speed;
//   uint8_t dir;
//   uint8_t output_sequence;
// };
struct configStruct_MSSwitch MSSwitch0 = {28, 29, 30, 2, 0, MSSwitch0States, 0};
// struct configStruct_MSSwitch
// {
//   uint8_t id;
//   uint8_t det_block;
//   uint8_t type;
//   uint8_t nr_states;
//   uint8_t IO;
//   struct configStruct_MSSwitchState * states;
//   struct configStruct_IOport * IO_Ports;
// };
char StationName_[16] = "TestStation";
char * StationName = (char *)_calloc(16, char);
strcpy(StationName, StationName_);

uint8_t StationBlocks_[1] = {0};
uint8_t * StationBlocks = (uint8_t *)_calloc(1, uint8_t);
memcpy(StationBlocks, StationBlocks_, 1);

struct configStruct_Station Station0 = {41, 1, 12, 42, (2^16)-1, StationBlocks, StationName};
// struct configStruct_Station
// {
//   uint8_t type;
//   uint8_t nr_blocks;
//   uint8_t name_len;
//   uint8_t reserved;
//   uint16_t parent;
//   uint8_t * blocks;
//   char * name;
// };

struct configStruct_SignalDependentSwitch SwitchDependency_[1] = {43, 44, 45};
struct configStruct_SignalDependentSwitch * SwitchDependency = (struct configStruct_SignalDependentSwitch *)_calloc(1, configStruct_SignalDependentSwitch);
memcpy(SwitchDependency, SwitchDependency_, 1);
// struct configStruct_SignalDependentSwitch
// {
//   uint8_t type;
//   uint8_t Sw;
//   uint8_t state;
// };

struct configStruct_SignalEvent SignalEvent_[1] = {{{50,51,52,53,54,55,56,57}}};
struct configStruct_SignalEvent * SignalEvent = (struct configStruct_SignalEvent *)_calloc(1, configStruct_SignalEvent);
memcpy(SignalEvent, SignalEvent_, 1);
// struct configStruct_SignalEvent
// {
//   uint8_t event[8];
// };


struct configStruct_IOport SignalIO_[1] = {{46, 47}};
struct configStruct_IOport * SignalIO = (struct configStruct_IOport *)_calloc(1, configStruct_IOport);
memcpy(SignalIO, SignalIO_, 1);

struct configStruct_Signal Signal0 = {48, 49, {50, 51, 52}, 1, 1, SignalIO, SignalEvent, SwitchDependency};
// struct configStruct_Signal
// {
//   uint8_t direciton;
//   uint16_t id;
//   struct configStruct_RailLink block;
//   uint8_t output_len;
//   uint8_t Switch_len;
//   struct configStruct_IOport * output;
//   struct configStruct_SignalEvent * stating;
//   struct configStruct_SignalDependentSwitch * Switches;
// };

	uint8_t * buffer = (uint8_t *)_calloc(2048, 1);
	uint8_t * start = buffer;

	printf("Unit\n");
	Config_write_Unit(&Unit, &buffer);
	printf("Block\n");
	Config_write_Block(&Block0, &buffer);
	printf("Switch\n");
	Config_write_Switch(&Switch0, &buffer);
	printf("MSSwitch\n");
	Config_write_MSSwitch(&MSSwitch0, &buffer);
	printf("Station\n");
	Config_write_Station(&Station0, &buffer);
	Config_write_Signal(&Signal0, &buffer);

	printf("%i bytes to be written\n", (unsigned long)(buffer - start));

	hexdump(start, (unsigned long)(buffer - start));

	struct configStruct_Unit ReadUnit;
	struct configStruct_Block Block0Read;
	struct configStruct_Switch Switch0Read;
	struct configStruct_MSSwitch MSSwitch0Read;
	struct configStruct_Station Station0Read;
	struct configStruct_Signal Signal0Read;

	buffer = start;

	Config_read_Unit_1(&ReadUnit, &buffer);
	Config_read_Block_1(&Block0Read, &buffer);
	Config_read_Switch_0(&Switch0Read, &buffer);
	Config_read_MSSwitch_0(&MSSwitch0Read, &buffer);
	Config_read_Station_0(&Station0Read, &buffer);
	Config_read_Signal_0(&Signal0Read, &buffer);

	printf("\n %i bytes read\n", (unsigned long)(buffer - start));



	return 0;
}