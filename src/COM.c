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
	//Check if the filestream is open
	if(uart0_filestream == -1){
		return 0;
	}
	int index = 0;

	//Create buffer and clear it
	unsigned char data_buffer[256] = {0};
	memset(data_buffer,0,256);
	while(1){
		// Read up to 255 characters from the port if they are there
    	unsigned char rx_buffer[255];

    	//Filestream, buffer to store in, number of bytes to read (max)
		int rx_length = read(uart0_filestream, (void*)rx_buffer, 255);
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

void COM_set_Output(int M){
	uint8_t * OutRegs   = (uint8_t *)malloc(((Units[M]->Out_length-1)/8)+1);
	uint8_t * BlinkMask = (uint8_t *)malloc(((Units[M]->Out_length-1)/8)+1);
	uint8_t * PulseMask = (uint8_t *)malloc(((Units[M]->Out_length-1)/8)+1);
	char Out = 0,Blink = 0,Pulse = 0;
	uint8_t byte,offset;

	for(int i = 0;i<Units[M]->S_L;i++){
		for(int j = 0;j<Units[M]->Signals[i]->length;j++){
			byte   = Units[M]->Signals[i]->adr[Units[M]->Signals[i]->state&0x3F] / 8;
			offset = Units[M]->Signals[i]->adr[Units[M]->Signals[i]->state&0x3F] % 8;
			if(Units[M]->Signals[i]->states[Units[M]->Signals[i]->state&0x3F] == (1 << j)){
				OutRegs[byte] |= (1 << offset);
				Out++;
			}
			if(Units[M]->Signals[i]->flash[Units[M]->Signals[i]->state&0x3F] == (1 << j)){
				BlinkMask[byte] |= (1 << offset);
				Blink++;
			}
			Units[M]->Signals[i]->state &= 0x3F;
		}
	}

	for(int i = 0;i<Units[M]->Swi_nr;i++){
		//for(int j = 0;j<Units[M]->S[i]->length;j++){
		if(Units[M]->S[i]->len & 0xC0 == 0){ //Pulse Address
			Units[M]->S[i]->state &= 0x3F;
			byte   = Units[M]->S[i]->Out[Units[M]->S[i]->state & 0x3F] / 8;
			offset = Units[M]->S[i]->Out[Units[M]->S[i]->state & 0x3F] % 8;
			PulseMask[byte] |= (1 << offset);
			Pulse++;
		}else if(Units[M]->S[i]->len & 0xC0 == 0x40){// Hold a single Address--------------------------------------------------------------------------- Roadmap: Toggle outputs, pulse multiple
			Units[M]->S[i]->state &= 0x3F;
			byte   = Units[M]->S[i]->Out[Units[M]->S[i]->state & 0x3F] / 8;
			offset = Units[M]->S[i]->Out[Units[M]->S[i]->state & 0x3F] % 8;
			OutRegs[byte] |= (1 << offset);
			Out++;
		}
	}

	if(Out > 0){
		printf("Set All Out Addresses:\n");
		struct COM_t TxPacket;
		TxPacket.data[0] = 0x14;
		TxPacket.data[1] = (Units[M]->Out_length/8)+4;
		for(int i = 0;i<(Units[M]->Out_length/8)+1;i++){
			TxPacket.data[2+i] = OutRegs[i];
		}
		for(int i = 0;i<(Units[M]->Out_length/8)+4;i++){
			printf("%02X ",TxPacket.data[i]);
		}
		printf("\n");
		memcpy(Units[M]->OutRegs,OutRegs,((Units[M]->Out_length-1)/8)+1);
	}
	if(Blink > 0){
		printf("Set Blink Mask:\n");
		struct COM_t TxPacket;
		TxPacket.data[0] = 0x15;
		TxPacket.data[1] = (Units[M]->Out_length/8)+4;
		for(int i = 0;i<(Units[M]->Out_length/8)+1;i++){
			TxPacket.data[2+i] = BlinkMask[i];
		}
		for(int i = 0;i<(Units[M]->Out_length/8)+4;i++){
			printf("%02X ",TxPacket.data[i]);
		}
		printf("\n");
		memcpy(Units[M]->BlinkMask,BlinkMask,((Units[M]->Out_length-1)/8)+1);
	}
	if(Pulse > 0){
		printf("Set Pulse Mask: \n");

	}
}

void COM_change_Output(int M){
	uint8_t * OutRegs   = (uint8_t *)malloc(((Units[M]->Out_length-1)/8)+1);
	uint8_t * BlinkMask = (uint8_t *)malloc(((Units[M]->Out_length-1)/8)+1);
	uint8_t * PulseMask = (uint8_t *)malloc(((Units[M]->Out_length-1)/8)+1);

	memcpy(OutRegs  ,Units[M]->OutRegs  ,((Units[M]->Out_length-1)/8)+1);
	memcpy(BlinkMask,Units[M]->BlinkMask,((Units[M]->Out_length-1)/8)+1);

	char Out = 0,Blink = 0,Pulse = 0, Toggle = 0;

	uint8_t * PulseAdr = (uint8_t *)malloc(1);
	uint8_t * BlinkAdr = (uint8_t *)malloc(1);
	uint8_t * ToggleAdr = (uint8_t *)malloc(1);

	uint8_t byte,offset;

	for(int i = 0;i<Units[M]->S_L;i++){
		for(int j = 0;j<Units[M]->Signals[i]->length;j++){
			if(Units[M]->Signals[i]->state & 0x80){ //If output needs to be updated
				Units[M]->Signals[i]->state &= 0x3F; //Output state is updated

				byte   = Units[M]->Signals[i]->adr[Units[M]->Signals[i]->state] / 8;
				offset = Units[M]->Signals[i]->adr[Units[M]->Signals[i]->state] % 8;

				if(Units[M]->Signals[i]->states[Units[M]->Signals[i]->state] == (1 << j)){
					//Enable Output
					if(!(OutRegs[byte] & (1<<offset))){
						//if output is not enabled yet, add to address list
						ToggleAdr[Toggle] = Units[M]->Signals[i]->adr[Units[M]->Signals[i]->state];
						Toggle++;
						ToggleAdr = realloc(ToggleAdr,Toggle+1);
					}

					OutRegs[byte] |= (1 << offset);
				}else{
					//Disable Output
					if((OutRegs[byte] & (1<<offset))){
						//if output is still enabled, add to address list
						ToggleAdr[Toggle] = Units[M]->Signals[i]->adr[Units[M]->Signals[i]->state];
						Toggle++;
						ToggleAdr = realloc(ToggleAdr,Toggle+1);
					}

					OutRegs[byte] |= (1 << offset);
				}
				if(Units[M]->Signals[i]->flash[Units[M]->Signals[i]->state] == (1 << j)){
					//Enable blink
					if(!(OutRegs[byte] & (1<<offset))){
						//if blink is not enabled yet, add to address list to enable
						BlinkAdr[Blink] = Units[M]->Signals[i]->adr[Units[M]->Signals[i]->state];
						Blink++;
						BlinkAdr = realloc(BlinkAdr,Blink+1);
					}

					OutRegs[byte] |= (1 << offset);
				}else{
					//Disable Output
					if((OutRegs[byte] & (1<<offset))){
						//if blink is still enabled, add to address list to disable
						BlinkAdr[Blink] = Units[M]->Signals[i]->adr[Units[M]->Signals[i]->state];
						Blink++;
						BlinkAdr = realloc(BlinkAdr,Blink+1);
					}

					OutRegs[byte] |= (1 << offset);
				}
				
			}
		}
	}

	for(int i = 0;i<Units[M]->Swi_nr;i++){
		//for(int j = 0;j<Units[M]->S[i]->length;j++){
		if(Units[M]->S[i]->len & 0xC0 == 0){ //Pulse Address
			if(Units[M]->S[i]->state & 0x80){
				Units[M]->S[i]->state &= 0x3F;
				byte   = Units[M]->S[i]->Out[Units[M]->S[i]->state & 0x3F] / 8;
				offset = Units[M]->S[i]->Out[Units[M]->S[i]->state & 0x3F] % 8;
				PulseMask[byte] |= (1 << offset);
				Pulse++;
			}
		}else if(Units[M]->S[i]->len & 0xC0 == 0x40){// Hold a single Address--------------------------------------------------------------------------- Roadmap: Toggle outputs, pulse multiple
			if(Units[M]->S[i]->state & 0x80){
				Units[M]->S[i]->state &= 0x3F;
				byte   = Units[M]->S[i]->Out[Units[M]->S[i]->state & 0x3F] / 8;
				offset = Units[M]->S[i]->Out[Units[M]->S[i]->state & 0x3F] % 8;
				OutRegs[byte] |= (1 << offset);
				Out++;
			}
		}
	}

	if(Out > 0){
		printf("Set All Out Addresses:\n");
		struct COM_t TxPacket;
		TxPacket.data[0] = 0x14;
		TxPacket.data[1] = (Units[M]->Out_length/8)+4;
		for(int i = 0;i<(Units[M]->Out_length/8)+1;i++){
			TxPacket.data[2+i] = OutRegs[i];
		}
		for(int i = 0;i<(Units[M]->Out_length/8)+4;i++){
			printf("%02X ",TxPacket.data[i]);
		}
		printf("\n");
		memcpy(Units[M]->OutRegs,OutRegs,((Units[M]->Out_length-1)/8)+1);
	}
	if(Blink > 0){
		printf("Set Blink Mask:\n");
		struct COM_t TxPacket;
		TxPacket.data[0] = 0x15;
		TxPacket.data[1] = (Units[M]->Out_length/8)+4;
		for(int i = 0;i<(Units[M]->Out_length/8)+1;i++){
			TxPacket.data[2+i] = BlinkMask[i];
		}
		for(int i = 0;i<(Units[M]->Out_length/8)+4;i++){
			printf("%02X ",TxPacket.data[i]);
		}
		printf("\n");
		memcpy(Units[M]->BlinkMask,BlinkMask,((Units[M]->Out_length-1)/8)+1);
	}
	if(Pulse > 0){
		printf("Set Pulse Mask: \n");

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

void COM_change_switch(int M){
	printf("COM_change_swit");
	if (uart0_filestream != -1){
		printf("ch\n");
		uint8_t * PulseAdr = (uint8_t *)malloc(1);
		uint8_t * ToggleAdr = (uint8_t *)malloc(1);
		char Pulse = 0, Toggle = 0;
		uint8_t byte,offset;	

		for(int i = 0;i<Units[M]->Swi_nr;i++){
			//for(int j = 0;j<Units[M]->S[i]->length;j++){
			printf("Switch: %i\t",Units[M]->S[i]->id);
			if((Units[M]->S[i]->len & 0xC0) == 0){ //Pulse One Addresses
				printf("P%x\t",Units[M]->S[i]->state);
				if((Units[M]->S[i]->state & 0x80) > 0){
					Units[M]->S[i]->state &= 0x3F;
					PulseAdr[Pulse] = Units[M]->S[i]->Out[Units[M]->S[i]->state];
					Pulse++;
					PulseAdr = realloc(PulseAdr,Pulse+1);
				}
			}else{
				printf("Weird Length bit\n")	;
			}// --------------------------------------------------------------------------- Roadmap: Toggle outputs, pulse multiple
			printf("\n");
		}

		printf("%i addresses\n",Pulse);
		
		struct COM_t TxPacket;
		if(Pulse == 1){
			TxPacket.data[0] = 0x11;
		}else if(Pulse > 1){
			TxPacket.data[0] = 0x14;
		}else{
			return;
		}
		TxPacket.data[1] = Pulse+3;
		for(int i = 0;i<Pulse;i++){
			TxPacket.data[i+2] = PulseAdr[i];
		}

		printf("COM Sending: ");
		for(int i = 0;i<(TxPacket.data[1]-1);i++){
			printf("%02X ",TxPacket.data[i]);
		}
		printf("\n\n");
		//Send via UART and get send bytes back
		int count = write(uart0_filestream, TxPacket.data, TxPacket.data[1]-1);
		//Check if all bytes were send
	}
}

void COM_set_train_speed(struct train * T,char speed){
	//printf("");
}
