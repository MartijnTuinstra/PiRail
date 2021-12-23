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
#include "testhelpers.h"

TEST_CASE_METHOD(TestsFixture, "IO  Creation and linking", "[IO][IO-1]" ) {
  char filenames[1][30] = {"./testconfigs/IO-1.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;

  switchboard::SwManager->LinkAndMap();

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
    CHECK(U->B[0]->In_detection  == U->Node[0]->io[9]);
    CHECK(U->B[1]->In_detection  == U->Node[0]->io[10]);
    CHECK(U->B[1]->Out_polarity[0] == U->Node[0]->io[11]);

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

TEST_CASE_METHOD(TestsFixture, "IO Output", "[IO][IO-2]"){
  char filenames[1][30] = {"./testconfigs/IO-1.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;

  switchboard::SwManager->LinkAndMap();

  U->Node[0]->io[22]->setOutput(IO_event_High);
  U->Node[0]->io[23]->setOutput(IO_event_Pulse);
  U->Node[0]->io[24]->setOutput(IO_event_Toggle);

  CHECK(U->Node[0]->io[22]->w_state.value == IO_event_High);
  CHECK(U->Node[0]->io[22]->r_state.value != IO_event_High);

  CHECK(U->Node[0]->io[23]->w_state.value == IO_event_Pulse);
  CHECK(U->Node[0]->io[23]->r_state.value != IO_event_High);

  CHECK(U->Node[0]->io[24]->w_state.value == IO_event_Toggle);
  CHECK(U->Node[0]->io[24]->r_state.value != IO_event_High);

  U->updateIO(); // FIXME: toggle and pulse are put in r_state.

  CHECK(U->Node[0]->io[22]->w_state.value == IO_event_High);
  CHECK(U->Node[0]->io[22]->r_state.value == IO_event_High);

  CHECK(U->Node[0]->io[23]->w_state.value == IO_event_Pulse);
  CHECK(U->Node[0]->io[23]->r_state.value != IO_event_Pulse);

  CHECK(U->Node[0]->io[24]->w_state.value == IO_event_Toggle);
  CHECK(U->Node[0]->io[24]->r_state.value != IO_event_Toggle);

}

TEST_CASE_METHOD(TestsFixture, "IO and Switchboard object", "[IO][IO-3]"){
  char filenames[1][30] = {"./testconfigs/IO-3.bin"};
  loadSwitchboard(filenames, 1);
  loadStock();

  Unit * U = switchboard::Units(1);
  REQUIRE(U);

  U->on_layout = true;

  switchboard::SwManager->LinkAndMap();

  SECTION("I - Blocks"){
    U->B[0]->In_detection->setInput(IO_event_High);

    CHECK(U->B[0]->detectionBlocked);

    // Polarity One Output
    CHECK(U->Node[0]->io[13]->w_state.output == IO_event_Low);
    U->B[1]->flipPolarity(0);
    CHECK(U->Node[0]->io[13]->w_state.output == IO_event_High);
    U->B[1]->flipPolarity(0);
    CHECK(U->Node[0]->io[13]->w_state.output == IO_event_Low);

    // Polarity Two Outputs
    CHECK(U->Node[0]->io[14]->w_state.output == IO_event_Low);
    CHECK(U->Node[0]->io[15]->w_state.output == IO_event_Low);
    U->B[2]->flipPolarity(0);
    CHECK(U->Node[0]->io[14]->w_state.output == IO_event_Low);
    CHECK(U->Node[0]->io[15]->w_state.output == IO_event_Pulse);
    U->updateIO();
    U->B[2]->flipPolarity(0);
    CHECK(U->Node[0]->io[14]->w_state.output == IO_event_Pulse);
    CHECK(U->Node[0]->io[15]->w_state.output == IO_event_Low);
  }

  SECTION("II - Switches"){
    REQUIRE(U->Sw[0]);
    REQUIRE(U->Sw[0]->IO[0]);
    REQUIRE(U->Sw[0]->IO[1]);
    REQUIRE(U->Sw[1]->IO[0]);
    REQUIRE(U->Sw[2]->feedback[0]);
    // Solonoid operated switch

    CHECK(U->Sw[0]->IO[0]->w_state.output == IO_event_Low);
    CHECK(U->Sw[0]->IO[1]->w_state.output == IO_event_Low);

    U->Sw[0]->setState(1);

    CHECK(U->Sw[0]->IO[0]->w_state.output == IO_event_Low);
    CHECK(U->Sw[0]->IO[1]->w_state.output == IO_event_Pulse);

    U->Sw[0]->setState(0);

    CHECK(U->Sw[0]->IO[0]->w_state.output == IO_event_Pulse);
    CHECK(U->Sw[0]->IO[1]->w_state.output == IO_event_Low);

    // Constant signal operated switch
    CHECK(U->Sw[1]->IO[0]->w_state.output == IO_event_Low); 

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

  SECTION("III - MSSwitches"){
    U->MSSw[0]->setState(0);

    CHECK(U->MSSw[0]->IO[0]->w_state.output == IO_event_High);
    CHECK(U->MSSw[0]->IO[1]->w_state.output == IO_event_Low);

    U->MSSw[0]->setState(1);

    CHECK(U->MSSw[0]->IO[0]->w_state.output == IO_event_Low);
    CHECK(U->MSSw[0]->IO[1]->w_state.output == IO_event_High);
  }

  SECTION("IV - Signals"){
    U->Sig[0]->set(DANGER);
    U->Sig[0]->setIO();

    CHECK(U->Node[0]->io[10]->w_state.output == IO_event_High);
    CHECK(U->Node[0]->io[11]->w_state.output == IO_event_Low);
    CHECK(U->Node[0]->io[12]->w_state.output == IO_event_Low);

    U->Sig[0]->set(CAUTION);
    U->Sig[0]->setIO();

    CHECK(U->Node[0]->io[10]->w_state.output == IO_event_Low);
    CHECK(U->Node[0]->io[11]->w_state.output == IO_event_High);
    CHECK(U->Node[0]->io[12]->w_state.output == IO_event_Low);

    U->Sig[0]->set(PROCEED);
    U->Sig[0]->setIO();

    CHECK(U->Node[0]->io[10]->w_state.output == IO_event_Low);
    CHECK(U->Node[0]->io[11]->w_state.output == IO_event_Low);
    CHECK(U->Node[0]->io[12]->w_state.output == IO_event_High);
  }

  // SECTION("V - Stations"){

  // }
}