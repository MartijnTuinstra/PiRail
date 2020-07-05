BIN=bin
SRC=src
LIB=lib
#ARGS=-std=c99 -lpthread -lssl -lcrypto -lwiringPi -lm -g3 $(INCLUDE) -Werror=unused-variable -Wno-packed-bitfield-compat -Wno-unused-parameter -D _DEFAULT_SOURCE
# GCC_ARGS=-std=c99 -lpthread -lssl -lcrypto -lm -g3 -Werror=unused-variable -Wno-packed-bitfield-compat -Wno-unused-parameter -D _DEFAULT_SOURCE

GCC_DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
GCC_INCLUDE = -I $(LIB)
GCC_ERROR_FLAGS = -Werror=unused-variable -Wno-packed-bitfield-compat -Wno-unused-parameter -Wall
GCC_LIBS = -lpthread -lssl -lcrypto -lm
GCC_FLAGS = -D _DEFAULT_SOURCE

GCC = g++ -std=c++14 -g3 $(GCC_INCLUDE) $(GCC_ERROR_FLAGS) $(GCC_LIBS) $(GCC_FLAGS)

BAAN_FILES = baan system logger mem modules config IO algorithm encryption \
             Z21 Z21_msg train submodule com sim pathfinding

BAAN_FILES += rollingstock/train rollingstock/engine rollingstock/car rollingstock/railtrain
BAAN_FILES += switchboard/links switchboard/rail switchboard/switch switchboard/msswitch switchboard/unit switchboard/station switchboard/signals
BAAN_FILES += config/ModuleConfig config/RollingConfig path

BAAN_FILES += websocket websocket_cts websocket_stc websocket_control scheduler

COMTEST_FILES = comtest system logger mem modules config IO algorithm encryption Z21 Z21_msg train submodule com sim pathfinding websocket websocket_cts websocket_stc websocket_control scheduler
COMTEST_FILES += rollingstock/train rollingstock/engine rollingstock/car rollingstock/railtrain

COMTEST_FILES += switchboard/links switchboard/rail switchboard/switch switchboard/msswitch switchboard/unit switchboard/station switchboard/signals
COMTEST_FILES += config/ModuleConfig config/RollingConfig path

CONFIG_READER_FILES = config_reader config logger mem
CONFIG_READER_FILES += config/ModuleConfig config/RollingConfig

BAAN_CONFIGS = 1 2 3 10 20 21 22 23 25 26
TEST_CONFIGS = PATH-1 SB-1.1 SB-1.2 SB-1.3 SB-2.1 SB-3.1 SB-4.1

TEST_STOCK_CONFIGS =

.DEFAULT_GOAL := all

-include $(BIN)/*.d

$(BIN)/%.o: $(SRC)/%.c
	@echo '(  GCC  ) -$@'
	@$(GCC) -c $(SRC)/$*.c -MP -MMD -MT '$@ $(BIN)/$*.d' -o $(BIN)/$*.o
	@$(GCC) -shared -o $(SRC)/$*.c -o $(BIN)/$*.so

$(BIN)/%.o: $(SRC)/%.cpp
	@echo '(  G++  ) -$@'
	@$(GCC) -c $(SRC)/$*.cpp -MP -MMD -MT '$@ $(BIN)/$*.d' -o $(BIN)/$*.o
	@$(GCC) -shared -o $(SRC)/$*.cpp -o $(BIN)/$*.so

.PHONY: all test run debug avr clean cleanall cleanavr update_modules_config update_rolling_config update_configs

all: config_reader baan comtest avr

update_configs: update_modules_config update_rolling_config

update_modules_config: $(addprefix configs/units/,$(addsuffix .bin,$(BAAN_CONFIGS))) $(addprefix test/testconfigs/,$(addsuffix .bin,$(TEST_CONFIGS)))
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

baan: $(addprefix $(BIN)/,$(addsuffix .o, $(BAAN_FILES)))
	@echo '( GCCLD ) $@'
	@$(GCC) -o $@ $^
	@$(GCC) -shared -o $@.so $^

config_reader: $(addprefix $(BIN)/,$(addsuffix .o, $(CONFIG_READER_FILES)))
	@echo '( GCCLD ) $@'
	@$(GCC) -o $@ $^

comtest: $(addprefix $(BIN)/,$(addsuffix .o, $(COMTEST_FILES)))
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

cleanall: cleanavr clean
