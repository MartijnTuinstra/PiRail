BIN=bin
SRC=src
LIB=../lib

GCC_DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
GCC_INCLUDE = -I $(LIB) -I ../generated/lib -I .
GCC_ERROR_FLAGS = -Werror=unused-variable -Wno-packed-bitfield-compat -Wno-unused-parameter -Wall
GCC_LIBS = -lpthread -lssl -lcrypto -lm
GCC_FLAGS = -D _DEFAULT_SOURCE

GCC = g++ -std=c++14 -g3 $(GCC_INCLUDE)
GCC_options = $(GCC_ERROR_FLAGS) $(GCC_FLAGS)

ROLLINGSTOCK_FILES = railtrain
SWITCHBOARD_FILES = rail switch msswitch signal station
SRC_FILES = path pathfinding algorithm trainfollowing switchsolver polaritysolver io configReader Z21 railtrain
TEST_FILES = main testhelpers $(addprefix src/,$(SRC_FILES)) $(addprefix switchboard/,$(SWITCHBOARD_FILES)) # $(addprefix rollingstock/,$(ROLLINGSTOCK_FILES))

.DEFAULT_GOAL := all

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

-include $(call rwildcard,bin,*.d)

$(BIN)/%.o: %.c
	@echo '(  GCC  ) -$@'
	@$(GCC) -c $*.c -MP -MMD -MT '$@ $(BIN)/$*.d' -o $(BIN)/$*.o

$(BIN)/%.o: %.cpp
	@echo '(  G++  ) -$@'
	@$(GCC) -c $*.cpp -MP -MMD -MT '$@ $(BIN)/$*.d' -o $(BIN)/$*.o

# libbaan.so: FORCE
# 	@echo "( BUILD ) baan"
# 	@$(MAKE) -C .. baan
# 	@cp ../baan.so libbaan.so

BAAN_FILES = $(filter-out ../bin/baan.o ../bin/config_reader.o ../bin/comtest.o, $(call rwildcard,../bin,*.o))

maintest: $(addprefix $(BIN)/,$(addsuffix .o, $(TEST_FILES)))
	@echo "( BUILD ) maintest"
	@$(GCC) -o $@ $(addprefix $(BIN)/,$(addsuffix .o, $(TEST_FILES))) $(BAAN_FILES) -D CB_NON_AVR $(GCC_options) $(GCC_LIBS)

.PHONY: all run clean_test clean_build clean build
# FORCE: ;

build: buildBaan maintest
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
