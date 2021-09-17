BIN=bin
SRC=src
LIB=lib
SHARED_LIB=shared/lib

GCC_DEPFLAGS = -MT $@ -MMD -MF $(DEPDIR)/$*.d
GCC_INCLUDE = -I $(LIB) -I generated/$(LIB) -I $(SHARED_LIB)
GCC_ERROR_FLAGS = -Werror=unused-variable -Wno-packed-bitfield-compat -Wno-unused-parameter #-Wall
GCC_LIBS = -pthread -lssl -lcrypto -lm
GCC_FLAGS = -D _DEFAULT_SOURCE -D _POSIX_C_SOURCE=600 -D CB_NON_AVR

GCC = g++ -std=c++14 -g3 $(GCC_INCLUDE)
GCC_options = $(GCC_ERROR_FLAGS) $(GCC_FLAGS)

GENERATED = config/LayoutStructure config/RollingStructure
GENERATED_OBJS =  $(addprefix $(BIN)/,$(addsuffix .o,$(GENERATED)))
GENERATED_FILES = $(addprefix generated/lib/,$(GENERATED))
GENERATOR = layoutGenerator.py

SHARED_FILES = circularbuffer
SHARED_OBJ = $(addprefix $(BIN)/shared/,$(addsuffix .o,$(SHARED_FILES)))

FILES_CONFIG = $(addprefix config/,LayoutStructure RollingStructure ModuleConfig RollingConfig configReader)
FILES_SWITCHBOARD = $(addprefix switchboard/,blockconnector links rail switch switchsolver msswitch polaritysolver unit station signals manager)
FILES_WEBSOCKET = $(addprefix websocket/,server client stc cts message)
FILES_ROLLING = $(addprefix rollingstock/,manager trainset engine car traindetection train/general train/route train/speed)
FILES_UTILS = $(addprefix utils/,logger mem encryption utils)
FILES_ALGORITHM = $(addprefix algorithm/,core component queue blockconnector traincontrol)
FILES_UART = $(addprefix uart/,uart RNetRX RNetTX)

BAAN_FILES = $(FILES_CONFIG) $(FILES_ROLLING) $(FILES_WEBSOCKET) \
             $(FILES_SWITCHBOARD) $(FILES_ALGORITHM) $(FILES_UTILS) \
             baan system IO \
             Z21 Z21_msg train submodule uart/uart uart/RNetTX uart/RNetRX sim path pathfinding scheduler/scheduler

BAAN_OBJS  = $(addprefix $(BIN)/,$(addsuffix .o, $(BAAN_FILES))) $(SHARED_OBJ) $(GENERATED_OBJS)

COMTEST_FILES = comtest system IO Z21 Z21_msg train submodule $(FILES_UART) sim path pathfinding scheduler/scheduler \
                $(FILES_ROLLING) $(FILES_WEBSOCKET) $(FILES_SWITCHBOARD) $(FILES_CONFIG) $(FILES_ALGORITHM) $(FILES_UTILS)
COMTEST_OBJS  = $(addprefix $(BIN)/,$(addsuffix .o, $(COMTEST_FILES))) $(SHARED_OBJ)

CONFIG_READER_FILES = $(FILES_CONFIG) $(FILES_UTILS) $(FILES_UTILS) \
                      config_reader uart/uart uart/RNetTX uart/RNetRX
CONFIG_OBJS         = $(addprefix $(BIN)/,$(addsuffix .o, $(CONFIG_READER_FILES))) $(GENERATED_OBJS) $(SHARED_OBJ)

BAAN_CONFIGS = 1 2 3 4 10 20 21 22 23 25 26
TEST_CONFIGS  = Alg-1-1 Alg-1-2 Alg-1-3 Alg-1-4 Alg-2 Alg-R-1 Alg-R-2 Alg-R-3 Alg-R-4 Alg-Sp
TEST_CONFIGS += PS-1 SwS-1
TEST_CONFIGS += IO-1 IO-3 PATH-1 PATH-2 PF-1 PF-2 RT-Z21
TEST_CONFIGS += SB-1.1 SB-1.2 SB-1.3 SB-1.4 SB-1.5 SB-2.1 SB-3.1 SB-4.1 SB-4.2-1 SB-4.2-2

