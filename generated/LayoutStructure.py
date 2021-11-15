from .layoutGenerator import *
# from .configReaderGenerator import *

versionLookup = [[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], 
                 [0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0], # UnitConfig Update
                 [0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0], # BlockConfig Update
                 [0,0,0,0,1,1,0,0,0,0,1,0,0,0,0,0,0], # StationConfig Update
                 [0,0,0,0,1,2,0,0,0,0,1,0,0,0,0,0,0], # BlockConfig Update, flags
                 [0,0,0,0,2,2,0,0,0,0,1,0,0,0,0,0,0], # Unit Header update
                 [0,0,0,0,2,2,1,0,0,0,1,0,0,0,0,0,0], # PolarityGroup update
                 [0,0,0,0,2,2,1,0,0,0,1,0,0,1,0,0,0]  # Signal update
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
    programStructure = [
     StF(0,"module", FT.U8),
     StF(1,"id",     FT.U16),
     StF(2,"type",   FT.U8)
    ],
    fileStructure = [
        [FF(FT.U8, 0), FF(FT.U16, 1), FF(FT.U8, 2)]
    ],
    Scannable = ScanEntry("%i%*c%i%*c%i", [0,1,2]),
    Preview = PreviewEntry("%02i:%02i:%s", [0,1,2], textIndex=[None,None,"S_RailLinkTypes"])
)

IOConfig = CFL.addStructure("IOport",
    programStructure = [
     StF(0, "Node", FT.U8),
     StF(1, "Port", FT.U16)
    ],
    fileStructure = [
        [FF(FT.U8, 0), FF(FT.U16, 1)]
    ],
    Scannable = ScanEntry("%i%*c%i", [0,1]),
    Preview = PreviewEntry("%02i:%02i", [0,1])
)

NodeIOConfig = CFL.addStructure("NodeIO",
    [
        StF(0, "type",   FT.U8),
        StF(1, "defaultState", FT.U8),
        StF(2, "inverted", FT.U8),

        StF(3, "reserved", FT.U8)
    ],
    [[FF(FT.U16 | FT.BIT, [0, 1, 2, 3], offset=[0, 4, 8, 9], width=[4, 4, 1, 7])]]
)

NodeConfig = CFL.addStructure("Node",
    [
        StF(0, "Node",   FT.U8),
        StF(1, "ports",  FT.U8),
        StF(2, "config", FT.LIST, nested=NodeIOConfig)
    ],
    [[FF(FT.U8, 0), FF(FT.U8, 1), FF(FT.LIST | FT.NESTED, 2, lengthField=1, nested=NodeIOConfig, version=0)]]
)

UnitConfig = CFL.addStructure("Unit",
    [
        StF(0,"Module",        FT.U8),
        StF(1,"Connections",   FT.U8),
        StF(2,"IO_Nodes",      FT.U8),
        StF(3,"Blocks",        FT.U16),
        StF(8,"PolarityGroup", FT.U16),
        StF(4,"Switches",      FT.U16),
        StF(5,"MSSwitches",    FT.U16),
        StF(6,"Signals",       FT.U16),
        StF(7,"Stations",      FT.U16)
    ],
    [[FF(FT.U8, 0), FF(FT.U8, 1), FF(FT.U8, 2), FF(FT.U16, 3), FF(FT.U16, 4), FF(FT.U16, 5), FF(FT.U16, 6), FF(FT.U8,  7)], 
     [FF(FT.U8, 0), FF(FT.U8, 1), FF(FT.U8, 2), FF(FT.U16, 3), FF(FT.U16, 4), FF(FT.U16, 5), FF(FT.U16, 6), FF(FT.U16, 7)],

     [FF(FT.U8, 0), FF(FT.U8, 1), FF(FT.U8, 2), FF(FT.U16, 3), FF(FT.U16, 8), FF(FT.U16, 4), FF(FT.U16, 5), FF(FT.U16, 6), FF(FT.U16, 7)]]
)

