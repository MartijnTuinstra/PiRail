from .layoutGenerator import *

versionLookup = [[0,0,0,0,0,0,0],
                 [1,0,0,0,0,0,0],
                 [1,0,1,0,0,0,0]]

CFL = ConfigFileLayout("RollingStructure", versionLookup, includePath="config/", sourcePath="generated/src/config/", headerPath="generated/lib/config/")

CFL.addStructure("RollingStockHeader",
  [
    StF(0,"PersonCatagories", FT.U8),
    StF(1,"CargoCatagories",  FT.U8),
    StF(2,"Engines",          FT.U16),
    StF(3,"Cars",             FT.U16),
    StF(4,"TrainSets",        FT.U16)
  ],
  [[FF(FT.U8, 0), FF(FT.U8, 1), FF(FT.U8,  2), FF(FT.U8,  3), FF(FT.U8,  4)],
   [FF(FT.U8, 0), FF(FT.U8, 1), FF(FT.U16, 2), FF(FT.U16, 3), FF(FT.U16, 4)]]
)

SpeedStepConfig = CFL.addStructure("EngineSpeedSteps",
  [
    StF(0,"speed", FT.U16),
    StF(1,"step",  FT.U8)
  ],
  [[FF(FT.U16, 0), FF(FT.U8, 1)]]
)

CFL.addStructure("Engine",
  [
    StF(0,"DCC_ID", FT.U16),
    StF(1,"length", FT.U16),
    StF(2,"type",   FT.U8),
    StF(3, "Z21_SpeedSteps", FT.U8, default=2),
    StF(4,"config_steps", FT.U8),
    StF(5,"name_len",     FT.U8),
    StF(6,"img_path_len", FT.U8),
    StF(7,"icon_path_len", FT.U8),
    StF(8,"functions",     FT.LIST, nested=FT.U8, fixedSize=29),

    StF(9,"name",      FT.LIST, nested=FT.CHAR),
    StF(10,"img_path",  FT.LIST, nested=FT.CHAR),
    StF(11,"icon_path", FT.LIST, nested=FT.CHAR),
    StF(12,"speed_steps", FT.LIST, nested=SpeedStepConfig)
  ],
  [[FF(FT.U16, 0), FF(FT.U16, 1), FF(FT.U8, 2), FF(FT.U8, 4), FF(FT.U8, 5), FF(FT.U8, 6), FF(FT.U8, 7),
                   FF(FT.LIST | FT.U8, 8, fixedSize=29), FF(FT.LIST | FT.CHAR, 9, lengthField=5), FF(FT.LIST | FT.CHAR, 10, lengthField=6), FF(FT.LIST | FT.CHAR, 11, lengthField=7), 
                   FF(FT.LIST | FT.NESTED, 12, lengthField=4, nested=SpeedStepConfig, version=0)],

   [FF(FT.U16, 0), FF(FT.U16, 1), FF(FT.U8, 2), FF(FT.U8 | FT.BIT, [3], offset=[0], width=[2]), FF(FT.U8, 4), FF(FT.U8, 5), FF(FT.U8, 6), FF(FT.U8, 7),
                   FF(FT.LIST | FT.U8, 8, fixedSize=29), FF(FT.LIST | FT.CHAR, 9, lengthField=5), FF(FT.LIST | FT.CHAR, 10, lengthField=6), FF(FT.LIST | FT.CHAR, 11, lengthField=7), 
                   FF(FT.LIST | FT.NESTED, 12, lengthField=4, nested=SpeedStepConfig, version=0)]
  ]
)

CFL.addStructure("Car",
  [
    StF(0,"nr",        FT.U16),
    StF(1,"max_speed", FT.U16),
    StF(2,"length",    FT.U16),
    StF(3,"flags",     FT.U8),
    StF(4,"type",      FT.U8),
    StF(5,"name_len",  FT.U8),
    StF(6,"icon_path_len", FT.U8),
    StF(7,"functions",     FT.LIST, nested=FT.U8, fixedSize=29),

    StF(8,"name",      FT.LIST, nested=FT.CHAR),
    StF(9,"icon_path", FT.LIST, nested=FT.CHAR),
  ],
  [[FF(FT.U16, 0), FF(FT.U16, 1), FF(FT.U16, 2), FF(FT.U8, 3), FF(FT.U8, 4), FF(FT.U8, 5), FF(FT.U8, 6),
                   FF(FT.LIST | FT.U8, 7, fixedSize=29), FF(FT.LIST | FT.CHAR, 8, lengthField=5), FF(FT.LIST | FT.CHAR, 9, lengthField=6)]]
)

TrainSetLinkConfig = CFL.addStructure("TrainSetLink",
  [
    StF(0,"type", FT.U8),
    StF(1,"id",   FT.U16)
  ],
  [[FF(FT.U8, 0), FF(FT.U16, 1)]]
)

CFL.addStructure("TrainSet",
  [
    StF(0,"name_len", FT.U8),
    StF(1,"nr_stock", FT.U8),
    StF(2,"category", FT.U8),

    StF(3,"name",        FT.LIST, nested=FT.CHAR),
    StF(4,"composition", FT.LIST, nested=TrainSetLinkConfig),
  ],
  [[FF(FT.U8, 0), FF(FT.U8, 1), FF(FT.U8, 2), FF(FT.LIST | FT.CHAR, 3, lengthField=0),
                  FF(FT.LIST | FT.NESTED, 4, lengthField=1, nested=TrainSetLinkConfig, version=0)]]
)

CFL.addStructure("Category",
  [
    StF(0,"name_len", FT.U8),

    StF(1,"name",      FT.LIST, nested=FT.CHAR),
  ],
  [[FF(FT.U8, 0), FF(FT.LIST | FT.CHAR, 1, lengthField=0)]]
)

CFL.writeToFile()