TEST_STOCK_CONFIGS =

.DEFAULT_GOAL := all

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

-include $(call rwildcard,$(BIN),*.d)

$(BIN)/shared/%.o: shared/src/%.c
	@echo '(  GCC  ) - $@ $<'
	@$(GCC) -c shared/src/$*.c -MP -MMD -MT '$@ $(BIN)/shared/$*.d' -o $(BIN)/shared/$*.o $(GCC_options)

$(BIN)/shared/%.o: shared/src/%.cpp
	@echo '(  G++  ) - $@ $<'
	@$(GCC) -c shared/src/$*.cpp -MP -MMD -MT '$@ $(BIN)/shared/$*.d' -o $(BIN)/shared/$*.o $(GCC_options)

$(BIN)/%.o: generated/$(SRC)/%.c
	@echo '(  GCC  ) -$@'
	@$(GCC) -c generated/$(SRC)/$*.c -MP -MMD -MT '$@ $(BIN)/$*.d' -o $(BIN)/$*.o $(GCC_options)

$(BIN)/%.o: $(SRC)/%.c
	@echo '(  GCC  ) -$@'
	@$(GCC) -c $(SRC)/$*.c -MP -MMD -MT '$@ $(BIN)/$*.d' -o $(BIN)/$*.o $(GCC_options)

$(BIN)/%.o: generated/$(SRC)/%.cpp
	@echo '(  G++  ) -$@'
	@$(GCC) -c generated/$(SRC)/$*.cpp -MP -MMD -MT '$@ $(BIN)/$*.d' -o $(BIN)/$*.o $(GCC_options)

$(BIN)/%.o: $(SRC)/%.cpp
	@echo '(  G++  ) -$@'
	@$(GCC) -c $(SRC)/$*.cpp -MP -MMD -MT '$@ $(BIN)/$*.d' -o $(BIN)/$*.o $(GCC_options)


.PHONY: all test run debug avr clean cleanall cleanavr update_modules_config update_rolling_config update_configs test runtest

all: config_reader baan comtest test avr

update_configs: update_modules_config update_rolling_config

# Generate files from higher level languages

generated/lib/config/%.h: $(addprefix generated/, %.py $(GENERATOR))
	@echo '(  PY   ) -$@'
	python3 -m generated.$*

# Update Configs to new version

%.bin_bu_MKFL: %.bin
	cp $^ $@

update_modules_config: $(addprefix configs/units/,$(addsuffix .bin_bu_MKFL,$(BAAN_CONFIGS))) $(addprefix test/testconfigs/,$(addsuffix .bin_bu_MKFL,$(TEST_CONFIGS)))
	./config_reader --update --module $(addprefix configs/units/,$(addsuffix .bin,$(BAAN_CONFIGS))) $(addprefix test/testconfigs/,$(addsuffix .bin,$(TEST_CONFIGS)))

update_rolling_config: configs/stock.bin $(addprefix test/testconfigs/,$(addsuffix .bin,$(TEST_STOCK_CONFIGS)))
	./config_reader --update --rollingediting $^

# Basic commands

run: all
	./baan

debug: all
	gdb ./baan

avr:
	@+$(MAKE) -C avr all --no-print-directory

# Build commands

$(BAAN_OBJS): $(addsuffix .h,$(GENERATED_FILES))
baan: $(BAAN_OBJS)
	@echo '( GCCLD ) $@'
	@echo $^
	@$(GCC) -o $@ $^ -D CB_NON_AVR $(GCC_options) $(GCC_LIBS)

config_reader: $(CONFIG_OBJS)
	@echo '( GCCLD ) $@'
	@$(GCC) -o $@ $^ -D CB_NON_AVR $(GCC_options) $(GCC_LIBS)

comtest: $(COMTEST_OBJS)
	@echo '( GCCLD ) $@'
	@$(GCC) -o $@ $^ -D CB_NON_AVR $(GCC_options) $(GCC_LIBS)

test: $(BAAN_OBJS)
	@echo '( BUILD ) maintest'
	@$(MAKE) -sC test build

runtest: test
	@echo '( TEST  ) maintest'
	@$(MAKE) -sC test run

# Cleaning project

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

	@$(MAKE) -sC test clean

cleanall: cleanavr clean
