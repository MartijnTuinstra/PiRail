#ifndef _INCLUDE_Z21_MSG_H
#define _INCLUDE_Z21_MSG_H

#if !defined(packedstruct)
#define packedstruct struct __attribute__((__packed__))
#endif

// packedstruct Z21_LAN_GET_SERIAL_NUMBER {
//   uint32_t serialnumber;
// };

// // 0x40 - X 0x21
// // 0x40 - X 0x21
// packedstruct LAN_X_GET_VERSION {
//   uint8_t XHeader;
//   uint8_t DB0;
//   uint8_t checksum;
// };

// // 0x40 - X 0x63
// packedstruct LAN_X_GET_VERSION_RESPONSE {
//   uint8_t XHeader;
//   uint8_t DB[3];
//   uint8_t checksum;
// };

// // 0x40 - X 0x21
// packedstruct LAN_X_GET_STATUS {
//   uint8_t XHeader;
//   uint8_t DB0;
//   uint8_t checksum;
// };




packedstruct Z21_Message_Structure {
  uint8_t lenLow;
  uint8_t lenHigh;
  uint8_t headerLow;
  uint8_t headerHigh;

  uint8_t raw[1];
};


void Z21_Set_EmergencyStop();
void Z21_Release_EmergencyStop();

void Z21_Set_Loco_Drive_Engine(Engine * E);
void Z21_Set_Loco_Drive_Train(Train * T);

void Z21_Set_Emergency();

void Z21_LAN_X_LOCO_INFO(uint8_t length, char * data);

void Z21_get_train(RailTrain * RT);
void Z21_get_loco_info(uint16_t DCC_id);

void Z21_setLocoFunction(Engine * E, uint8_t function, uint8_t type);

void Z21_KeepAliveFunc(void * args);
#endif