BIN=bin
SRC=src
LIB=lib
SHARED_LIB=shared/lib
#ARGS=-std=c99 -lpthread -lssl -lcrypto -lwiringPi -lm -g3 $(INCLUDE) -Werror=unused-variable -Wno-packed-bitfield-compat -Wno-unused-parameter -D _DEFAULT_SOURCE
# GCC_ARGS=-std=c99 -lpthread -lssl -lcrypto -lm -g3 -Werror=unused-variable -Wno-packed-bitfield-compat -Wno-unused-parameter -D _DEFAULT_SOURCE

GCC_DEPFLAGS = -MT $@ -MMD -MF $(DEPDIR)/$*.d
GCC_INCLUDE = -I $(LIB) -I generated/$(LIB) -I $(SHARED_LIB)
GCC_ERROR_FLAGS = -Werror=unused-variable -Wno-packed-bitfield-compat -Wno-unused-parameter -Wall
GCC_LIBS = -pthread -lssl -lcrypto -lm
GCC_FLAGS = -D _DEFAULT_SOURCE -D _POSIX_C_SOURCE=600

GCC = g++ -std=c++14 -g3 $(GCC_INCLUDE) $(GCC_ERROR_FLAGS) $(GCC_LIBS) $(GCC_FLAGS)



FILES_CONFIG = $(addprefix config/,LayoutStructure RollingStructure ModuleConfig RollingConfig configReader)
FILES_SWITCHBOARD = $(addprefix switchboard/,blockconnector links rail switch msswitch unit station signals manager)
FILES_WEBSOCKET = $(addprefix websocket/,server client stc cts message)
FILES_ROLLING = $(addprefix rollingstock/,manager train engine car railtrain)
FILES_UTILS = $(addprefix utils/,logger mem encryption utils)
FILES_ALGORITHM = $(addprefix algorithm/,core component queue blockconnector traincontrol)

BAAN_FILES = $(FILES_CONFIG) $(FILES_ROLLING) $(FILES_WEBSOCKET) \
             $(FILES_SWITCHBOARD) $(FILES_ALGORITHM) $(FILES_UTILS) \
             baan system IO \
             Z21 Z21_msg train submodule com sim path pathfinding scheduler/scheduler

#BAAN_FILES += websocket websocket_cts websocket_stc websocket_control

COMTEST_FILES = comtest system IO Z21 Z21_msg train submodule com sim path pathfinding scheduler/scheduler
COMTEST_FILES += $(FILES_ROLLING) $(FILES_WEBSOCKET) $(FILES_SWITCHBOARD) $(FILES_CONFIG) $(FILES_ALGORITHM) $(FILES_UTILS)

CONFIG_READER_FILES = config_reader $(FILES_CONFIG) $(FILES_UTILS)

BAAN_CONFIGS = 1 2 3 4 10 20 21 22 23 25 26
TEST_CONFIGS = PATH-1 PATH-2 IO-1 IO-3 SB-1.1 SB-1.2 SB-1.3 SB-2.1 SB-3.1 SB-4.1 SB-4.2-1 SB-4.2-2 Alg-1-1 Alg-1-2 Alg-1-3 Alg-1-4 Alg-2 Alg-3 PF-1

SHARED_OBJ_FILES = circularbuffer

TEST_STOCK_CONFIGS =

.DEFAULT_GOAL := all

