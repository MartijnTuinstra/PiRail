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

GCC = gcc -std=c99 -g3 $(GCC_INCLUDE) $(GCC_ERROR_FLAGS) $(GCC_LIBS) $(GCC_FLAGS)

BAAN_FILES = baan system logger mem modules config rail signals switch IO algorithm encryption \
             Z21 Z21_msg train submodule com sim pathfinding

BAAN_FILES += websocket websocket_cts websocket_stc websocket_control

COMTEST_FILES = logger mem comtest com

CONFIG_READER_FILES = config_reader config logger mem

.DEFAULT_GOAL := all

-include $(BIN)/*.d

$(BIN)/%.o: $(SRC)/%.c
	@echo ' '-$@
	@$(GCC) -c $(SRC)/$*.c -MP -MMD -MT '$@ $(BIN)/$*.d' -o $(BIN)/$*.o
	@$(GCC) -shared -o $(SRC)/$*.c -o $(BIN)/$*.so

.PHONY: all test run debug

all: config_reader baan comtest

run: all
	./baan

debug: all
	gdb ./baan

test: all
	@+$(MAKE) -sC test

_avr:
	@+$(MAKE) -f avr/Makefile all

baan: $(addprefix $(BIN)/,$(addsuffix .o, $(BAAN_FILES)))
	@echo $@
	$(GCC) -o $@ $^
	@$(GCC) -shared -o $@.so $^

config_reader: $(addprefix $(BIN)/,$(addsuffix .o, $(CONFIG_READER_FILES)))
	@echo $@
	$(GCC) -o $@ $^

comtest: $(addprefix $(BIN)/,$(addsuffix .o, $(CONFIG_READER_FILES)))
	@echo $@
	$(GCC) -o $@ $^ 

clean:
	@echo "CLEAN"
	@rm -f baan
	@rm -f baan.so
	@rm -f config_reader
	@rm -rf bin/*