BlockConfig = CFL.addStructure("Block", 
    [
        StF(0,"id",     FT.U8),
        StF(1,"type",   FT.U8),
        # StF(2, "flags", FT.UNION, unionStruct="uint8_t Polarity:2; uint8_t fixedDirection:1; " + \
                                            #   "uint8_t station:1; uint8_t nostop:1; uint8_t turntable:1;", size=FT.U8),

        StF(3,"next",   RailLinkConfig),
        StF(4,"prev",   RailLinkConfig),
        StF(5,"IOdetection", IOConfig),
        StF(6,"IOpolarity",  IOConfig),
        StF(7,"speed",  FT.U8),
        StF(8,"length", FT.U16),
        StF(9,"fl",     FT.U8),

        StF(10,"Polarity",    FT.U8),
        StF(11,"Polarity_IO", FT.LIST | FT.NESTED, nested=IOConfig, fixedSize=2),

        StF(12, "OneWay", FT.BOOL)
    ],
    [[FF(FT.U8, 0), FF(FT.U8, 1), FF(RailLinkConfig, 3, version=0), FF(RailLinkConfig, 4, version=0),
                    FF(FT.U8, 7), FF(FT.U16, 8), FF(FT.U8, 9)],

     [FF(FT.U8, 0), FF(FT.U8, 1), FF(RailLinkConfig, 3, version=0), FF(RailLinkConfig, 4, version=0),
                    FF(IOConfig, 5, version=0), FF(IOConfig, 6, version=0), FF(FT.U8, 7), FF(FT.U16, 8), FF(FT.U8, 9)],

     [FF(FT.U8, 0), FF(FT.U8, 1), FF(RailLinkConfig, 3, version=0), FF(RailLinkConfig, 4, version=0),
                    FF(IOConfig, 5, version=0), FF(FT.U8, 10), FF(FT.LIST | FT.NESTED, 11, nested=IOConfig, fixedSize=2, version=0),
                    FF(FT.U8, 7), FF(FT.U16, 8), FF(FT.U8, 9)]
    ],
    Editable = [
        ConfigEntry([ 1], name="type", textIndex=["S_RailTypes"]),
        ConfigEntry([ 3], name="Next Block"),
        ConfigEntry([ 4], name="Previous Block"),
        ConfigEntry([ 7], name="Maximum Speed"),
        ConfigEntry([ 8], name="Block Length"),
        ConfigEntry([12], name="Oneway"),
        ConfigEntry([ 9], name="Flags"),
        ConfigEntry([ 5], name="IO Dection Input"),
        ConfigEntry([10], name="Block Polarity Type", textIndex=["S_PolarityTypes"]),
        ConfigEntry([11], custom=('  const char * BlockPolarityStrings[2][2] = {{"Out  ", ""}, {"NOR  ", "REV  "}};\n' + \
                                  "  if(data->Polarity < BLOCK_FL_POLARITY_LINKED_BLOCK){\n" + \
                                  "    if(data->Polarity > 1){\n" + \
                                  '      scanBuffer[0] = 0;  sprintf(fieldName, "Block Polarity %.5s", BlockPolarityStrings[data->Polarity - 2][0]);\n' + \
                                  "      configEditor_preview_IOport(fieldPreview, data->Polarity_IO[0]);\n" + \
                                  '      printf("%-20s %-12s | ", fieldName, fieldPreview);\n' + \
                                  '      fgets(scanBuffer, 100, stdin);\n' + \
                                  '      configEditor_scan_IOport(scanBuffer, &data->Polarity_IO[0]);\n' + \
                                  '    }\n\n' + \
                                  '    if(data->Polarity > 2){\n' + \
                                  '      scanBuffer[0] = 0;  sprintf(fieldName, "Block Polarity %.5s", BlockPolarityStrings[data->Polarity - 2][1]);\n' + \
                                  "      configEditor_preview_IOport(fieldPreview, data->Polarity_IO[1]);\n" + \
                                  '      printf("%-20s %-12s | ", fieldName, fieldPreview);\n' + \
                                  '      fgets(scanBuffer, 100, stdin);\n' + \
                                  '      configEditor_scan_IOport(scanBuffer, &data->Polarity_IO[1]);\n' + \
                                  '    }\n  }\n' + \
                                  '  else{\n' + \
                                  '    scanBuffer[0] = 0;  sprintf(fieldName, "Block Polarity Block");' + \
                                  "    configEditor_preview_IOport(fieldPreview, data->Polarity_IO[0]);" + \
                                  '    printf("%-20s %-12s | ", fieldName, fieldPreview);\n' + \
                                  '    fgets(scanBuffer, 100, stdin);\n' + \
                                  '    configEditor_scan_IOport(scanBuffer, &data->Polarity_IO[0]);\n' + \
                                  '  }\n')),
    ]
)

