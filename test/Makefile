BIN=bin
SRC=src
LIB=../lib

GCC_DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
GCC_INCLUDE = -I $(LIB) -I ../generated/lib -I .
GCC_ERROR_FLAGS = -Werror=unused-variable -Wno-packed-bitfield-compat -Wno-unused-parameter -Wall
GCC_LIBS = -lpthread -lssl -lcrypto -lm
GCC_FLAGS = -D _DEFAULT_SOURCE

GCC = g++ -std=c++14 -g3 $(GCC_INCLUDE) $(GCC_ERROR_FLAGS) $(GCC_LIBS) $(GCC_FLAGS)

ROLLINGSTOCK_FILES = railtrain
SWITCHBOARD_FILES = rail switch msswitch signal station
SRC_FILES = path pathfinding algorithm io configReader Z21
TEST_FILES = main testhelpers $(addprefix src/,$(SRC_FILES)) $(addprefix switchboard/,$(SWITCHBOARD_FILES)) # $(addprefix rollingstock/,$(ROLLINGSTOCK_FILES))

.DEFAULT_GOAL := all

$(BIN)/%.o: %.c
	@echo '(  GCC  ) -$@'
	@$(GCC) -c $*.c -MP -MMD -MT '$@ $(BIN)/$*.d' -o $(BIN)/$*.o

$(BIN)/%.o: %.cpp
	@echo '(  G++  ) -$@'
	@$(GCC) -c $*.cpp -MP -MMD -MT '$@ $(BIN)/$*.d' -o $(BIN)/$*.o

libbaan.so: FORCE
	@echo "( BUILD ) baan"
	@$(MAKE) -C .. baan
	@cp ../baan.so libbaan.so

maintest: $(addprefix $(BIN)/,$(addsuffix .o, $(TEST_FILES))) libbaan.so
	@echo "( BUILD ) maintest"
	@$(GCC) -o $@ $(addprefix $(BIN)/,$(addsuffix .o, $(TEST_FILES))) -L . -lbaan -Wl,-rpath=/var/www/html/baan/pirail_avr/test/

.PHONY: all run clean_test clean_build clean build
FORCE: ;

build: maintest
all: run
clean: clean_test clean_build

run: clean_test maintest
	@echo "Starting Testing"
	@./maintest -r console

clean_test:
	@echo "Clear Test Results"
	@rm -rf testresults/*

clean_build:
	@echo "Clear build"
	@rm bin/src/*
	@rm bin/switchboard/*
	@rm bin/testhelpers.*
