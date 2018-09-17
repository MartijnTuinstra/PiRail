BIN=./bin
SRC=./src
LIB=./lib
INCLUDE = -I $(LIB) -I $(SRC)
ARGS=-std=c99 -lpthread -lssl -lcrypto -lwiringPi -lm -g $(INCLUDE) -Wall -W -Werror=unused-variable

GCC = gcc $(ARGS)
GCC_SIMPLE = gcc -g -Wall -W -Werror=unused-variable $(INCLUDE) -std=c99

ifndef VERBOSE
.SILENT:
endif
.PHONY: all

all: config_reader baan

config_reader: $(BIN)/config_reader.o $(BIN)/config.o $(BIN)/logger.o
	@echo config_reader
	@$(GCC_SIMPLE) -o $@ $(BIN)/config_reader.o $(BIN)/config.o $(BIN)/logger.o

$(BIN)/config_reader.o: $(SRC)/config_reader.c $(LIB)/config.h $(LIB)/logger.h
	@echo config_reader.o
	@$(GCC_SIMPLE) -c -o $@ $(SRC)/config_reader.c 

$(BIN)/config.o: $(SRC)/config.c $(LIB)/config.h $(LIB)/logger.h
	@echo config.c
	$(GCC) $(SRC)/config.c -c -o $@

baan: $(BIN)/baan.o $(BIN)/logger.o $(BIN)/rail.o $(BIN)/train.o $(BIN)/system.o $(BIN)/websocket_control.o $(BIN)/websocket_msg.o $(BIN)/module.o \
		$(BIN)/train_sim.o $(BIN)/com.o $(BIN)/algorithm.o $(BIN)/signals.o $(BIN)/switch.o $(BIN)/Z21.o $(BIN)/websocket.o $(BIN)/encryption.o $(BIN)/IO.o $(BIN)/config.o
	@echo baan
	$(GCC) -o baan $(BIN)/baan.o $(BIN)/logger.o $(BIN)/rail.o $(BIN)/train.o $(BIN)/system.o $(BIN)/websocket_control.o $(BIN)/websocket_msg.o $(BIN)/module.o \
	$(BIN)/train_sim.o $(BIN)/com.o $(BIN)/algorithm.o $(BIN)/signals.o $(BIN)/switch.o $(BIN)/Z21.o $(BIN)/websocket.o $(BIN)/encryption.o $(BIN)/IO.o $(BIN)/config.o

$(BIN)/baan.o: baan.c $(LIB)/logger.h $(LIB)/train.h $(LIB)/rail.h $(LIB)/switch.h $(LIB)/com.h $(LIB)/websocket_control.h $(LIB)/system.h
	@echo baan.o
	$(GCC) baan.c -c -o $(BIN)/baan.o

$(LIB)/algorithm.h: $(LIB)/rail.h
$(BIN)/algorithm.o: $(SRC)/algorithm.c $(LIB)/algorithm.h $(LIB)/system.h \
		$(LIB)/logger.h $(LIB)/train.h $(LIB)/switch.h $(LIB)/signals.h \
		$(LIB)/module.h $(LIB)/com.h $(LIB)/websocket_msg.h
	@echo algorithm.o
	$(GCC) $(SRC)/algorithm.c -c -o $(BIN)/algorithm.o

$(LIB)/com.h: $(LIB)/signals.h
$(BIN)/com.o: $(SRC)/com.c $(LIB)/com.h $(LIB)/system.h $(LIB)/rail.h \
		$(LIB)/switch.h $(LIB)/signals.h $(LIB)/train.h $(LIB)/logger.h \
		$(LIB)/module.h
	@echo com.c
	$(GCC) $(SRC)/com.c -c -o $(BIN)/com.o

$(BIN)/encryption.o: $(SRC)/encryption.c $(LIB)/encryption.h
	@echo encryption.o
	$(GCC) $(SRC)/encryption.c -c -o $(BIN)/encryption.o

$(BIN)/logger.o: $(SRC)/logger.c $(LIB)/logger.h
	@echo logger.o
	$(GCC) $(SRC)/logger.c -c -o $(BIN)/logger.o

