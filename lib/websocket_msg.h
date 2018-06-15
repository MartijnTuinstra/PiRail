#ifndef _INCLUDE_STATUS_H
  #define _INCLUDE_STATUS_H
  #define ACTIVATE 0
  #define RELEASE  1

  #include <stdint.h>

  struct WS_Message {
    uint16_t type;
    char data[16];
    char data_length;
  };

  // struct WS_Message MessageList[0x1FFF];
  // char MessageCounter = 0;

  void WS_init_Message_List();

  char WS_init_Message(char type);

  void WS_add_Message(uint16_t ID, char length,char data[16]);

  void WS_send_open_Messages(int Client_fd);

  void WS_clear_message(uint16_t ID);

  void WS_EmergencyStop();

  void WS_ShortCircuit();

  void WS_ClearEmergency();

  void WS_EnginesLib(int client_fd);
  void WS_CarsLib(int client_fd);
  void WS_TrainsLib(int client_fd);

  void WS_Partial_Layout(char M_A,char M_B);

  void WS_Track_Layout();

  void WS_trackUpdate(int Client_fd);

  void WS_SwitchesUpdate(int Client_fd);

  void WS_NewClient_track_Switch_Update(int Client_fd);

  void WS_NewTrain(char nr,char M,char B);

  void WS_TrainSplit(char nr,char M1,char B1,char M2,char B2);

  void WS_reset_switches(int client_fd);

  void WS_LinkTrain(uint8_t fID, uint8_t tID);

  void WS_TrainData(char data[14]);
#endif