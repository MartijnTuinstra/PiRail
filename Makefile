BIN=./bin
SRC=./src
LIB=./lib
INCLUDE = -I $(LIB) -I $(SRC)
#ARGS=-std=c99 -lpthread -lssl -lcrypto -lwiringPi -lm -g3 $(INCLUDE) -Werror=unused-variable -Wno-packed-bitfield-compat -Wno-unused-parameter -D _DEFAULT_SOURCE
GCC_ARGS=-std=c99 -lpthread -lssl -lcrypto -lm -g3 -Werror=unused-variable -Wno-packed-bitfield-compat -Wno-unused-parameter -D _DEFAULT_SOURCE

GCC = gcc $(INCLUDE)
GCC_SIMPLE = gcc -g -Wall -W -Werror=unused-variable $(INCLUDE) -std=c99

ifndef VERBOSE
.SILENT:
endif

.PHONY: all

all: config_reader baan
#all: baan

$(BIN)/%.o: $(SRC)/%.c
	@echo $@
	@$(GCC) -c -o $@ $^ $(GCC_ARGS)

config_reader: $(BIN)/config_reader.o $(BIN)/config.o $(BIN)/logger.o $(BIN)/mem.o
	@echo config_reader
	@$(GCC_SIMPLE) -o $@ $(BIN)/config_reader.o $(BIN)/config.o $(BIN)/logger.o $(BIN)/mem.o

$(SRC)/config_reader.c: $(LIB)/config.h $(LIB)/logger.h

$(LIB)/config.h: $(LIB)/config_data.h
$(SRC)/config.c: $(LIB)/config.h $(LIB)/logger.h

# baan: $(BIN)/baan.o $(BIN)/logger.o $(BIN)/rail.o $(BIN)/train.o $(BIN)/system.o $(BIN)/websocket_control.o $(BIN)/websocket_msg.o $(BIN)/module.o \
		$(BIN)/train_sim.o $(BIN)/com.o $(BIN)/algorithm.o $(BIN)/signals.o $(BIN)/switch.o $(BIN)/Z21.o $(BIN)/Z21_msg.o $(BIN)/websocket.o $(BIN)/encryption.o $(BIN)/IO.o \
		$(BIN)/config.o $(BIN)/mem.o $(BIN)/submodule.o $(BIN)/pathfinding.o
BAANOBJ = baan system logger mem modules config rail signals switch IO algorithm encryption websocket_control websocket_msg websocket Z21 Z21_msg train submodule com sim
baan: $(addprefix $(BIN)/,$(addsuffix .o,$(BAANOBJ)))
	@echo baan
	$(GCC) -o baan $^ $(GCC_ARGS)

baan.c: $(LIB)/logger.h $(LIB)/system.h $(LIB)/mem.h
$(BIN)/baan.o: baan.c
	@echo baan.o
	$(GCC) baan.c -c -o $(BIN)/baan.o

$(SRC)/mem.c: $(LIB)/mem.h $(LIB)/logger.h

$(LIB)/algorithm.h: $(LIB)/rail.h
$(SRC)/algorithm.c: $(LIB)/algorithm.h $(LIB)/system.h $(LIB)/mem.h \
		$(LIB)/logger.h $(LIB)/switch.h $(LIB)/signals.h \
		$(LIB)/modules.h #$(LIB)/train.h $(LIB)/com.h $(LIB)/websocket_msg.h $(LIB)/submodules.h

$(LIB)/com.h: $(LIB)/signals.h
$(SRC)/com.c: $(LIB)/com.h $(LIB)/system.h $(LIB)/rail.h \
		$(LIB)/switch.h $(LIB)/signals.h $(LIB)/train.h $(LIB)/logger.h \
		$(LIB)/modules.h $(LIB)/submodule.h

$(SRC)/encryption.c: $(LIB)/encryption.h

$(SRC)/logger.c: $(LIB)/logger.h $(LIB)/mem.h

# $(LIB)/modules.h: $(LIB)/rail.h $(LIB)/switch.h $(LIB)/signals.h
$(SRC)/modules.c: $(LIB)/modules.h

# $(LIB)/pathfinding.h: $(LIB)/rail.h
# $(SRC)/pathfinding.c: $(LIB)/pathfinding.h $(LIB)/logger.h $(LIB)/switch.h

$(LIB)/rail.h: $(LIB)/config_data.h
$(SRC)/rail.c: $(LIB)/rail.h $(LIB)/system.h $(LIB)/modules.h $(LIB)/switch.h $(LIB)/logger.h # $(LIB)/algorithm.h

# $(LIB)/route.h: $(LIB)/rail.h #$(LIB)/switch.h

$(LIB)/signals.h: $(LIB)/rail.h
$(SRC)/signals.c: $(LIB)/signals.h $(LIB)/system.h $(LIB)/mem.h $(LIB)/config_data.h $(LIB)/modules.h $(LIB)/logger.h

$(LIB)/switch.h: $(LIB)/rail.h #$(LIB)/train.h
$(SRC)/switch.c: $(LIB)/switch.h $(LIB)/logger.h

$(SRC)/submodule.c: $(LIB)/algorithm.h $(LIB)/Z21.h $(LIB)/logger.h # $(LIB)/com.h $(LIB)/train_sim.h 

$(SRC)/system.c: $(LIB)/system.h $(LIB)/logger.h $(LIB)/mem.h

$(LIB)/train.h: $(LIB)/rail.h #$(LIB)/route.h
$(SRC)/train.c: $(LIB)/train.h $(LIB)/system.h $(LIB)/logger.h $(LIB)/switch.h# $(LIB)/pathfinding.h

# $(SRC)/train_sim.c: $(LIB)/train_sim.h $(LIB)/rail.h $(LIB)/train.h $(LIB)/system.h $(LIB)/modules.h $(LIB)/submodules.h

$(SRC)/websocket.c: $(LIB)/websocket.h

$(LIB)/websocket_control.h: $(LIB)/websocket.h $(LIB)/websocket_msg.h $(LIB)/modules.h
$(SRC)/websocket_control.c: $(LIB)/websocket_control.h

$(LIB)/websocket_msg.h: $(LIB)/websocket.h $(LIB)/train.h
$(SRC)/websocket_msg.c: $(LIB)/websocket_msg.h $(LIB)/system.h $(LIB)/rail.h $(LIB)/switch.h \
	                    $(LIB)/train.h $(LIB)/logger.h $(LIB)/modules.h $(LIB)/Z21.h

# $(SRC)/Z21.c: $(LIB)/Z21.h $(LIB)/logger.h $(LIB)/submodules.h $(LIB)/Z21_msg.h

# $(SRC)/Z21_msg.c: $(LIB)/Z21.h $(LIB)/Z21_msg.h $(LIB)/train.h

$(LIB)/IO.h: $(LIB)/modules.h
$(SRC)/IO.c: $(LIB)/IO.h

.PHONY: clean

clean:
	@echo "CLEAN"
	@rm -f baan
	@rm bin/*
