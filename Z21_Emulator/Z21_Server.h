#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SOFTWARE_V_H 0x01
#define SOFTWARE_V_L 0x23
#define HW_Type      0x00000201 //D_HWT_Z21_NEW
#define FW_Version   0x00000120

#define  H_Z21_LAN_X             		    0x4000
#define HX_Z21_LAN_X_GET_VERSION  		    0x2121
#define HX_Z21_LAN_X_GET_STATUS             0x2124
#define HX_Z21_LAN_X_SET_TRACK_POWER_OFF    0x2180
#define HX_Z21_LAN_X_SET_TRACK_POWER_ON     0x2181
#define HX_Z21_LAN_X_SET_STOP               0x8080
#define HX_Z21_LAN_X_GET_FRIMWARE_VERSION   0xF10A

#define HX_Z21_LAN_X_BC_TRACK_POWER_OFF     0x6100
#define HX_Z21_LAN_X_BC_TRACK_POWER_ON      0x6101
#define HX_Z21_LAN_X_BC_PROGRAMMING_MODE    0x6102
#define HX_Z21_LAN_X_BC_TRACK_SHORT_CIRCUIT 0x6108
#define HX_Z21_LAN_X_UNKOWN_COMMAND         0x6182
#define HX_Z21_LAN_X_STATUS_CHANGED         0x6182
#define HX_Z21_LAN_X_BC_STOPED              0x8100

#define HX_Z21_LAN_X_GET_LOCO_INFO          0xE3F0
#define HX_Z21_LAN_X_SET_LOCO_DRIVE         0xE410
#define HX_Z21_LAN_X_SET_LOCO_FUNCTION      0xE4F8

#define HX_Z21_LAN_X_LOCO_INFO              0xEF

#define HX_Z21_LAN_X_GET_TURNOUT_INFO       0x4300
#define HX_Z21_LAN_X_SET_TURNOUT            0x5300

#define HX_Z21_LAN_X_TURNOUT_INFO           0x43

#define  H_Z21_LAN_SET_BROADCASTFLAGS       0x5000
#define  H_Z21_LAN_GET_BROADCASTFLAGS       0x5100

#define  H_Z21_LAN_SYSTEMSTATE_DATACHANGED  0x8400

#define  H_Z21_LAN_SYSTEMSTATE_GETDATA      0X8500

#define  H_Z21_LAN_GET_HWINFO               0x1A00

#define  H_Z21_LAN_GET_LOCOMODE             0x6000
#define  H_Z21_LAN_SET_LOCOMODE             0x6100
#define  H_Z21_LAN_GET_TURNOUTMODE          0x7000
#define  H_Z21_LAN_SET_TURNOUTMODE          0x7100

/*
 * error - wrapper for perror
 */
void die(char *msg);

void * server();

struct UDP_Client{
	char clientID;
	char clientPort;
	char clientFlags;
	uint32_t BroadcastFlags;
};

struct UDP_Client * Clients[255];


struct UDP_return;

void Z21E_recv(char * message, struct UDP_return * rMsg,char ClientIP);
