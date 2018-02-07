#ifndef H_COM
	#define H_COM
	struct train;

	struct COM_t{
		char Adr;
		char Length;
		char Opcode;
		char Data[32];
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
#endif
