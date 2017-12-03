#define Serial_Port "/dev/ttyAMA0"
#define Serial_Baud B115200

pthread_mutex_t mutex_UART;

//------------COM PROTOCOL------------//
//
//Packet data
//Adr:		8bit
//Length:	5bit
//OpCode: 3bit
//Data:		8bit //Length times Repeated
//
//Bit size:		8				4				4				8
//					{Adr} {Lenght}{OpCode} {Data} ...
//
//Special Adresses
//	0   = Master / Computer
//	255 = Broadcast
//
//Opcodes
//	0  = My adress is {Type}
//	1  = Reset adress line
//	2  = {Empty}
//
//	3  = Block data
//	4  = Switch Request
//	5  = {Empty}
//
//	6  = Get block data
//	7  = Set All Switches (Max 64 switches)
//	8  = Set 1 Switch {Loc}{Data}
//	9  = Set All Accessoires {Max 32 type 3 signals}
//	10 = Set 1 Accessoire {Loc}{Data}
//	11 = {Empty}
//	12 = {Empty}
//	13 = {Empty}
//	14 = {Empty}
//	15 = {Empty}
//
//Opcodes 3-5 block transmission

int uart0_filestream = -1;

void * UART(){

	//OPEN THE UART
	uart0_filestream = open(Serial_Port, O_RDWR | O_NOCTTY);
	if (uart0_filestream == -1)
	{
		//ERROR - CAN'T OPEN SERIAL PORT
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
	}

	//CONFIGURE THE UART
	struct termios options;
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = B38400 | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);

	while(!stop){
		usleep(1000000);
	}

  //----- CLOSE THE UART -----
	close(uart0_filestream);
}

char * COM_Send(struct COM_t DATA){
	if (uart0_filestream == -1){
		return "No UART";
	}

	char out[40];

	out[0] = DATA.Adr;
	out[1] = DATA.Length << 4;
	out[1] += DATA.Opcode & 0b1111;
	for(int i = 0;i<DATA.Length;i++){
		out[i+2] = DATA.Data[i];
	}
	//printf("Sending: \"%s\", and length is: %i\n",out,(DATA.Length + 2));
	//return buf;

	tcflush(uart0_filestream, TCIFLUSH);

	digitalWrite(0,HIGH);
	int count = write(uart0_filestream, &out[0], (DATA.Length + 2));		//Filestream, bytes to write, number of bytes to write
	if (count < 0)
	{
		printf("UART TX error\n");
	}else{
		//printf("Count: %i\n",count);
	}
	tcdrain(uart0_filestream);
	digitalWrite(0,LOW);
}

int COM_Recv(char * OUT_Data){
	//printf(buf);
	//return buf;
	if(uart0_filestream == -1){
		return;
	}
  unsigned char data_buffer[256] = {0};
	int index = 0;
	memset(data_buffer,0,256);
  while(1){
		// Read up to 255 characters from the port if they are there
    unsigned char rx_buffer[255];
		int rx_length = read(uart0_filestream, (void*)rx_buffer, 255);		//Filestream, buffer to store in, number of bytes to read (max)
		if (rx_length < 0)
		{
			//An error occured (will occur if there are no bytes)
		}
		else if (rx_length == 0)
		{
			//No data waiting
		}
		else if (rx_length == 8)
		{
      rx_buffer[rx_length] = '\0';
      //printf("%i bytes read : %s\n", rx_length, rx_buffer);
			for(int i = 0;i<8;i++){
				data_buffer[index++] = rx_buffer[i];
			}
		}
		else
		{
			//Bytes received
			rx_buffer[rx_length] = '\0';
			//printf("%i bytes read : %s\n", rx_length, rx_buffer);
			for(int i = 0;i<rx_length;i++){
				data_buffer[index++] = rx_buffer[i];
			}
      break;
		}
		return index;
  }

	for(int i = 0;i<index;i++){
		OUT_Data[i] = data_buffer[i];
	}
}

char * COM_SaR(char * buf[60]){

}

void COM_change_A_signal(int M){
	if (uart0_filestream != -1){

		struct COM_t C;
		memset(C.Data,0,32);
		C.Adr = M;
		C.Opcode = 0b1001;

		int nr_signals = Units[M]->Si_L;

		int location = 0;
		int position = 0;
		int Signal_Adr = 0;
		int empty = 0;

		for(int i = 0;i<=nr_signals;i++){
			if(Units[M]->Signals[i] != NULL){
				printf("Signal found on 0%o\tRegister: %i\tPosition %i\tState: %i\n",Units[M]->Signals[i]->UAdr,location,position,Units[M]->Signals[i]->state);
				C.Data[location] += Units[M]->Signals[i]->state << position;
				position += (Units[M]->Signals[i]->type+1);
			}else{
				printf("No Signal\t\tRegister: %i\tPosition %i\n",location,position);
				position += 3;
				empty++;
			}

			if(Units[M]->Signals[i+1] != NULL && position >= (8 - Units[M]->Signals[i+1]->type - 1)){
				position = 0;
				if(empty != 2){
					location++;
					Signal_Adr++;
				}
				empty = 0;
			}else if(position >= 5){
				position = 0;
				if(empty != 2){
					location++;
					Signal_Adr++;
				}
				empty = 0;
			}
		}

		Signal_Adr += 1;

		printf("Binary data: ");
		for(int i = 0;i<Signal_Adr;i++){
			printf(BYTE_TO_BINARY_PATTERN,BYTE_TO_BINARY(C.Data[i]));
			printf(" ");
		}
		printf("\n");
		C.Length = Signal_Adr;

		pthread_mutex_lock(&mutex_UART);
		COM_Send(C);
		pthread_mutex_unlock(&mutex_UART);
	}
}