PolarityGroup = CFL.addStructure("PolarityGroup",
    [
        StF(0, "id",        FT.U16),
        StF(1, "type",      FT.U8),
        StF(2, "nr_blocks", FT.U8),
        StF(3, "blocks",    FT.LIST, nested=FT.U8),
        StF(4, "IO", FT.LIST | FT.NESTED, nested=IOConfig, fixedSize=2)
    ],
    [
        [FF(FT.U8, 1), FF(FT.U8, 2), FF(FT.LIST | FT.U8, 3, lengthField=2)],
        [FF(FT.U16, 0), FF(FT.U8, 1), FF(FT.U8, 2), FF(FT.LIST | FT.U8, 3, lengthField=2), FF(FT.LIST | FT.NESTED, 4, nested=IOConfig, fixedSize=2, version=0)]
    ],
    Editable = [
        ConfigEntry([1], name="type", textIndex=["S_PolarityTypes"]),
        ConfigEntry([4], custom=('  const char * BlockPolarityStrings[2][2] = {{"Out  ", ""}, {"NOR  ", "REV  "}};\n' + \
                                 "  if(data->type < BLOCK_FL_POLARITY_LINKED_BLOCK){\n" + \
                                 "    if(data->type > 1){\n" + \
                                 '      scanBuffer[0] = 0;  sprintf(fieldName, "Block Polarity %.5s", BlockPolarityStrings[data->type - 2][0]);\n' + \
                                 "      configEditor_preview_IOport(fieldPreview, data->IO[0]);\n" + \
                                 '      printf("%-20s %-12s | ", fieldName, fieldPreview);\n' + \
                                 '      fgets(scanBuffer, 100, stdin);\n' + \
                                 '      configEditor_scan_IOport(scanBuffer, &data->IO[0]);\n' + \
                                 '    }\n\n' + \
                                 '    if(data->type > 2){\n' + \
                                 '      scanBuffer[0] = 0;  sprintf(fieldName, "Block Polarity %.5s", BlockPolarityStrings[data->type - 2][1]);\n' + \
                                 "      configEditor_preview_IOport(fieldPreview, data->IO[1]);\n" + \
                                 '      printf("%-20s %-12s | ", fieldName, fieldPreview);\n' + \
                                 '      fgets(scanBuffer, 100, stdin);\n' + \
                                 '      configEditor_scan_IOport(scanBuffer, &data->IO[1]);\n' + \
                                 '    }\n  }\n' + \
                                 '  else{\n' + \
                                 '    scanBuffer[0] = 0;  sprintf(fieldName, "Block Polarity Block");\n' + \
                                 "    configEditor_preview_IOport(fieldPreview, data->IO[0]);\n" + \
                                 '    printf("%-20s %-12s | ", fieldName, fieldPreview);\n' + \
                                 '    fgets(scanBuffer, 100, stdin);\n' + \
                                 '    configEditor_scan_IOport(scanBuffer, &data->IO[0]);\n' + \
                                 '  }\n\n')),
        ConfigEntry([2], name="Number of blocks", alloc=(3, 'uint8_t') ),
        ConfigEntry([3], name="Block", listLength=2),
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
                    FF(FT.U8 | FT.BIT, [5, 6], offset=[0, 4], width=[4, 4]), FF(FT.U8, 7), FF(FT.U8, 8), FF(FT.U8, 9),

                    FF(FT.LIST | FT.NESTED, 10, lengthField=5, nested=IOConfig, version=0), FF(FT.LIST | FT.U8, 11, lengthField=5, sizeMultiply=2),
                    FF(FT.LIST | FT.NESTED, 12, lengthField=9, nested=IOConfig, version=0), FF(FT.LIST | FT.U8, 13, lengthField=9, sizeMultiply=2)
    ]],
    Editable = [
        ConfigEntry([1], "Detection Block"),
        ConfigEntry([2], "Approach  Link"),
        ConfigEntry([3], "Straight  Link"),
        ConfigEntry([4], "Diverging Link"),
        
        ConfigEntry([7], "Straight  Speed"),
        ConfigEntry([8], "Diverging Speed"),
        
        ConfigEntry([6], "IO Type"),
        ConfigEntry([5], "Nr IO Ports", alloc=(10, 'struct configStruct_IOport')),
        ConfigEntry([9], "Nr Feedback IO Ports", alloc=(12, 'struct configStruct_IOport')),
        
        ConfigEntry([10], "IO Port", listLength=5),
        ConfigEntry([11], custom='  for(int i = 0; i < (data->IO_length * 2); i++){\n' + \
                                 '    scanBuffer[0] = 0;\n'
                                 '    printf("IO port %2i  %3s      (%2i:%2i %i)    | ", i/2, (i % 2) ? "div" : "str", data->IO_Ports[i/2].Node, data->IO_Ports[i/2].Port, data->IO_Event[i]);\n' + \
                                 '    fgets(scanBuffer, 20, stdin);\n' + \
                                 '    configEditor_scan_uint8_t(scanBuffer, &data->IO_Event[i]);\n' + \
                                 "  }\n"),
        ConfigEntry([12], "FeedBack IO Port", listLength=9, conditional=[9, '>', '0']),
        ConfigEntry([11], custom='  for(int i = 0; i < (data->feedback_len * 2); i++){\n' + \
                                 '    scanBuffer[0] = 0;\n'
                                 '    printf("FB port %2i  %3s      (%2i:%2i %i)    | ", i/2, (i % 2) ? "div" : "str", data->FB_Ports[i/2].Node, data->FB_Ports[i/2].Port, data->FB_Event[i]);\n' + \
                                 '    fgets(scanBuffer, 20, stdin);\n' + \
                                 '    configEditor_scan_uint8_t(scanBuffer, &data->FB_Event[i]);\n' + \
                                 "  }\n", conditional=[9]),
    ]
) 

