#include "algorithm.h"
#include "com.h"
#include "config.h"
#include "config_data.h"
#include "encryption.h"
#include "IO.h"
#include "logger.h"
#include "mem.h"
#include "modules.h"
#include "rail.h"
#include "signals.h"
#include "sim.h"
#include "submodule.h"
#include "switch.h"
#include "system.h"
#include "train.h"
#include "websocket.h"
#include "websocket_control.h"
#include "websocket_msg.h"
#include "Z21.h"
#include "Z21_msg.h"

#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <check.h>

struct s_systemState * SYS;

START_TEST (loadTest)
{
	printf("loadTest\n");

	init_main();
	init_logger_file_only("testresults/loadTest.txt");
	set_level(INFO);

	Units = _calloc(30, Unit *);
	unit_len = 30;

	read_module_Config(0);

	// Check Valuse
	fail_unless(U_B(0,0)->max_speed == 180);
	fail_unless(U_B(0,12)->dir == 1);
	fail_unless(U_B(0,0)->length == 40);
	fail_unless(U_B(0,0)->oneWay == 1);

	fail_unless(U_B(0,0)->prev.module == 0);
	fail_unless(U_B(0,0)->prev.id == 1);
	fail_unless(U_B(0,0)->prev.type == 0);

	fail_unless(U_B(0,0)->next.module == 1);
	fail_unless(U_B(0,0)->next.id == 1);
	fail_unless(U_B(0,0)->next.type == 254);

	// Check Station
	fail_unless(U_St(0,0)->blocks_len == 5);
	fail_unless(U_B(0,3)->station == U_St(0,0));
	fail_unless(U_B(0,4)->station == U_St(0,0));
	fail_unless(U_B(0,5)->station == U_St(0,0));
	fail_unless(U_B(0,6)->station == U_St(0,0));
	fail_unless(U_B(0,7)->station == U_St(0,0));
	fail_unless(strcmp(U_St(0,0)->name, "Spoor_1") == 0);

	unload_module_Configs();

	exit_logger();
	_free(SYS);
	SYS = 0;
	print_allocs();
}
END_TEST

START_TEST (BlockLinkTest)
{
	printf("BlockLinkTest\n");

	init_main();
	init_logger_file_only("testresults/BlockLinkTest.txt");
	set_level(INFO);

	Units = _calloc(30, Unit *);
	unit_len = 30;

	read_module_Config(0);
	SIM_Connect_Rail_links();

	// Check block links
	fail_unless(U_B(0,2)->prev.p == Units[0]->B[3]);
	fail_unless(U_B(0,2)->next.p == Units[0]->B[1]);

	// Check Switchs
	fail_unless(U_B(0,13)->prev.p == Units[0]->Sw[0]);
	fail_unless(U_Sw(0,0)->app.p == Units[0]->B[13]);
	fail_unless(U_Sw(0,0)->str.p == Units[0]->B[14]);
	fail_unless(U_Sw(0,0)->div.p == 0);

	unload_module_Configs();

	exit_logger();
	_free(SYS);
	SYS = 0;
	print_allocs();
}
END_TEST

