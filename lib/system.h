
#ifndef _INCLUDE_system_H
	#define _INCLUDE_system_H

	struct systemState{
		uint16_t _STATE;
		uint16_t _Clients;
		int _COM_fd;
	};

	struct adr{
		int M;		// Module
		int B;		// Block
		int S;		// Section
		int type;	// Type
	};

	void _SYS_change(int STATE,char send);

	extern struct systemState * _SYS;	

	#ifndef TRUE
		#define FALSE 0
		#define TRUE  1
	#endif

	#define MAX_A MAX_Modules*MAX_Blocks*MAX_Segments
	#define MAX_Devices 20

	#define TRACK_SCALE 160

	#define GREEN 0
	#define AMBER 1
	#define RED 2
	#define BLOCKED 3
	#define PARKED 4
	#define RESERVED 5
	#define UNKNOWN 6
	#define BLOCK_STATES 4

	#define STATE_Z21_FLAG        0x8000
	#define STATE_WebSocket_FLAG  0x4000
	#define STATE_COM_FLAG        0x2000
	#define STATE_Client_Accept   0x1000

	#define STATE_TRACK_DIGITAL   0x0200
	#define STATE_RUN             0x0100

	#define STATE_Modules_Loaded  0x0002
	#define STATE_Modules_Coupled 0x0004
#endif