MSSwitchStateConfig = CFL.addStructure("MSSwitchState",
    [
        StF(0, "sideA",           RailLinkConfig),
        StF(1, "sideB",           RailLinkConfig),
        StF(2, "speed",           FT.U16),
        StF(3, "dir",             FT.U8),
        StF(4, "output_sequence", FT.U8)
    ],
    [[FF(RailLinkConfig, 0, version=0), FF(RailLinkConfig, 1, version=0), FF(FT.U16, 2), FF(FT.U8, 3), FF(FT.U8, 4)]],
    Editable = [
        ConfigEntry([],   name="State %i", extraArgument=["uint8_t i"], printArgument=["i"]),
        ConfigEntry([ 0], name=" - sideA"),
        ConfigEntry([ 1], name=" - sideB"),
        ConfigEntry([ 2], name=" - speed"),
        ConfigEntry([ 3], name=" - Direction"),
        ConfigEntry([ 4], name=" - Output Sequence")
    ]
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
    ]],
    Editable = [
        ConfigEntry([ 1], name="Detection Block"),
        ConfigEntry([ 2], name="Type"),
        ConfigEntry([ 3], name="Number of States", alloc=(5, 'struct configStruct_MSSwitchState')),
        ConfigEntry([ 5], custom=('  for(uint8_t i = 0; i < data->nr_states; i++)\n' + \
                                  '    configEditor_MSSwitchState(&data->states[i], i);\n\n')),

        ConfigEntry([ 4], name="Number of IO Ports", alloc=(6, 'struct configStruct_IOport')),
        ConfigEntry([ 6], name="IO Port", listLength=4)
    ]
)

StationConfig = CFL.addStructure("Station",
    [
        StF(0,"type",      FT.U8),
        StF(1,"nr_blocks", FT.U8),
        StF(2,"name_len",  FT.U8),
        StF(3,"reserved",  FT.U8),
        StF(4,"parent",    FT.U8),
        StF(7,"id",        FT.U8),

        StF(5,"blocks",    FT.LIST, nested=FT.U8),
        StF(6,"name",      FT.LIST, nested=FT.CHAR)
    ],
    [[FF(FT.U8, 0), FF(FT.U8, 1), FF(FT.U8, 2), FF(FT.U8, 3), FF(FT.U16, 4),
      FF(FT.LIST | FT.U8, 5, lengthField=1), FF(FT.LIST | FT.CHAR, 6, lengthField=2)],
     [FF(FT.U8, 0), FF(FT.U8, 1), FF(FT.U8, 2), FF(FT.U8, 3), FF(FT.U8, 4),
      FF(FT.LIST | FT.U8, 5, lengthField=1), FF(FT.LIST | FT.CHAR, 6, lengthField=2)
    ]],
    Editable = [
        ConfigEntry([6], "Name", overrideType="string", listLength=2),
        ConfigEntry([0], "Type"),
        ConfigEntry([4], "Parent"),
        ConfigEntry([1], "Number of Blocks", alloc=(5, "uint8_t")),

        ConfigEntry([5], "Blocks", listLength=1),
    ]
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
    [[FF(FT.LIST | FT.U8, 0)]],
    Editable = [
        ConfigEntry([0], custom=('  for(uint8_t k = 0; k < 8; k++){' + \
                                 '    scanBuffer[0] = 0;\n' +\
                                 '    sprintf(fieldName, "IO %i State %s", i, S_RailStates[k]);\n' + \
                                 "    configEditor_preview_uint8_t(fieldPreview, data->event[k]);\n" + \
                                 '    printf("%-27s %-5s | ", fieldName, fieldPreview);\n' + \
                                 '    fgets(scanBuffer, 100, stdin);\n' + \
                                 '    configEditor_scan_uint8_t(scanBuffer, &data->event[k]);\n' + \
                                 '  }\n'), extraArgument=["uint8_t i"]),
    ]
)

