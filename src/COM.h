#ifndef H_COM
	#define H_COM
	struct train;

	struct COM_t{
		char length;
		char data[32];
	};

	void * UART();

	char * COM_Send(struct COM_t DATA);

	int COM_Recv(char * OUT_Data);

	char * COM_SaR(char * buf[60]);

	void COM_change_A_signal(int M);

	void COM_change_signal(struct signal * Si);

	void COM_change_switch(int M);

	void COM_set_train_speed(struct train * T,char speed);


	#define COMopc_ReportID     0x00
	#define COMopc_EmergencyEn  0x01
	#define COMopc_EmergencyDis 0x02
	#define COMopc_PowerON      0x03
	#define COMopc_PowerOFF     0x04
	#define COMopc_RESET        0x05

	#define COMopc_ACK          0x7F

	#define COMopc_TogSinAdr    0x10
	#define COMopc_PulSinAdr    0x11
	#define COMopc_TogBlSinAdr  0x12
	#define COMopc_TogMulAdr	0x13
	#define COMopc_SetAllOut	0x14
	#define COMopc_SetBlOut     0x15
	#define COMopc_PostAllOut	0x16
	#define COMopc_PostBlkMsk	0x17
	#define COMopc_ReqOut_Bl	0x47

	#define COMopc_PostSinAdr	0x06
	#define COMopc_PostMulAdr	0x07
	#define COMopc_PostAllIn	0x08
	#define COMopc_ReqIn		0x4C

	#define COMopc_ChangeDevID	0x50
	#define COMopc_SetIN_OUT	0x51
	#define COMopc_SetBlInter	0x52
	#define COMopc_SetPulseLen	0x53
	#define COMopc_SetInInter	0x54
	#define COMopc_PostEEPROM	0x55
	#define COMopc_ReqEEPROM	0x59
#endif