START_TEST (BlockAlgorithmTest)
{
	printf("BlockAlgorithmTest\n");

	init_main();
	init_logger("testresults/BlockAlgorithmTest.txt");
	set_level(INFO);

	Units = _calloc(30, Unit *);
	unit_len = 30;

	read_module_Config(0);
	SIM_Connect_Rail_links();

	for(int i = 0; i < Units[0]->block_len; i++){
		if(!U_B(0, i))
			continue;
		Algor_process(U_B(0,i), _FORCE);
	}

	// Check Algor Blocks
	fail_unless(U_B(0,5)->Alg.next == 5);
	fail_unless(U_B(0,5)->Alg.next1 == 2);
	fail_unless(U_B(0,5)->Alg.next2 == 4);
	fail_unless(U_B(0,5)->Alg.next3 == 5);
	fail_unless(U_B(0,5)->Alg.prev == 6);
	fail_unless(U_B(0,5)->Alg.prev1 == 2);
	fail_unless(U_B(0,5)->Alg.prev2 == 4);
	fail_unless(U_B(0,5)->Alg.prev3 == 6);

	fail_unless(U_B(0,5)->Alg.N[3] == U_B(0,1));
	fail_unless(U_B(0,5)->Alg.N[2] == U_B(0,2));
	fail_unless(U_B(0,5)->Alg.N[1] == U_B(0,3));
	fail_unless(U_B(0,5)->Alg.N[0] == U_B(0,4));
	fail_unless(U_B(0,5)->Alg.P[0] == U_B(0,6));
	fail_unless(U_B(0,5)->Alg.P[1] == U_B(0,7));
	fail_unless(U_B(0,5)->Alg.P[2] == U_B(0,8));
	fail_unless(U_B(0,5)->Alg.P[3] == U_B(0,9));

	fail_unless(U_B(0,12)->Alg.next == 6);
	fail_unless(U_B(0,12)->Alg.N[0] == U_B(0,13));
	fail_unless(U_B(0,12)->Alg.N[1] == U_B(0,14));
	fail_unless(U_B(0,12)->Alg.N[2] == U_B(0,15));
	fail_unless(U_B(0,12)->Alg.N[3] == U_B(0,16));
	fail_unless(U_B(0,12)->Alg.N[4] == U_B(0,17));
	fail_unless(U_B(0,12)->Alg.N[5] == U_B(0,18));

	fail_unless(U_B(0,20)->Alg.next == 3);
	fail_unless(U_B(0,20)->Alg.N[0] == U_B(0,21));
	fail_unless(U_B(0,20)->Alg.N[1] == U_B(0,22));
	fail_unless(U_B(0,20)->Alg.N[2] == U_B(0,23));

	mark_point();

	fail_unless(U_B(0,24)->Alg.next == 2);
	fail_unless(U_B(0,24)->Alg.N[0] == U_B(0,25));
	fail_unless(U_B(0,24)->Alg.N[1] == U_B(0,26));

	fail_unless(U_B(0,30)->Alg.next == 3);
	fail_unless(U_B(0,30)->Alg.N[0] == U_B(0,29));
	fail_unless(U_B(0,30)->Alg.N[1] == U_B(0,28));
	fail_unless(U_B(0,30)->Alg.N[2] == U_B(0,27));

	mark_point();

	loggerf(WARNING, "Throw switch");
	throw_switch(U_Sw(0,0), 1, 0);
	throw_switch(U_Sw(0,1), 1, 0);
	throw_switch(U_Sw(0,2), 1, 0);
	throw_switch(U_Sw(0,3), 1, 0);

	processAlgorQueue();

	fail_unless(U_B(0,12)->Alg.next == 1);
	fail_unless(U_B(0,12)->Alg.N[1] == 0);
	
	fail_unless(U_B(0,20)->Alg.next == 1);
	fail_unless(U_B(0,20)->Alg.N[1] == 0);

	fail_unless(U_B(0,24)->Alg.next == 3);
	fail_unless(U_B(0,30)->Alg.next == 3);

	mark_point();

	unload_module_Configs();

	exit_logger();
	_free(SYS);
	SYS = 0;
	print_allocs();
}
END_TEST

START_TEST (BlockStatingTest)
{
	printf("BlockStatingTest\n");

	init_main();
	init_logger_file_only("testresults/BlockStatingTest.txt");
	set_level(INFO);

	Units = _calloc(30, Unit *);
	unit_len = 30;

	read_module_Config(0);
	load_rolling_Configs();
	SIM_Connect_Rail_links();

	U_B(0, 3)->blocked = 1;
	U_Sw(0,1)->state = 1;

	U_B(0,14)->reserved = 1;
	U_B(0,14)->dir ^= 0b100;

	U_B(0,20)->state = CAUTION;

	mark_point();

	for(int i = 0; i < Units[0]->block_len; i++){
		if(!U_B(0, i))
			continue;
		Algor_process(U_B(0,i), _FORCE);
	}

	// Check Valuse
	fail_unless(U_B(0,3)->state == BLOCKED, "REQ1");
	fail_unless(U_B(0,4)->state == DANGER,  "REQ2");
	fail_unless(U_B(0,5)->state == CAUTION, "REQ3");
	fail_unless(U_B(0,6)->state == PROCEED, "REQ4");

	fail_unless(U_B(0,21)->state == CAUTION, "REQ5");

	fail_unless(U_B(0,14)->state == RESERVED);
	fail_unless(U_B(0,14)->reverse_state == DANGER, "REQ8");
	fail_unless(U_B(0,13)->state == DANGER,  "REQ6");
	fail_unless(U_B(0,12)->state == CAUTION, "REQ6");

	fail_unless(U_B(0,20)->state == PROCEED, "REQ7");

	unload_module_Configs();
	unload_rolling_Configs();

	exit_logger();
	_free(SYS);
	SYS = 0;
	print_allocs();
}
END_TEST