SignalConfig = CFL.addStructure("Signal",
    [
        StF(0,"direction",  FT.U8),
        StF(1,"id",         FT.U16),
        StF(2,"block",      RailLinkConfig),
        StF(8,"tmp_block",  RailLinkConfig),
        StF(3,"output_len", FT.U8),
        StF(4,"Switch_len", FT.U8),

        StF(5,"output",     FT.LIST, nested=IOConfig),
        StF(6,"stating",    FT.LIST, nested=SignalEvent),
        StF(7,"Switches",   FT.LIST, nested=SignalDependentSwitch)
    ],
    [[FF(FT.U8, 0), FF(FT.U16, 1), FF(RailLinkConfig, 2, version=0), FF(FT.U8, 3), FF(FT.U8, 4),
      FF(FT.LIST | FT.NESTED, 5, lengthField=3, nested=IOConfig, version=0), FF(FT.LIST | FT.NESTED, 6, lengthField=3, nested=SignalEvent, version=0),
      FF(FT.LIST | FT.NESTED, 7, lengthField=4, nested=SignalDependentSwitch, version=0)
    ],
    [FF(FT.U8, 0), FF(FT.U16, 1), FF(RailLinkConfig, 2, version=0), FF(RailLinkConfig, 8, version=0), FF(FT.U8, 3), FF(FT.U8, 4),
      FF(FT.LIST | FT.NESTED, 5, lengthField=3, nested=IOConfig, version=0), FF(FT.LIST | FT.NESTED, 6, lengthField=3, nested=SignalEvent, version=0),
      FF(FT.LIST | FT.NESTED, 7, lengthField=4, nested=SignalDependentSwitch, version=0)
    ]],
    Editable = [
        ConfigEntry([ 2], name="Protected Link"),
        ConfigEntry([ 8], name="Link to block"),
        ConfigEntry([ 0], name="Forward Direction", overrideType="bool"),
        ConfigEntry([ 3], name="Number of IO Ports", alloc=[(5, "struct configStruct_IOport"), (6, "struct configStruct_SignalEvent")]),
        ConfigEntry([ 5], name="IO Port", listLength=3),
        ConfigEntry([ 6], custom=('  for(uint8_t i = 0; i < data->output_len; i++)\n' + \
                                  '    configEditor_SignalEvent(&data->stating[i], i);\n\n'))
    ]
)

LayoutConfig = CFL.addStructure("WebLayout",
    [
        StF(0, "LayoutLength", FT.U16),
        StF(1, "Layout",       FT.LIST, nested=FT.CHAR)
    ],
    [[FF(FT.U16, 0), FF(FT.LIST | FT.CHAR, 1, lengthField=0)]]
)

ConnectorConfig = CFL.addStructure("Connector",
    [
        StF(0, "unit", FT.U8),
        StF(1, "connector", FT.U8),
        StF(2, "crossover", FT.U8)
    ],
    [[FF(FT.U8, 0), FF(FT.U8, 1), FF(FT.U8, 2)]]
)

ConnectorSetupConfig = CFL.addStructure("ConnectorSetup",
    [
        StF(0, "unit",        FT.U8),
        StF(1, "connections", FT.U8),
        StF(2, "connectors",  FT.LIST, nested=ConnectorConfig)
    ],
    [[FF(FT.U8, 0), FF(FT.U8, 1), FF(FT.LIST | FT.NESTED, 2, lengthField=1, nested=ConnectorConfig, version=0)]])


CFL.writeToFile()