
files=system logger mem modules config rail signals switch IO algorithm encryption scheduler \
      websocket websocket_cts websocket_stc websocket_control Z21 Z21_msg train submodule com sim pathfinding \
      switchboard/rail switchboard/switch switchboard/unit
BIN=../bin
object=$(addprefix $(BIN)/,$(addsuffix .o, $(files)))

configs/units/0.bin: ../configs/units/0.bin
	cp ../configs/units/0.bin configs/units/0.bin

test: test.c
	@echo Clear Test Results
	@rm -rf testresults
	@mkdir testresults
	@echo Copy Shared Object
	@cp ../baan.so libbaan.so
	@echo Compile
	@g++ -I../lib -c test.c -o test.o -lpthread -lssl -lcrypto -lm -lcheck -lcheck_pic -lrt -lsubunit -g3
	@g++ test.o $(object) -o test -lpthread -lssl -lcrypto -lm -lcheck -lcheck_pic -lrt -lsubunit -g3
	@echo Test
	@./test
	@rm test
