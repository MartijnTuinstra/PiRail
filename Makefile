BIN=./bin
SRC=./src
LIB=./lib
ARGS=-std=c99 -lpthread -lssl -lcrypto -lwiringPi -lm -g


baan: baan.o $(BIN)/algorithm.o $(BIN)/com.o $(BIN)/encryption.o $(BIN)/modules.o $(BIN)/pathfinding.o $(BIN)/rail.o $(BIN)/signals.o $(BIN)/status.o $(BIN)/switch.o $(BIN)/train_control.o $(BIN)/train_sim.o $(BIN)/trains.o $(BIN)/websocket.o $(BIN)/Z21.o
	gcc $(ARGS) baan.o -o baan $(wildcard $(BIN)/*.o)

baan.o: baan.c $(LIB)/system.h $(LIB)/train_sim.h $(LIB)/websocket.h $(LIB)/status.h $(LIB)/Z21.h $(LIB)/com.h $(LIB)/rail.h $(LIB)/switch.h $(LIB)/signals.h $(LIB)/trains.h $(LIB)/modules.h $(LIB)/algorithm.h $(LIB)/pathfinding.h
	gcc $(ARGS) baan.c -c -o baan.o

$(BIN)/algorithm.o : $(SRC)/algorithm.c $(LIB)/system.h $(LIB)/algorithm.h $(LIB)/rail.h $(LIB)/trains.h $(LIB)/switch.h $(LIB)/signals.h $(LIB)/modules.h $(LIB)/com.h $(LIB)/status.h
	@echo Algorithm.o
	gcc $(SRC)/algorithm.c -c -o $(BIN)/algorithm.o $(ARGS)

$(BIN)/com.o : $(LIB)/com.h $(LIB)/system.h $(LIB)/rail.h $(LIB)/switch.h $(LIB)/signals.h $(LIB)/trains.h $(LIB)/modules.h
	@echo COM.o
	gcc $(SRC)/com.c -c -o $(BIN)/com.o $(ARGS)

$(BIN)/encryption.o : $(SRC)/encryption.c $(LIB)/encryption.h
	@echo encryption.o
	gcc $(SRC)/encryption.c -c -o $(BIN)/encryption.o $(ARGS)

$(BIN)/modules.o : $(SRC)/modules.c $(LIB)/system.h $(LIB)/rail.h $(LIB)/switch.h $(LIB)/signals.h $(LIB)/trains.h $(LIB)/modules.h $(LIB)/algorithm.h
	@echo modules.o
	gcc $(SRC)/modules.c -c -o $(BIN)/modules.o $(ARGS)

$(BIN)/pathfinding.o : $(SRC)/pathfinding.c $(LIB)/system.h $(LIB)/rail.h $(LIB)/switch.h $(LIB)/trains.h $(LIB)/pathfinding.h
	@echo pathfinding.o
	gcc $(SRC)/pathfinding.c -c -o $(BIN)/pathfinding.o $(ARGS)

$(BIN)/rail.o : $(SRC)/rail.c $(LIB)/system.h $(LIB)/rail.h $(LIB)/modules.h $(LIB)/switch.h
	@echo rail.o
	gcc $(SRC)/rail.c -c -o $(BIN)/rail.o $(ARGS)

$(BIN)/signals.o : $(SRC)/signals.c $(LIB)/system.h $(LIB)/signals.h $(LIB)/rail.h $(LIB)/modules.h $(LIB)/com.h
	@echo signals.o
	gcc $(SRC)/signals.c -c -o $(BIN)/signals.o $(ARGS)

$(BIN)/status.o : $(SRC)/status.c $(LIB)/websocket.h $(LIB)/system.h $(LIB)/rail.h $(LIB)/switch.h $(LIB)/trains.h $(LIB)/modules.h $(LIB)/Z21.h
	@echo status.o
	gcc $(SRC)/status.c -c -o $(BIN)/status.o $(ARGS)

$(BIN)/switch.o : $(SRC)/switch.c $(LIB)/system.h $(LIB)/rail.h $(LIB)/switch.h $(LIB)/trains.h $(LIB)/websocket.h $(LIB)/pathfinding.h $(LIB)/modules.h $(LIB)/com.h
	@echo switch.o
	gcc $(SRC)/switch.c -c -o $(BIN)/switch.o $(ARGS)

$(BIN)/train_control.o : $(SRC)/train_control.c $(LIB)/trains.h
	@echo train_control.o
	gcc $(SRC)/train_control.c -c -o $(BIN)/train_control.o $(ARGS)

$(BIN)/train_sim.o : $(SRC)/train_sim.c $(LIB)/system.h $(LIB)/rail.h $(LIB)/trains.h $(LIB)/modules.h
	@echo train_sim.o
	gcc $(SRC)/train_sim.c -c -o $(BIN)/train_sim.o $(ARGS)

$(BIN)/trains.o : $(SRC)/trains.c $(LIB)/system.h $(LIB)/rail.h $(LIB)/switch.h $(LIB)/trains.h $(LIB)/pathfinding.h $(LIB)/com.h $(LIB)/Z21.h
	@echo trains.o
	gcc $(SRC)/trains.c -c -o $(BIN)/trains.o $(ARGS)

$(BIN)/websocket.o : $(SRC)/websocket.c $(LIB)/system.h $(LIB)/rail.h $(LIB)/switch.h $(LIB)/trains.h $(LIB)/websocket.h $(LIB)/status.h $(LIB)/encryption.h $(LIB)/modules.h $(LIB)/Z21.h
	@echo websocket.o
	gcc $(SRC)/websocket.c -c -o $(BIN)/websocket.o $(ARGS)

$(BIN)/Z21.o : $(SRC)/Z21.c $(LIB)/Z21.h $(LIB)/status.h $(LIB)/trains.h
	@echo Z21.o
	gcc $(SRC)/Z21.c -c -o $(BIN)/Z21.o $(ARGS)

