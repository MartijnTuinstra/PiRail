
files=system logger mem modules config rail signals switch IO algorithm encryption \
      websocket websocket_cts websocket_stc websocket_control Z21 Z21_msg train submodule com sim pathfinding
BIN=../bin
object=$(addprefix $(BIN)/,$(addsuffix .o, $(files)))

test: test.c
	@echo Clear Test Results
	@rm -rf testresults
	@mkdir testresults
	@echo Copy Shared Object
	@cp ../baan.so libbaan.so
	@echo Compile
	@gcc -I../lib -c test.c -o test.o -lpthread -lssl -lcrypto -lm -lcheck -lcheck_pic -lrt -lsubunit -g3
	@gcc test.o $(object) -o test -lpthread -lssl -lcrypto -lm -lcheck -lcheck_pic -lrt -lsubunit -g3
	@echo Test
	@./test
	@rm test
