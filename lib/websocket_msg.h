#ifndef _INCLUDE_WEBSOCKET_MSG_H
  #define _INCLUDE_WEBSOCKET_MSG_H
  #define ACTIVATE 0
  #define RELEASE  1

  #include <stdint.h>
  #include "websocket.h"
  #include "train.h"

  struct WS_Message {
    uint16_t type;
    char data[16];
    char data_length;
  };

  // struct WS_Message MessageList[0x1FFF];
  // char MessageCounter = 0;


  //System Messages
  void WS_Partial_Layout(uint8_t M_A,uint8_t M_B);
  void WS_Track_Layout(int Client_fd);

  void WS_stc_Z21_info(int client_fd);
  void WS_stc_Z21_IP(int client_fd);

void WS_cts_Enable_Disable_SubmoduleState(uint8_t opcode, uint8_t flags);
  void WS_stc_SubmoduleState();

  //Admin Messages

  //Train Messages
  void WS_UpdateTrain(void * t, int type);
  void WS_TrainSubscribe(uint8_t * data, struct web_client_t * client);


  void WS_EnginesLib(int client_fd);
  void WS_CarsLib(int client_fd);
  void WS_TrainsLib(int client_fd);
  void WS_stc_TrainCategories(int client_fd);

  void WS_NewTrain(char nr,char M,char B);
  void WS_TrainSplit(char nr,char M1,char B1,char M2,char B2);
  void WS_LinkTrain(uint8_t fID, uint8_t tID);
  void WS_TrainData(char data[14]);

  struct s_opc_AddNewCartolib;
  struct s_opc_AddNewEnginetolib;
  struct s_opc_AddNewTraintolib;

  void WS_cts_AddCartoLib(struct s_opc_AddNewCartolib * data, struct web_client_t * client);
  void WS_cts_Edit_Car(Cars * C, struct s_opc_AddNewCartolib * data, struct web_client_t * client);
  
  void WS_cts_AddEnginetoLib(struct s_opc_AddNewEnginetolib * data, struct web_client_t * client);
  void WS_cts_Edit_Engine(struct s_opc_EditEnginelib * msg, struct web_client_t * client);

  void WS_cts_AddTraintoLib(struct s_opc_AddNewTraintolib * data, struct web_client_t * client);
  void WS_cts_Edit_Train(Trains * T, struct s_opc_AddNewTraintolib * data, struct web_client_t * client);

  //Track Messages
  void WS_trackUpdate(int Client_fd);
  void WS_SwitchesUpdate(int Client_fd);
  void WS_NewClient_track_Switch_Update(int Client_fd);

  void WS_Track_LayoutDataOnly(int unit, int Client_fd);
  void WS_stc_TrackLayoutRawData(int unit, int Client_fd);

  void WS_reset_switches(int client_fd);

  //General Messages
  void WS_EmergencyStop();
  void WS_ShortCircuit();
  void WS_ClearEmergency();

  void WS_init_Message_List();
  char WS_init_Message(char type);
  void WS_add_Message(uint16_t ID, char length,char data[16]);
  void WS_send_open_Messages(int Client_fd);
  void WS_clear_message(uint16_t ID, char ret_code);
#endif