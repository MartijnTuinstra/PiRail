#include "catch.hpp"

#include "utils/mem.h"
#include "utils/logger.h"
#include "system.h"

#include "config/ModuleConfig.h"
#include "switchboard/manager.h"
#include "switchboard/rail.h"
#include "switchboard/switch.h"
#include "switchboard/msswitch.h"
#include "switchboard/signals.h"
#include "switchboard/station.h"
#include "switchboard/unit.h"

#include "IO.h"
#include "train.h"

void init_test(char (* filenames)[30], int nr_files);

TEST_CASE( "IO  Creation and linking", "[IO][IO-1]" ) {
  char filenames[1][30] = {"./testconfigs/IO-1.bin"};
  init_test(filenames, 1);

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;
  U->link_all();

  SECTION("I - Size and type"){
    REQUIRE(U->IO_Nodes == 1);
    REQUIRE(U->Node[0]->io_ports == 40);

    CHECK(U->Node[0]->io[0]->type == IO_Undefined);
    CHECK(U->Node[0]->io[1]->type == IO_Output);
    CHECK(U->Node[0]->io[2]->type == IO_Output_Blink);
    CHECK(U->Node[0]->io[3]->type == IO_Output_Servo);
    CHECK(U->Node[0]->io[4]->type == IO_Output_PWM);
    CHECK(U->Node[0]->io[5]->type == IO_Input);
    CHECK(U->Node[0]->io[6]->type == IO_Input_Block);
    CHECK(U->Node[0]->io[7]->type == IO_Input_Switch);
    CHECK(U->Node[0]->io[8]->type == IO_Input_MSSwitch);
  }

  SECTION("II - linking"){
    CHECK(U->B[0]->In  == U->Node[0]->io[9]);
    CHECK(U->B[1]->In  == U->Node[0]->io[10]);
    CHECK(U->B[1]->dir_Out == U->Node[0]->io[11]);

    // ======== SWITCHES ========
    CHECK(U->Sw[0]->IO[0] == U->Node[0]->io[12]);

    CHECK(U->Sw[1]->IO[0] == U->Node[0]->io[13]);
    CHECK(U->Sw[1]->IO[1] == U->Node[0]->io[14]);

    // ======== MSSWITCHES ========
    CHECK(U->MSSw[0]->IO[0] == U->Node[0]->io[15]);

    CHECK(U->MSSw[1]->IO[0] == U->Node[0]->io[16]);
    CHECK(U->MSSw[1]->IO[1] == U->Node[0]->io[17]);

    // ======== SIGNALS ========
    CHECK(U->Sig[0]->output[0] == U->Node[0]->io[18]);

    CHECK(U->Sig[1]->output[0] == U->Node[0]->io[19]);
    CHECK(U->Sig[1]->output[1] == U->Node[0]->io[20]);
  }
}

TEST_CASE( "IO Output", "[IO][IO-2]"){
  char filenames[1][30] = {"./testconfigs/IO-1.bin"};
  init_test(filenames, 1);

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;
  U->link_all();


  U->Node[0]->io[22]->setOutput(IO_event_High);

  REQUIRE(U->Node[0]->io[22]->w_state.value == IO_event_High);
  REQUIRE(U->Node[0]->io[22]->r_state.value != IO_event_High);

  U->updateIO(0);

  REQUIRE(U->Node[0]->io[22]->w_state.value == IO_event_High);
  REQUIRE(U->Node[0]->io[22]->r_state.value == IO_event_High);
}

TEST_CASE( "IO and Switchboard object", "[IO][IO-3]"){
  char filenames[1][30] = {"./testconfigs/IO-3.bin"};
  init_test(filenames, 1);

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;
  U->link_all();

  SECTION("I - Blocks"){
    U->B[0]->In->setInput(IO_event_High);

    CHECK(U->B[0]->detectionblocked);
  }

  SECTION("II - Switches"){
    logger.setlevel_stdout(INFO);
    loggerf(ERROR, "Setting Sw 0 to state 1");
    // Solonoid operated switch
    U->Sw[0]->setState(1);

    CHECK(U->Sw[0]->IO[0]->w_state.output == IO_event_Low);
    CHECK(U->Sw[0]->IO[1]->w_state.output == IO_event_Pulse);

    // Constant signal operated switch
    U->Sw[1]->setState(1);

    CHECK(U->Sw[1]->IO[0]->w_state.output == IO_event_High); 

    // Feedback
    U->Sw[2]->setState(1);

    CHECK(U->Sw[2]->feedbackWrongState);
    CHECK(U->B[0]->switchWrongFeedback);

    U->Sw[2]->feedback[0]->setInput(IO_event_High);

    CHECK(!U->Sw[2]->feedbackWrongState);
    CHECK(!U->B[0]->switchWrongFeedback);
  }

  // SECTION("III - MSSwitches"){

  // }

  // SECTION("IV - Signals"){

  // }

  // SECTION("V - Stations"){

  // }
}