void COM_change_A_switch(int M){
	if (uart0_filestream != -1){

		struct COM_t C;
		memset(C.Data,0,32);
		C.Adr = M;
		C.Opcode = 0b0111;

		int nr_signals = Units[M]->S_L;

		int location = 0;
		int position = 0;
		int Switch_Adr = 0;
		int empty = 0;

		for(int i = 0;i<=nr_signals;i++){
			if(Units[M]->S[i] != NULL){
				printf("Switch found on 0%o\tlocation: %i\tPosition %i\n",Units[M]->S[i]->UAdr,location+2,position);
				if(Units[M]->S[i]->state == 0){
					C.Data[location] += 1 << position;
				}else{
					C.Data[location] += 2 << position;
				}
				position += 2;
			}else{
				printf("No Switch\t\tlocation: %i\tPosition %i\n",location+2,position);
				position += 2;
				empty++;
			}

			if(position >= 7){
				position = 0;
				if(empty != 4){
					location++;
					Switch_Adr++;
				}
				empty = 0;
			}
		}

		Switch_Adr += 1;

		//printf("out[1]=%i\t%i bytes Signals\t%i\n",out[1],Signal_Adr,Signal_Adr << 4);
		C.Length = Switch_Adr;

		pthread_mutex_lock(&mutex_UART);
		COM_Send(C);
		pthread_mutex_unlock(&mutex_UART);
	}
}

void COM_change_signal(struct signal * Si){
	if (uart0_filestream != -1){
		COM_change_A_signal(Si->MAdr);/*
		int M = Si->MAdr;

		struct COM_t C;
		memset(C.Data,0,32);
		C.Adr = M;
		C.Opcode = 0b1010;
		C.Length = 2;

		int nr_signals = Units[M]->Si_L;

		int location = 0;
		int position = 0;
		int Signal_Adr = 0;
		int empty = 0;
		int loc = -1;
		char data;

		printf("Finding Signal 0%o\n",Si->id);

		for(int i = 0;i<=nr_signals;i++){
			if(Units[M]->Signals[i] != NULL){
				printf("Signal found on 0%o\tlocation: %i\tPosition %i\n",Units[M]->Signals[i]->UAdr,location+2,position);
				if(Units[M]->Signals[i]->UAdr == Si->UAdr){
					loc = location;
				}
				out[2+location] += Units[M]->Signals[i]->state << position;
				position += (Units[M]->Signals[i]->type+1);
			}else{
				printf("No Signal\t\tlocation: %i\tPosition %i\n",location+2,position);
				position += 3;
				empty++;
			}

			if(Units[M]->Signals[i+1] != NULL && position >= (8 - Units[M]->Signals[i+1]->type - 1)){
				position = 0;
				if(empty == 2){
					printf("empty set\n");
					//nr_signals--;
				}else{
					location++;
					Signal_Adr++;
				}
				if(loc != -1){
					break;
				}
				empty = 0;
			}else if(position >= 5){
				position = 0;
				if(empty == 2){
					printf("empty set\n");
					//nr_signals--;
				}else{
					location++;
					Signal_Adr++;
				}
				if(loc != -1){
					break;
				}
				empty = 0;
			}
		}

		out[3] = out[loc+2];
		out[2] = loc;

		printf("Sending: [%i][%i]",out[0],out[1]);
		for(int i = 0;i<2;i++){
			printf("[%i]",out[i+2]);
		}
		printf("\n\n");
		int count = write(uart0_filestream, &out[0], 4);		//Filestream, bytes to write, number of bytes to write*/
	}
}

void COM_change_switch(struct Swi * S){
	if (uart0_filestream != -1){
		int M = S->Module;

		int nr_signals = Units[M]->Swi_nr;

		char out[15];

		memset(out,0,15);

		out[0] = M;
		out[1] = 0b101000;

		int location = 0;
		int position = 0;
		int Signal_Adr = 0;
		int empty = 0;
		int loc = -1;
		char data;

		printf("Finding Signal 0%o\n",S->UAdr);

		for(int i = 0;i<=nr_signals;i++){
			if(Units[M]->S[i] != NULL){
				printf("Switch found on 0%o\tlocation: %i\tPosition %i\n",Units[M]->S[i]->UAdr,location+2,position);
				if(Units[M]->S[i]->UAdr == S->UAdr){
					loc = location;
				}
				if(Units[M]->S[i]->state == 0){
					out[2+location] += 1 << position;
				}else{
					out[2+location] += 2 << position;
				}
				position += 2;
			}else{
				printf("No Switch\t\tlocation: %i\tPosition %i\n",location+2,position);
				position += 2;
				empty++;
			}

			if(position >= 7){
				position = 0;
				if(empty != 4){
					location++;
					Signal_Adr++;
				}
				if(loc != -1){
					break;
				}
				empty = 0;
			}
		}

		out[3] = out[loc+2];
		out[2] = loc;

		printf("Sending: [%i][%i]",out[0],out[1]);
		for(int i = 0;i<2;i++){
			printf("[%i]",out[i+2]);
		}
		printf("\n\n");
		int count = write(uart0_filestream, &out[0], 4);		//Filestream, bytes to write, number of bytes to write*
	}
}

void COM_set_train_speed(struct train * T,char speed){
	//printf("");
}