START_TEST (TrainFollowingTest)
{
	printf("TrainFollowingTest\n");

	init_main();
	init_logger_file_only("testresults/TrainFollowingTest.txt");
	set_level(INFO);

	Units = _calloc(30, Unit *);
	unit_len = 30;

	read_module_Config(0);
	load_rolling_Configs();
	SIM_Connect_Rail_links();

	U_B(0, 5)->blocked = 1;

	mark_point();

	for(int i = 0; i < Units[0]->block_len; i++){
		if(!U_B(0, i))
			continue;
		Algor_process(U_B(0,i), _FORCE);
	}

	// Check Valuse
	fail_unless(U_B(0,5)->train != 0);

	Block * B = U_B(0,4);
	B->IOchanged = 1;	B->statechanged = 1;	Units[B->module]->block_state_changed = 1;	B->blocked = 1;
	putAlgorQueue(B, 1);
	processAlgorQueue();

	mark_point();

	fail_unless(U_B(0,4)->train == U_B(0,5)->train);

	B = U_B(0,5);
	B->IOchanged = 1;	B->statechanged = 1;	Units[B->module]->block_state_changed = 1;	B->blocked = 0;
	putAlgorQueue(B, 1);
	processAlgorQueue();

	mark_point();

	fail_unless(U_B(0,5)->train == 0);

	// Reverse check
	B->IOchanged = 1;	B->statechanged = 1;	Units[B->module]->block_state_changed = 1;	B->blocked = 1;
	putAlgorQueue(B, 1);
	processAlgorQueue();

	mark_point();

	fail_unless(U_B(0,5)->train == U_B(0,4)->train);
	fail_unless(U_B(0,3)->dir == 0);
	fail_unless(U_B(0,4)->dir == 0b100);
	fail_unless(U_B(0,5)->dir == 0b100);
	fail_unless(U_B(0,6)->dir == 0b100);
	fail_unless(U_B(0,7)->dir == 0b100);
	fail_unless(U_B(0,8)->dir == 0b100);
	fail_unless(U_B(0,9)->dir == 0b100);
	fail_unless(U_B(0,10)->dir == 0b100);
	fail_unless(U_B(0,11)->dir == 0b100);

	B = U_B(0,4);
	B->IOchanged = 1;	B->statechanged = 1;	Units[B->module]->block_state_changed = 1;	B->blocked = 0;
	putAlgorQueue(B, 1);
	processAlgorQueue();

	mark_point();

	B = U_B(0,14);
	B->IOchanged = 1;	B->statechanged = 1;	Units[B->module]->block_state_changed = 1;	B->blocked = 1;
	putAlgorQueue(B, 1);
	processAlgorQueue();

	B = U_B(0,14);
	B->IOchanged = 1;	B->statechanged = 1;	Units[B->module]->block_state_changed = 1;	B->blocked = 0;
	putAlgorQueue(B, 1);
	processAlgorQueue();

	fail_unless(U_B(0,14)->train != 0);
	loggerf(INFO, "%i == %i", U_B(0,14)->state, UNKNOWN);
	fail_unless(U_B(0,14)->state == UNKNOWN);

	mark_point();

	unload_module_Configs();
	unload_rolling_Configs();

	exit_logger();
	_free(SYS);
	SYS = 0;
	print_allocs();
}
END_TEST


int main(void)
{
	Suite *s1 = suite_create("Core");
	TCase *tc1_1 = tcase_create("Core");
	SRunner *sr = srunner_create(s1);
	int nf;

	suite_add_tcase(s1, tc1_1);
	tcase_add_test(tc1_1, loadTest);
	tcase_add_test(tc1_1, BlockLinkTest);
	tcase_add_test(tc1_1, BlockAlgorithmTest);
	tcase_add_test(tc1_1, BlockStatingTest);
	tcase_add_test(tc1_1, TrainFollowingTest);

	srunner_run_all(sr, CK_VERBOSE);
	nf = srunner_ntests_failed(sr);
	srunner_free(sr);

	return nf == 0 ? 0 : 1;
}
 