-include $(BIN)/*.d

$(BIN)/shared/%.o: shared/src/%.c
	@echo '(  GCC  ) - $@ $<'
	@$(GCC) -c shared/src/$*.c -MP -MMD -MT '$@ $(BIN)/shared/$*.d' -o $(BIN)/shared/$*.o
	@$(GCC) -shared -o shared/src/$*.c -o $(BIN)/shared/$*.so

$(BIN)/shared/%.o: shared/src/%.cpp
	@echo '(  G++  ) - $@ $<'
	@$(GCC) -c shared/src/$*.cpp -MP -MMD -MT '$@ $(BIN)/shared/$*.d' -o $(BIN)/shared/$*.o
	@$(GCC) -shared -o shared/src/$*.cpp -o $(BIN)/shared/$*.so

$(BIN)/%.o: generated/$(SRC)/%.c
	@echo '(  GCC  ) -$@'
	@$(GCC) -c generated/$(SRC)/$*.c -MP -MMD -MT '$@ $(BIN)/$*.d' -o $(BIN)/$*.o
	@$(GCC) -shared -o $(SRC)/$*.c -o $(BIN)/$*.so
$(BIN)/%.o: $(SRC)/%.c
	@echo '(  GCC  ) -$@'
	@$(GCC) -c $(SRC)/$*.c -MP -MMD -MT '$@ $(BIN)/$*.d' -o $(BIN)/$*.o
	@$(GCC) -shared -o $(SRC)/$*.c -o $(BIN)/$*.so

$(BIN)/%.o: generated/$(SRC)/%.cpp
	@echo '(  G++  ) -$@'
	@$(GCC) -c generated/$(SRC)/$*.cpp -MP -MMD -MT '$@ $(BIN)/$*.d' -o $(BIN)/$*.o
	@$(GCC) -shared -o $(SRC)/$*.cpp -o $(BIN)/$*.so
$(BIN)/%.o: $(SRC)/%.cpp
	@echo '(  G++  ) -$@'
	@$(GCC) -c $(SRC)/$*.cpp -MP -MMD -MT '$@ $(BIN)/$*.d' -o $(BIN)/$*.o
	@$(GCC) -shared -o $(SRC)/$*.cpp -o $(BIN)/$*.so

.PHONY: all test run debug avr clean cleanall cleanavr update_modules_config update_rolling_config update_configs

all: config_reader baan comtest avr

update_configs: update_modules_config update_rolling_config

%.bin_bu_MKFL: %.bin
	cp $^ $@

generated/src/config/LayoutStructure.cpp generated/lib/config/LayoutStructure.h: generated/LayoutStructures.py generated/layoutGenerator.py
	@echo '(  PY   ) -$@'
	python3 -m generated.LayoutStructures

generated/src/config/RollingStructure.cpp generated/lib/config/RollingStructure.h: generated/RollingStructures.py generated/layoutGenerator.py
	@echo '(  PY   ) -$@'
	python3 -m generated.RollingStructures

generateConfigStructures: generated/lib/config/LayoutStructure.h generated/src/config/LayoutStructure.cpp generated/lib/config/RollingStructure.h generated/src/config/RollingStructure.cpp

update_modules_config: $(addprefix configs/units/,$(addsuffix .bin_bu_MKFL,$(BAAN_CONFIGS))) $(addprefix test/testconfigs/,$(addsuffix .bin_bu_MKFL,$(TEST_CONFIGS)))
	./config_reader --update --module $^

update_rolling_config: configs/stock.bin $(addprefix test/testconfigs/,$(addsuffix .bin,$(TEST_STOCK_CONFIGS)))
	./config_reader --update --rollingediting $^

run: all
	./baan

debug: all
	gdb ./baan

test: all
	@+$(MAKE) -sC test

avr:
	@+$(MAKE) -C avr all --no-print-directory

baan: $(addprefix $(BIN)/,$(addsuffix .o, $(BAAN_FILES))) $(addprefix $(BIN)/shared/,$(addsuffix .o, $(SHARED_OBJ_FILES)))
	@echo '( GCCLD ) $@'
	@echo $^
	@$(GCC) -o $@ $^ -D CB_NON_AVR
	@$(GCC) -shared -o $@.so $(addprefix $(BIN)/,$(addsuffix .o, $(BAAN_FILES))) $(addprefix $(BIN)/shared/,$(addsuffix .o, $(SHARED_OBJ_FILES))) -D CB_NON_AVR

config_reader: $(addprefix $(BIN)/,$(addsuffix .o, $(CONFIG_READER_FILES)))
	@echo '( GCCLD ) $@'
	@$(GCC) -o $@ $^

comtest: $(addprefix $(BIN)/,$(addsuffix .o, $(COMTEST_FILES))) $(addprefix $(BIN)/shared/,$(addsuffix .o, $(SHARED_OBJ_FILES)))
	@echo '( GCCLD ) $@'
	@$(GCC) -o $@ $^ 

cleanavr:
	@echo "CLEAN AVR"
	@+$(MAKE) -C avr clean --no-print-directory

clean:
	@echo "CLEAN"
	@rm -f baan
	@rm -f baan.so
	@rm -f config_reader
	@rm -f comtest
	@find bin -type f -name "*.o" -delete
	@find bin -type f -name "*.so" -delete
	@find bin -type f -name "*.d" -delete

	@find generated/lib generated/src -type f -delete

cleanall: cleanavr clean
