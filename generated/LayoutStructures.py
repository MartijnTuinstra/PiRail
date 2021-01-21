from .layoutGenerator import *

versionLookup = [[0,0,0,0,0,0,0,0,0,0,0,0,0], 
                 [0,0,0,1,0,0,0,0,0,0,0,0,0], # UnitConfig Update
                 [0,0,0,1,1,0,0,0,0,0,0,0,0]  # BlockConfig Update
                ]

CFL = ConfigFileLayout("LayoutStructure", versionLookup, includePath="config/", sourcePath="generated/src/config/", headerPath="generated/lib/config/")

# CFL.addCustomStructure("\nstruct module_configNew {\n"
#                         "  struct configStruct_Unit header;\n\n"
#                         "  struct configStruct_Node * Nodes;\n\n" 
#                         "  struct configStruct_Block * Blocks;\n"
#                         "  struct configStruct_Switch * Switches;\n"
#                         "  struct configStruct_MSSwitch * MSSwitches;"
#                         "  struct configStruct_Station * Stations;\n"
#                         "  struct configStruct_Signals * Signals;\n\n"
#                         "  uint16_t Layout_length;\n"
#                         "  char * Layout;\n};\n")


RailLinkConfig = CFL.addStructure("RailLink",
	[
	 StF(0,"module", FT.U8),
	 StF(1,"id",     FT.U16),
	 StF(2,"type",   FT.U8)
	],
	[[FF(FT.U8, 0), FF(FT.U16, 1), FF(FT.U8, 2)]]
)

IOConfig = CFL.addStructure("IOport",
	[
	 StF(0, "Node", FT.U8),
	 StF(1, "Port", FT.U16)
	],
	[[FF(FT.U8, 0), FF(FT.U16, 1)]]
)

NodeConfig = CFL.addStructure("Node",
	[
		StF(0, "Node",   FT.U8),
		StF(1, "size",   FT.U8),
		StF(2, "data",   FT.LIST, nested=FT.U8)
	],
	[[FF(FT.U8, 0), FF(FT.U8, 1), FF(FT.LIST | FT.U8, 2, lengthField=1)]]
)

UnitConfig = CFL.addStructure("Unit",
	[
		StF(0,"Module",      FT.U8),
		StF(1,"Connections", FT.U8),
		StF(2,"IO_Nodes",    FT.U8),
		StF(3,"Blocks",      FT.U16),
		StF(4,"Switches",    FT.U16),
		StF(5,"MSSwitches",  FT.U16),
		StF(6,"Signals",     FT.U16),
		StF(7,"Stations",    FT.U16)
	],
	[[FF(FT.U8, 0), FF(FT.U8, 1), FF(FT.U8, 2), FF(FT.U16, 3), FF(FT.U16, 4), FF(FT.U16, 5), FF(FT.U16, 6), FF(FT.U8,  7)], 
	 [FF(FT.U8, 0), FF(FT.U8, 1), FF(FT.U8, 2), FF(FT.U16, 3), FF(FT.U16, 4), FF(FT.U16, 5), FF(FT.U16, 6), FF(FT.U16, 7)]]
)

BlockConfig = CFL.addStructure("Block", 
	[
		StF(0,"id",     FT.U8),
		StF(1,"type",   FT.U8),
		StF(2,"next",   RailLinkConfig),
		StF(3,"prev",   RailLinkConfig),
		StF(4,"IO_In",  IOConfig),
		StF(5,"IO_Out", IOConfig),
		StF(6,"speed",  FT.U8),
		StF(7,"length", FT.U16),
		StF(8,"fl",     FT.U8)
	],
	[[FF(FT.U8, 0), FF(FT.U8, 1), FF(RailLinkConfig, 2, version=0), FF(RailLinkConfig, 3, version=0),
	                FF(FT.U8, 6), FF(FT.U16, 7), FF(FT.U8, 8)],
	 [FF(FT.U8, 0), FF(FT.U8, 1), FF(RailLinkConfig, 2, version=0), FF(RailLinkConfig, 3, version=0),
	                FF(IOConfig, 4, version=0), FF(IOConfig, 5, version=0), FF(FT.U8, 6), FF(FT.U16, 7), FF(FT.U8, 8)]
	]
)

