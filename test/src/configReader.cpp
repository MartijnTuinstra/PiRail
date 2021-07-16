#include "catch.hpp"

#include "utils/mem.h"
#include "utils/logger.h"
#include "system.h"

#include "config/configReader.h"
#include "config/LayoutStructure.h"

TEST_CASE("ConfigReader Same Version", "[CR]"){

	struct configStruct_Unit Unit = {1, 2, 3, 4, 5, 6, 7, 8};
	struct configStruct_Block Block0 = {9, 10, {11,12,255}, {13,14,255}, {15,16}, {17,18}, 255, 1000, 0xA8};
	struct configStruct_Switch Switch0 = {19, 20, {21,22,255}, {23,24,255}, {25,26,255}, 0, 14, 255, 10, 0, 0, 0, 0, 0};

	struct configStruct_MSSwitchState MSSwitch0States_[2] = {{{31,32,255}, {33,34,255}, 100, 35, 1}, {{36,37,255}, {38,39,255}, 100, 40, 2}};
	struct configStruct_MSSwitchState * MSSwitch0States = (struct configStruct_MSSwitchState *)calloc(2, sizeof(struct configStruct_MSSwitchState));
	memcpy(MSSwitch0States, MSSwitch0States_, 2 * sizeof(struct configStruct_MSSwitchState));

	struct configStruct_MSSwitch MSSwitch0 = {28, 29, 30, 2, 0, MSSwitch0States, 0};

	char StationName_[16] = "TestStation";
	char * StationName = (char *)calloc(16, sizeof(char));
	strcpy(StationName, StationName_);

	uint8_t StationBlocks_[1] = {0};
	uint8_t * StationBlocks = (uint8_t *)calloc(1, sizeof(uint8_t));
	memcpy(StationBlocks, StationBlocks_, 1);

	struct configStruct_Station Station0 = {41, 1, 12, 42, (2^16)-1, StationBlocks, StationName};

	struct configStruct_SignalDependentSwitch SwitchDependency_[1] = {43, 44, 45};
	struct configStruct_SignalDependentSwitch * SwitchDependency = (struct configStruct_SignalDependentSwitch *)calloc(1, sizeof(struct configStruct_SignalDependentSwitch));
	memcpy(SwitchDependency, SwitchDependency_, 1);
	struct configStruct_SignalEvent SignalEvent_[1] = {{{50,51,52,53,54,55,56,57}}};
	struct configStruct_SignalEvent * SignalEvent = (struct configStruct_SignalEvent *)calloc(1, sizeof(struct configStruct_SignalEvent));
	memcpy(SignalEvent, SignalEvent_, 1);
	struct configStruct_IOport SignalIO_[1] = {{46, 47}};
	struct configStruct_IOport * SignalIO = (struct configStruct_IOport *)calloc(1, sizeof(struct configStruct_IOport));
	memcpy(SignalIO, SignalIO_, 1);

	struct configStruct_Signal Signal0 = {48, 49, {50, 51, 52}, 1, 1, SignalIO, SignalEvent, SwitchDependency};

	uint8_t * buffer = (uint8_t *)calloc(2048, sizeof(char));
	uint8_t * buf_ptr = &buffer[0];

	Config_write_Unit(&Unit, &buf_ptr);
	Config_write_Block(&Block0, &buf_ptr);
	Config_write_Switch(&Switch0, &buf_ptr);
	Config_write_MSSwitch(&MSSwitch0, &buf_ptr);
	Config_write_Station(&Station0, &buf_ptr);
	Config_write_Signal(&Signal0, &buf_ptr);

	printf("%i bytes to be written\n", (unsigned int)(buf_ptr - buffer));

	struct configStruct_Unit ReadUnit;
	struct configStruct_Block Block0Read;
	struct configStruct_Switch Switch0Read;
	struct configStruct_MSSwitch MSSwitch0Read;
	struct configStruct_Station Station0Read;
	struct configStruct_Signal Signal0Read;

	buf_ptr = &buffer[0];

	Config_read_Unit(CONFIG_LAYOUTSTRUCTURE_LU_MAX_VERSION, &ReadUnit, &buf_ptr);
	Config_read_Block(CONFIG_LAYOUTSTRUCTURE_LU_MAX_VERSION, &Block0Read, &buf_ptr);
	Config_read_Switch(CONFIG_LAYOUTSTRUCTURE_LU_MAX_VERSION, &Switch0Read, &buf_ptr);
	Config_read_MSSwitch(CONFIG_LAYOUTSTRUCTURE_LU_MAX_VERSION, &MSSwitch0Read, &buf_ptr);
	Config_read_Station(CONFIG_LAYOUTSTRUCTURE_LU_MAX_VERSION, &Station0Read, &buf_ptr);
	Config_read_Signal(CONFIG_LAYOUTSTRUCTURE_LU_MAX_VERSION, &Signal0Read, &buf_ptr);

	// Unit
	CHECK(ReadUnit.Module == Unit.Module);
	CHECK(ReadUnit.Connections == Unit.Connections);
	CHECK(ReadUnit.IO_Nodes == Unit.IO_Nodes);
	CHECK(ReadUnit.Blocks == Unit.Blocks);
	CHECK(ReadUnit.Switches == Unit.Switches);
	CHECK(ReadUnit.MSSwitches == Unit.MSSwitches);
	CHECK(ReadUnit.Signals == Unit.Signals);
	CHECK(ReadUnit.Stations == Unit.Stations);

	// Block
	CHECK(Block0Read.id == Block0.id);
	CHECK(Block0Read.type == Block0.type);

	CHECK(Block0Read.next.module == Block0.next.module);
	CHECK(Block0Read.next.id == Block0.next.id);
	CHECK(Block0Read.next.type == Block0.next.type);

	CHECK(Block0Read.prev.module == Block0.prev.module);
	CHECK(Block0Read.prev.id == Block0.prev.id);
	CHECK(Block0Read.prev.type == Block0.prev.type);

	CHECK(Block0Read.IOdetection.Node == Block0.IOdetection.Node);
	CHECK(Block0Read.IOdetection.Port == Block0.IOdetection.Port);

	CHECK(Block0Read.IOpolarity.Node == Block0.IOpolarity.Node);
	CHECK(Block0Read.IOpolarity.Port == Block0.IOpolarity.Port);

	CHECK(Block0Read.Polarity == Block0.Polarity);
	CHECK(Block0Read.Polarity_IO[0].Node == Block0.Polarity_IO[0].Node);
	CHECK(Block0Read.Polarity_IO[0].Port == Block0.Polarity_IO[0].Port);

	CHECK(Block0Read.Polarity_IO[1].Node == Block0.Polarity_IO[1].Node);
	CHECK(Block0Read.Polarity_IO[1].Port == Block0.Polarity_IO[1].Port);

	CHECK(Block0Read.speed == Block0.speed);
	CHECK(Block0Read.length	 == Block0.length);
	CHECK(Block0Read.fl == Block0.fl);

	// Switch
	CHECK(Switch0Read.id == Switch0.id);
	CHECK(Switch0Read.det_block == Switch0.det_block);

	CHECK(Switch0Read.App.module == Switch0.App.module);
	CHECK(Switch0Read.App.id == Switch0.App.id);
	CHECK(Switch0Read.App.type == Switch0.App.type);

	CHECK(Switch0Read.Str.module == Switch0.Str.module);
	CHECK(Switch0Read.Str.id == Switch0.Str.id);
	CHECK(Switch0Read.Str.type == Switch0.Str.type);

	CHECK(Switch0Read.Div.module == Switch0.Div.module);
	CHECK(Switch0Read.Div.id == Switch0.Div.id);
	CHECK(Switch0Read.Div.type == Switch0.Div.type);

	CHECK(Switch0Read.IO_length == Switch0.IO_length);
	CHECK(Switch0Read.IO_type == Switch0.IO_type);
	CHECK(Switch0Read.speed_Str == Switch0.speed_Str);
	CHECK(Switch0Read.speed_Div == Switch0.speed_Div);

	CHECK(Switch0Read.feedback_len == Switch0.feedback_len);

	// MSSwitch
	CHECK(MSSwitch0Read.id == MSSwitch0.id);
	CHECK(MSSwitch0Read.det_block == MSSwitch0.det_block);
	CHECK(MSSwitch0Read.type == MSSwitch0.type);
	CHECK(MSSwitch0Read.nr_states == MSSwitch0.nr_states);
	CHECK(MSSwitch0Read.IO == MSSwitch0.IO);

	CHECK(MSSwitch0Read.states[0].sideA.module == MSSwitch0.states[0].sideA.module);
	CHECK(MSSwitch0Read.states[0].sideA.id == MSSwitch0.states[0].sideA.id);
	CHECK(MSSwitch0Read.states[0].sideA.type == MSSwitch0.states[0].sideA.type);
	CHECK(MSSwitch0Read.states[0].sideB.module == MSSwitch0.states[0].sideB.module);
	CHECK(MSSwitch0Read.states[0].sideB.id == MSSwitch0.states[0].sideB.id);
	CHECK(MSSwitch0Read.states[0].sideB.type == MSSwitch0.states[0].sideB.type);
	CHECK(MSSwitch0Read.states[0].speed == MSSwitch0.states[0].speed);
	CHECK(MSSwitch0Read.states[0].dir == MSSwitch0.states[0].dir);
	CHECK(MSSwitch0Read.states[0].output_sequence == MSSwitch0.states[0].output_sequence);

	CHECK(MSSwitch0Read.states[1].sideA.module == MSSwitch0.states[1].sideA.module);
	CHECK(MSSwitch0Read.states[1].sideA.id == MSSwitch0.states[1].sideA.id);
	CHECK(MSSwitch0Read.states[1].sideA.type == MSSwitch0.states[1].sideA.type);
	CHECK(MSSwitch0Read.states[1].sideB.module == MSSwitch0.states[1].sideB.module);
	CHECK(MSSwitch0Read.states[1].sideB.id == MSSwitch0.states[1].sideB.id);
	CHECK(MSSwitch0Read.states[1].sideB.type == MSSwitch0.states[1].sideB.type);
	CHECK(MSSwitch0Read.states[1].speed == MSSwitch0.states[1].speed);
	CHECK(MSSwitch0Read.states[1].dir == MSSwitch0.states[1].dir);
	CHECK(MSSwitch0Read.states[1].output_sequence == MSSwitch0.states[1].output_sequence);

	// Station
	CHECK(Station0Read.type == Station0.type);
	CHECK(Station0Read.nr_blocks == Station0.nr_blocks);
	CHECK(Station0Read.name_len == Station0.name_len);
	CHECK(Station0Read.reserved == Station0.reserved);
	CHECK(Station0Read.parent == Station0.parent);

	for(int i = 0; i < Station0Read.nr_blocks; i++){
		CHECK(Station0Read.blocks[i] == Station0.blocks[i]);
	}
	CHECK(strcmp(Station0Read.name, Station0.name) == 0);

	// Signals
	CHECK(Signal0Read.direction == Signal0.direction);
	CHECK(Signal0Read.id == Signal0.id);

	CHECK(Signal0Read.block.module == Signal0.block.module);
	CHECK(Signal0Read.block.id == Signal0.block.id);
	CHECK(Signal0Read.block.type == Signal0.block.type);

	CHECK(Signal0Read.output_len == Signal0.output_len);
	CHECK(Signal0Read.Switch_len == Signal0.Switch_len);

	CHECK(Signal0Read.output[0].Node == Signal0.output[0].Node);
	CHECK(Signal0Read.output[0].Port == Signal0.output[0].Port);

	for(int i = 0; i < 8; i++)
		CHECK(Signal0Read.stating[0].event[i] == Signal0.stating[0].event[i]);

	CHECK(Signal0Read.Switches[0].type == Signal0.Switches[0].type);
	CHECK(Signal0Read.Switches[0].Sw == Signal0.Switches[0].Sw);
	CHECK(Signal0Read.Switches[0].state == Signal0.Switches[0].state);

	free(MSSwitch0States);
	free(MSSwitch0Read.states);
	free(MSSwitch0Read.IO_Ports);

	free(StationName);
	free(StationBlocks);
	free(Station0Read.name);
	free(Station0Read.blocks);

	free(SwitchDependency);
	free(SignalEvent);
	free(SignalIO);
	free(Switch0Read.IO_Ports);
	free(Switch0Read.IO_Event);;
	free(Switch0Read.FB_Ports);
	free(Switch0Read.FB_Event);

	free(Signal0Read.output);
	free(Signal0Read.stating);
	free(Signal0Read.Switches);

	free(buffer);
}