$(LIB)/module.h: $(LIB)/rail.h $(LIB)/switch.h $(LIB)/signals.h
$(BIN)/module.o: $(SRC)/module.c $(LIB)/module.h $(LIB)/system.h \
		$(LIB)/logger.h $(LIB)/train.h $(LIB)/algorithm.h \
		$(LIB)/websocket_msg.h $(LIB)/websocket_control.h
	@echo module.c
	$(GCC) $(SRC)/module.c -c -o $(BIN)/module.o

$(BIN)/rail.o: $(SRC)/rail.c $(LIB)/rail.h $(LIB)/system.h $(LIB)/module.h \
		$(LIB)/switch.h $(LIB)/logger.h
	@echo rail.o
	$(GCC) $(SRC)/rail.c -c -o $(BIN)/rail.o

$(LIB)/route.h: $(LIB)/rail.h #$(LIB)/switch.h

$(LIB)/signals.h: $(LIB)/rail.h
$(BIN)/signals.o: $(SRC)/signals.c $(LIB)/signals.h $(LIB)/logger.h
	@echo signal.o
	$(GCC) $(SRC)/signals.c -c -o $(BIN)/signals.o

$(LIB)/switch.h: $(LIB)/rail.h $(LIB)/train.h
$(BIN)/switch.o: $(SRC)/switch.c $(LIB)/switch.h $(LIB)/logger.h
	@echo switch.o
	$(GCC) $(SRC)/switch.c -c -o $(BIN)/switch.o

$(BIN)/system.o: $(SRC)/system.c $(LIB)/system.h $(LIB)/websocket_control.h \
		$(LIB)/logger.h
	@echo system.o
	$(GCC) $(SRC)/system.c -c -o $(BIN)/system.o

$(LIB)/train.h: $(LIB)/rail.h $(LIB)/route.h
$(BIN)/train.o: $(SRC)/train.c $(LIB)/train.h $(LIB)/system.h \
		$(LIB)/logger.h $(LIB)/switch.h
	@echo train.o
	$(GCC) $(SRC)/train.c -c -o $(BIN)/train.o

$(BIN)/train_sim.o: $(SRC)/train_sim.c $(LIB)/train_sim.h $(LIB)/rail.h \
		$(LIB)/train.h $(LIB)/system.h $(LIB)/module.h
	@echo train_sim.o
	$(GCC) $(SRC)/train_sim.c -c -o $(BIN)/train_sim.o

$(BIN)/websocket.o: $(SRC)/websocket.c $(LIB)/websocket.h
	@echo websocket.o
	$(GCC) $(SRC)/websocket.c -c -o $(BIN)/websocket.o

$(LIB)/websocket_control.h: $(LIB)/websocket.h $(LIB)/websocket_msg.h
$(BIN)/websocket_control.o: $(SRC)/websocket_control.c \
		$(LIB)/websocket_control.h $(LIB)/logger.h $(LIB)/encryption.h
	@echo websocket_control.o
	$(GCC) $(SRC)/websocket_control.c -c -o $(BIN)/websocket_control.o

$(BIN)/websocket_msg.o: $(SRC)/websocket_msg.c $(LIB)/websocket_msg.h \
		$(LIB)/system.h $(LIB)/rail.h $(LIB)/switch.h $(LIB)/train.h \
		$(LIB)/logger.h $(LIB)/module.h $(LIB)/Z21.h
	@echo websocket_msg.o
	$(GCC) $(SRC)/websocket_msg.c -c -o $(BIN)/websocket_msg.o

$(BIN)/Z21.o: $(SRC)/Z21.c $(LIB)/Z21.h $(LIB)/logger.h
	@echo Z21.o
	$(GCC) $(SRC)/Z21.c -c -o $(BIN)/Z21.o

$(BIN)/IO.o: $(SRC)/IO.c $(LIB)/IO.h
	@echo IO.o
	$(GCC) $(SRC)/IO.c -c -o $(BIN)/IO.o

.PHONY: clean

clean:
	@echo "CLEAN"
	@rm -f baan
	@rm bin/*
