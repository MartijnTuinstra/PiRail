from .layoutGenerator import *

versionLookup = [[0,0,0,0,0,0,0]]
CFL = ConfigFileLayout("RollingStructure", versionLookup, includePath="config/", sourcePath="generated/src/config/", headerPath="generated/lib/config/")

TrainHeaderConfig = CFL.addStructure("TrainHeader",
  [
    StF(0,"PersonCatagories", FT.U8),
    StF(1,"CargoCatagories",  FT.U8),
    StF(2,"Engines",          FT.U8),
    StF(3,"Cars",             FT.U8),
    StF(4,"Trains",           FT.U8)
  ],
  [[FF(FT.U8, 0), FF(FT.U8, 1), FF(FT.U8, 2), FF(FT.U8, 3), FF(FT.U8, 4)]]
)

SpeedStepConfig = CFL.addStructure("speed_steps",
  [
    StF(0,"speed", FT.U16),
    StF(1,"step",  FT.U8)
  ],
  [[FF(FT.U16, 0), FF(FT.U8, 1)]]
)

EngineConfig = CFL.addStructure("Engines",
  [
    StF(0,"DCC_ID", FT.U16),
    StF(1,"lenght", FT.U16),
    StF(2,"type",   FT.U8),
    StF(3,"config_steps", FT.U8),
    StF(4,"name_len",     FT.U8),
    StF(5,"img_path_len", FT.U8),
    StF(6,"icon_path_len", FT.U8),
    StF(7,"functions",     FT.LIST, nested=FT.U8, fixedSize=29),

    StF(8,"name",      FT.LIST, nested=FT.CHAR),
    StF(9,"img_path",  FT.LIST, nested=FT.CHAR),
    StF(10,"icon_path", FT.LIST, nested=FT.CHAR),
    StF(11,"speed_steps", FT.LIST, nested=SpeedStepConfig)
  ],
  [[FF(FT.U16, 0), FF(FT.U16, 1), FF(FT.U8, 2), FF(FT.U8, 3), FF(FT.U8, 4), FF(FT.U8, 5), FF(FT.U8, 6),
                   FF(FT.LIST | FT.U8, 7, fixedSize=29), FF(FT.LIST | FT.CHAR, 8, lengthField=4), FF(FT.LIST | FT.CHAR, 9, lengthField=5), FF(FT.LIST | FT.CHAR, 10, lengthField=6), 
                   FF(FT.LIST | FT.NESTED, 11, lengthField=3, nested=SpeedStepConfig, version=0)]]
)

CarConfig = CFL.addStructure("Cars",
  [
    StF(0,"nr",        FT.U16),
    StF(1,"max_speed", FT.U16),
    StF(2,"lenght",    FT.U16),
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

TrainCompConfig = CFL.addStructure("TrainComp",
  [
    StF(0,"type", FT.U8),
    StF(1,"id",   FT.U16)
  ],
  [[FF(FT.U8, 0), FF(FT.U16, 1)]]
)

TrainConfig = CFL.addStructure("Train",
  [
    StF(0,"name_len", FT.U8),
    StF(1,"nr_stock", FT.U8),
    StF(2,"category", FT.U8),

    StF(3,"name",      FT.LIST, nested=FT.CHAR),
    StF(4,"icon_path", FT.LIST, nested=FT.CHAR),
  ],
  [[FF(FT.U8, 0), FF(FT.U8, 1), FF(FT.U8, 2), FF(FT.LIST | FT.CHAR, 3, lengthField=0), FF(FT.LIST | FT.CHAR, 4, lengthField=1)]]
)

CategoryConfig = CFL.addStructure("Category",
  [
    StF(0,"name_len", FT.U8),

    StF(1,"name",      FT.LIST, nested=FT.CHAR),
  ],
  [[FF(FT.U8, 0), FF(FT.LIST | FT.CHAR, 1, lengthField=0)]]
)

CFL.writeToFile()