TEST_CASE("ConfigReader oldVersion", "[CR]"){
	uint8_t buffer2[100] = {
		0x10, 0x20, 0x01, 0x10, 0x01, 0xFF, 0x02, 0x02, 0x20, 0xFF, 0xFF, 0x10, 0x01, 0x00
	};

	uint8_t * buffer = (uint8_t *)calloc(25, sizeof(uint8_t));
	uint8_t * buf_ptr = &buffer[0];
	memcpy(buffer, buffer2, 14);

	uint8_t fileVersion = 0;

	struct configStruct_Block Block0Read;
	printf("Read Block\n");
	Config_read_Block(fileVersion, &Block0Read, &buf_ptr);

	CHECK(Block0Read.id == 0x10);
	CHECK(Block0Read.type == 0x20);
	CHECK(Block0Read.next.module == 0x01);
	CHECK(Block0Read.next.id == 0x0110);
	CHECK(Block0Read.next.type == 0xFF);
	CHECK(Block0Read.prev.module == 0x02);
	CHECK(Block0Read.prev.id == 0x2002);
	CHECK(Block0Read.prev.type == 0xFF);
	CHECK(Block0Read.speed == 0xFF);
	CHECK(Block0Read.length == 0x0110);
	CHECK(Block0Read.fl == 0x00);

	free(buffer);
}