SwitchConfig = CFL.addStructure("Switch",
	[
		StF(0,"id",           FT.U8), #0
		StF(1,"det_block",    FT.U8),
		StF(2,"App",          RailLinkConfig), #2
		StF(3,"Str",          RailLinkConfig),
		StF(4,"Div",          RailLinkConfig),
		StF(5,"IO_length",    FT.U8), #5
		StF(6,"IO_type",      FT.U8),
		StF(7,"speed_Str",    FT.U8), #7
		StF(8,"speed_Div",    FT.U8),
		StF(9,"feedback_len", FT.U8), #9

		StF(10,"IO_Ports",     FT.LIST, nested=IOConfig),
		StF(11,"IO_Event",     FT.LIST, nested=FT.U8),

		StF(12,"FB_Ports",     FT.LIST, nested=IOConfig),
		StF(13,"FB_Event",     FT.LIST, nested=FT.U8)
	],
	[[FF(FT.U8, 0), FF(FT.U8, 1), FF(RailLinkConfig, 2, version=0), FF(RailLinkConfig, 3, version=0), FF(RailLinkConfig, 4, version=0),
	                FF(FT.U8 | FT.BIT, 5, offset=0, width=4), FF(FT.U8 | FT.BIT, 6, offset=4, width=4), FF(FT.U8, 7), FF(FT.U8, 8), FF(FT.U8, 9),

	                FF(FT.LIST | FT.NESTED, 10, lengthField=5, nested=IOConfig, version=0), FF(FT.LIST | FT.U8, 11, lengthField=5, sizeMultiply=2),
	                FF(FT.LIST | FT.NESTED, 12, lengthField=9, nested=IOConfig, version=0), FF(FT.LIST | FT.U8, 13, lengthField=9, sizeMultiply=2)
	]]
) 

MSSwitchStateConfig = CFL.addStructure("MSSwitchState",
	[
		StF(0, "sideA",           RailLinkConfig),
		StF(1, "sideB",           RailLinkConfig),
		StF(2, "speed",           FT.U16),
		StF(3, "dir",             FT.U8),
		StF(4, "output_sequence", FT.U8)
	],
	[[FF(RailLinkConfig, 0, version=0), FF(RailLinkConfig, 1, version=0), FF(FT.U16, 2), FF(FT.U8, 3), FF(FT.U8, 4)]]
)

MSSwitchConfig = CFL.addStructure("MSSwitch",
	[
		StF(0,"id",        FT.U8),
		StF(1,"det_block", FT.U8),
		StF(2,"type",      FT.U8),
		StF(3,"nr_states", FT.U8),
		StF(4,"IO",        FT.U8),

		StF(5,"states",    FT.LIST, nested=MSSwitchStateConfig),
		StF(6,"IO_Ports",  FT.LIST, nested=IOConfig)
	],
	[[FF(FT.U8, 0), FF(FT.U8, 1), FF(FT.U8, 2), FF(FT.U8, 3), FF(FT.U8, 4),
	  FF(FT.LIST | FT.NESTED, 5, lengthField=3, nested=MSSwitchStateConfig, version=0), FF(FT.LIST | FT.NESTED, 6, lengthField=4, nested=IOConfig, version=0)
	]]
)

StationConfig = CFL.addStructure("Station",
	[
		StF(0,"type",      FT.U8),
		StF(1,"nr_blocks", FT.U8),
		StF(2,"name_len",  FT.U8),
		StF(3,"reserved",  FT.U8),
		StF(4,"parent",    FT.U16),

		StF(5,"blocks",    FT.LIST, nested=FT.U8),
		StF(6,"name",      FT.LIST, nested=FT.CHAR)
	],
	[[FF(FT.U8, 0), FF(FT.U8, 1), FF(FT.U8, 2), FF(FT.U8, 3), FF(FT.U16, 4),
	  FF(FT.LIST | FT.U8, 5, lengthField=1), FF(FT.LIST | FT.CHAR, 6, lengthField=2)
	]]
)

SignalDependentSwitch = CFL.addStructure("SignalDependentSwitch",
	[
		StF(0,"type",  FT.U8),
		StF(1,"Sw",    FT.U8),
		StF(2,"state", FT.U8)
	],
	[[FF(FT.U8, 0), FF(FT.U8, 1), FF(FT.U8, 2)]]
)

SignalEvent = CFL.addStructure("SignalEvent",
	[StF(0,"event", FT.LIST, nested=FT.U8, fixedSize=8)],
	[[FF(FT.LIST | FT.U8, 0)]]
)

SignalConfig = CFL.addStructure("Signal",
	[
		StF(0,"direction",  FT.U8),
		StF(1,"id",         FT.U16),
		StF(2,"block",      RailLinkConfig),
		StF(3,"output_len", FT.U8),
		StF(4,"Switch_len", FT.U8),

		StF(5,"output",     FT.LIST, nested=IOConfig),
		StF(6,"stating",    FT.LIST, nested=SignalEvent),
		StF(7,"Switches",   FT.LIST, nested=SignalDependentSwitch)
	],
	[[FF(FT.U8, 0), FF(FT.U16, 1), FF(RailLinkConfig, 2, version=0), FF(FT.U8, 3), FF(FT.U8, 4),
	  FF(FT.LIST | FT.NESTED, 5, lengthField=3, nested=IOConfig, version=0), FF(FT.LIST | FT.NESTED, 6, lengthField=3, nested=SignalEvent, version=0),
	  FF(FT.LIST | FT.NESTED, 7, lengthField=4, nested=SignalDependentSwitch, version=0)
	]]
)

LayoutConfig = CFL.addStructure("WebLayout",
	[
		StF(0, "LayoutLength", FT.U16),
		StF(1, "Layout",       FT.LIST, nested=FT.CHAR)
	],
	[[FF(FT.U16, 0), FF(FT.LIST | FT.CHAR, 1, lengthField=0)]]
)



CFL.writeToFile()