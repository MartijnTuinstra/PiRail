BIN=./bin
SRC=./src
LIB=./lib
INCLUDE = -I $(LIB) -I $(SRC)
ARGS=-std=c99 -lpthread -lssl -lcrypto -lwiringPi -lm -g $(INCLUDE)

GCC = gcc $(ARGS)

ifndef VERBOSE
.SILENT:
endif

baan: $(BIN)/baan.o $(BIN)/logger.o $(BIN)/rail.o $(BIN)/train.o $(BIN)/system.o $(BIN)/websocket_control.o $(BIN)/websocket_msg.o $(BIN)/module.o \
		$(BIN)/train_sim.o $(BIN)/com.o $(BIN)/algorithm.o $(BIN)/signals.o $(BIN)/switch.o $(BIN)/Z21.o
	@echo baan
	$(GCC) -o baan $(wildcard $(BIN)/*.o)

$(BIN)/baan.o: baan.c $(LIB)/logger.h $(LIB)/train.h $(LIB)/rail.h $(LIB)/switch.h $(LIB)/com.h $(LIB)/websocket_control.h $(LIB)/system.h
	@echo baan.o
	$(GCC) baan.c -c -o $(BIN)/baan.o

$(LIB)/algorithm.h: $(LIB)/rail.h
$(BIN)/algorithm.o: $(SRC)/algorithm.c $(LIB)/algorithm.h $(LIB)/system.h \
		$(LIB)/logger.h $(LIB)/train.h $(LIB)/switch.h $(LIB)/signals.h \
		$(LIB)/module.h $(LIB)/com.h $(LIB)/websocket_msg.h
	@echo algorithm.o
	$(GCC) $(SRC)/algorithm.c -c -o $(BIN)/algorithm.o

$(LIB)/con.h: $(LIB)/signals.h
$(BIN)/com.o: $(SRC)/com.c $(LIB)/com.h $(LIB)/system.h $(LIB)/rail.h \
		$(LIB)/switch.h $(LIB)/signals.h $(LIB)/train.h $(LIB)/logger.h \
		$(LIB)/module.h
	@echo com.c
	$(GCC) $(SRC)/com.c -c -o $(BIN)/com.o

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

$(LIB)/websocket_control.h: $(LIB)/websocket.h $(LIB)/websocket_msg.h
$(BIN)/websocket_control.o: $(SRC)/websocket_control.c \
		$(LIB)/websocket_control.h $(LIB)/logger.h
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
# baan: baan.o $(BIN)/algorithm.o $(BIN)/com.o $(BIN)/encryption.o $(BIN)/modules.o $(BIN)/pathfinding.o $(BIN)/rail.o $(BIN)/signals.o $(BIN)/status.o $(BIN)/switch.o $(BIN)/train_control.o $(BIN)/train_sim.o $(BIN)/trains.o $(BIN)/websocket.o $(BIN)/Z21.o $(BIN)/logger.o
# 	gcc $(ARGS) -o baan $(wildcard $(BIN)/*.o) baan.o

# baan.o: baan.c $(LIB)/system.h $(LIB)/train_sim.h $(LIB)/websocket.h $(LIB)/status.h $(LIB)/Z21.h $(LIB)/com.h $(LIB)/rail.h $(LIB)/switch.h $(LIB)/signals.h $(LIB)/trains.h $(LIB)/modules.h $(LIB)/algorithm.h $(LIB)/pathfinding.h $(LIB)/logger.h
# 	gcc $(ARGS) baan.c -c -o baan.o

# $(BIN)/algorithm.o : $(SRC)/algorithm.c $(LIB)/system.h $(LIB)/algorithm.h $(LIB)/rail.h $(LIB)/trains.h $(LIB)/switch.h $(LIB)/signals.h $(LIB)/modules.h $(LIB)/com.h $(LIB)/status.h
# 	@echo Algorithm.o
# 	gcc $(SRC)/algorithm.c -c -o $(BIN)/algorithm.o $(ARGS)

# $(BIN)/com.o : $(SRC)/com.c $(LIB)/com.h $(LIB)/system.h $(LIB)/rail.h $(LIB)/switch.h $(LIB)/signals.h $(LIB)/trains.h $(LIB)/modules.h $(LIB)/logger.h
# 	@echo COM.o
# 	gcc $(SRC)/com.c -c -o $(BIN)/com.o $(ARGS)

# $(BIN)/encryption.o : $(SRC)/encryption.c $(LIB)/encryption.h
# 	@echo encryption.o
# 	gcc $(SRC)/encryption.c -c -o $(BIN)/encryption.o $(ARGS)

# $(LIB)/modules.h: $(LIB)/switch.h $(LIB)/rail.h $(LIB)/signals.h
# $(BIN)/modules.o : $(SRC)/modules.c $(LIB)/system.h $(LIB)/trains.h $(LIB)/modules.h $(LIB)/algorithm.h $(LIB)/websocket.h $(LIB)/logger.h
# 	@echo modules.o
# 	gcc $(SRC)/modules.c -c -o $(BIN)/modules.o $(ARGS)

# $(BIN)/pathfinding.o : $(SRC)/pathfinding.c $(LIB)/system.h $(LIB)/rail.h $(LIB)/switch.h $(LIB)/trains.h $(LIB)/pathfinding.h
# 	@echo pathfinding.o
# 	gcc $(SRC)/pathfinding.c -c -o $(BIN)/pathfinding.o $(ARGS)

# $(BIN)/rail.o : $(SRC)/rail.c $(LIB)/system.h $(LIB)/rail.h $(LIB)/modules.h $(LIB)/switch.h
# 	@echo rail.o
# 	gcc $(SRC)/rail.c -c -o $(BIN)/rail.o $(ARGS)

# $(BIN)/signals.o : $(SRC)/signals.c $(LIB)/system.h $(LIB)/signals.h $(LIB)/rail.h $(LIB)/modules.h $(LIB)/com.h
# 	@echo signals.o
# 	gcc $(SRC)/signals.c -c -o $(BIN)/signals.o $(ARGS)

# $(BIN)/status.o : $(SRC)/status.c $(LIB)/websocket.h $(LIB)/system.h $(LIB)/rail.h $(LIB)/switch.h $(LIB)/trains.h $(LIB)/modules.h $(LIB)/Z21.h $(LIB)/logger.h
# 	@echo status.o
# 	gcc $(SRC)/status.c -c -o $(BIN)/status.o $(ARGS)

# $(LIB)/switch.h: $(LIB)/rail.h $(LIB)/trains.h
# $(BIN)/switch.o : $(SRC)/switch.c $(LIB)/system.h $(LIB)/trains.h $(LIB)/websocket.h $(LIB)/pathfinding.h $(LIB)/modules.h $(LIB)/com.h $(LIB)/logger.h
# 	@echo switch.o
# 	gcc $(SRC)/switch.c -c -o $(BIN)/switch.o $(ARGS)

# $(BIN)/train_control.o : $(SRC)/train_control.c $(LIB)/trains.h
# 	@echo train_control.o
# 	gcc $(SRC)/train_control.c -c -o $(BIN)/train_control.o $(ARGS)

# $(BIN)/train_sim.o : $(SRC)/train_sim.c $(LIB)/system.h $(LIB)/rail.h $(LIB)/trains.h $(LIB)/modules.h
# 	@echo train_sim.o
# 	gcc $(SRC)/train_sim.c -c -o $(BIN)/train_sim.o $(ARGS)

# $(LIB)/trains.h: $(LIB)/rail.h
# $(BIN)/trains.o : $(SRC)/trains.c $(LIB)/system.h $(LIB)/switch.h $(LIB)/trains.h $(LIB)/pathfinding.h $(LIB)/com.h $(LIB)/Z21.h $(LIB)/logger.h
# 	@echo trains.o
# 	gcc $(SRC)/trains.c -c -o $(BIN)/trains.o $(ARGS)

# $(BIN)/websocket.o : $(SRC)/websocket.c $(LIB)/system.h $(LIB)/switch.h $(LIB)/trains.h $(LIB)/websocket.h $(LIB)/status.h $(LIB)/encryption.h $(LIB)/modules.h $(LIB)/Z21.h $(LIB)/logger.h
# 	@echo websocket.o
# 	gcc $(SRC)/websocket.c -c -o $(BIN)/websocket.o $(ARGS)

# $(BIN)/Z21.o : $(SRC)/Z21.c $(LIB)/Z21.h $(LIB)/status.h $(LIB)/trains.h
# 	@echo Z21.o
# 	gcc $(SRC)/Z21.c -c -o $(BIN)/Z21.o $(ARGS)

# $(BIN)/logger.o : $(SRC)/logger.c
# 	@echo logger.o
# 	gcc $(SRC)/logger.c -c -o $(BIN)/logger.o $(ARGS)


clean: baan.c
	sudo rm baan
	sudo rm bin/*
