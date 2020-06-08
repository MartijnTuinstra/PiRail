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

BAAN_FILES = baan system logger mem modules config rail signals switch IO algorithm encryption \
             Z21 Z21_msg train submodule com sim pathfinding

BAAN_FILES += websocket websocket_cts websocket_stc websocket_control scheduler

COMTEST_FILES = comtest system logger mem modules config rail signals switch IO algorithm encryption Z21 Z21_msg train submodule com sim pathfinding websocket websocket_cts websocket_stc websocket_control scheduler

CONFIG_READER_FILES = config_reader config logger mem

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

.PHONY: all test run debug avr

all: config_reader baan comtest avr

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

clean:
	@echo "CLEAN"
	@rm -f baan
	@rm -f baan.so
	@rm -f config_reader
	@rm -f comtest
	@rm -rf bin/*
	@+$(MAKE) -C avr clean --no-print-directory
