#define BUFSIZE 1024
#define PORT 21105
#define BASE_IP 0x0002A8C0
//#define PORT 34472

/*
 * error - wrapper for perror
 */
void die(char *msg) {
  perror(msg);
  exit(1);
}

struct UDP_return {
	char    * msg;
	uint8_t   length;
};

uint8_t XOR_Byte(struct UDP_return * Msg){
	uint8_t value = 0;
	for(int i = 4;i<Msg->length;i++){
		value ^= Msg->msg[i];
	}
	return value;
}

void printPacket(char * message){
	int dataLen = message[0] + (message[1] << 8);
	
	printf("Data: ");
	for(int i = 0;i<dataLen;i++){
		printf("%02x ",message[i]);
	}
	printf("\n");
}

void printFPacket(struct UDP_return * rMsg){
	printf("Data: ");
	for(int i = 0;i<rMsg->length;i++){
		printf("%02x ",rMsg->msg[i]);
	}
	printf("\n");
}

int s;

void Z21E_BroadCaster(uint32_t Flag,struct UDP_return * Msg,char delete){
	printf("BroadCaster: ");
	for(int i = 1;i<255;i++){
		if(Clients[i] != 0 && Clients[i]->clientFlags == 1){
			//Connected Client
			printf("C%i",i);
			if((Clients[i]->BroadcastFlags & Flag) != 0){
				//Client has that flag
				printf("F");
				struct sockaddr_in si_other;
				si_other.sin_addr.s_addr |= (Clients[i]->clientID << 24);
				si_other.sin_port = Clients[i]->clientPort;
				int slen = sizeof(si_other);
				sendto(s, Msg->msg, Msg->length, 0, (struct sockaddr*) &si_other, slen);
			}
			printf("\t");
		}
	}
	if(delete == 1){
		memset(Msg->msg,0,Msg->length);
		Msg->length = 0;
	}
}

void Z21_M_LAN_X_LOCO_INFO(uint16_t adr,struct UDP_return * rMsg){
	if(Engines[adr] == 0){
		Engines[adr] = create_Loc(adr);
	}
	struct Loc * T = Engines[adr];
	rMsg->length = 0xE;
	rMsg->msg[0] = 0xE;
	rMsg->msg[2] = 0x40;
	rMsg->msg[4] = 0xEF;
	rMsg->msg[5] = (T->adr & 0x3F00) >> 8; //LocMSB
	rMsg->msg[6] = T->adr & 0xff; //LocLSB
	rMsg->msg[7] = T->fahrstufen; //Fahrstufeninformation
	rMsg->msg[8] = T->speed; //Direction and speed
	rMsg->msg[9] = T->f[0]; //Fahrstufeninformation
	rMsg->msg[0xA] = T->f[1]; //Fahrstufeninformation
	rMsg->msg[0xB] = T->f[2]; //Fahrstufeninformation
	rMsg->msg[0xC] = T->f[3]; //Fahrstufeninformation
	rMsg->msg[0xD] = XOR_Byte(rMsg); //CentralState
	
	return;
}

void Z21E_recv(char * message, struct UDP_return * rMsg,char ClientIP){
	int dataLen = message[0] + (message[1] << 8);
	int Header  = (message[2] << 8) + message[3];
	uint16_t XHeader;
	
	rMsg->length = 0;
	memset(rMsg->msg,0,100);
	
	printf("\n");
	
	if(Header == H_Z21_LAN_X){
		XHeader = (message[4] << 8) + message[5];
		if(XHeader == HX_Z21_LAN_X_GET_VERSION){
			printf("HX_Z21_LAN_X_GET_VERSION\n");
			rMsg->length = 9;
			rMsg->msg[0] = 0x09;
			rMsg->msg[2] = 0x40;
			rMsg->msg[4] = 0x63;
			rMsg->msg[5] = 0x21;
			rMsg->msg[6] = 0x30;
			rMsg->msg[7] = 0x12;
			rMsg->msg[8] = 0x60;
			
			return;
		}
		else if(XHeader == HX_Z21_LAN_X_GET_STATUS){
			printf("HX_Z21_LAN_X_GET_STATUS\n");
			//Return with LAN_X_STATUS_CHANGED
			rMsg->length = 8;
			rMsg->msg[0] = 0x08;
			rMsg->msg[2] = 0x40;
			rMsg->msg[4] = 0x62;
			rMsg->msg[5] = 0x22;
			rMsg->msg[6] = 0x00;
			rMsg->msg[7] = rMsg->msg[5] ^ rMsg->msg[6] ^ rMsg->msg[7];
			
			return;
		}
		else if(XHeader == HX_Z21_LAN_X_SET_TRACK_POWER_OFF){
			printf("HX_Z21_LAN_X_SET_TRACK_POWER_OFF\n");
			//Return with LAN_X_BC_TRACK_POWER_OFF
			rMsg->length = 7;
			rMsg->msg[0] = 0x07;
			rMsg->msg[2] = 0x40;
			rMsg->msg[4] = 0x61;
			rMsg->msg[5] = 0x00;
			rMsg->msg[6] = 0x61;
			
			return;	
		}
		else if(XHeader == HX_Z21_LAN_X_SET_TRACK_POWER_ON){
			printf("HX_Z21_LAN_X_SET_TRACK_POWER_ON\n");
			//Return with LAN_X_BC_TRACK_POWER_ON
			rMsg->length = 7;
			rMsg->msg[0] = 0x07;
			rMsg->msg[2] = 0x40;
			rMsg->msg[4] = 0x61;
			rMsg->msg[5] = 0x01;
			rMsg->msg[6] = 0x60;
			
			return;
		}
		else if(XHeader == HX_Z21_LAN_X_SET_STOP){
			printf("HX_Z21_LAN_X_SET_STOP\n");
			//Return with LAN_X_BC_STOPPED
			rMsg->length = 7;
			rMsg->msg[0] = 0x07;
			rMsg->msg[2] = 0x40;
			rMsg->msg[4] = 0x81;
			rMsg->msg[5] = 0x00;
			rMsg->msg[6] = 0x81;
			
			return;
		}
		else if(XHeader == HX_Z21_LAN_X_GET_FRIMWARE_VERSION){
			printf("HX_Z21_LAN_X_GET_FRIMWARE_VERSION\n");
			rMsg->length = 9;
			rMsg->msg[0] = 0x09;
			rMsg->msg[2] = 0x40;
			rMsg->msg[4] = 0xF3;
			rMsg->msg[5] = 0x0A;
			rMsg->msg[6] = SOFTWARE_V_H;
			rMsg->msg[7] = SOFTWARE_V_L;
			rMsg->msg[8] = rMsg->msg[4]^rMsg->msg[5]^rMsg->msg[6]^rMsg->msg[7];
			
			return;
		}
		else if(XHeader == HX_Z21_LAN_X_GET_LOCO_INFO){
			printf("HX_Z21_LAN_X_GET_LOCO_INFO\t");
			if((message[6] & 0xC0) > 0){
				printf("Loc Address: %i\n",(((message[6] & 0x3F) << 8) + message[7]));
				Z21_M_LAN_X_LOCO_INFO((((message[6] & 0x3F) << 8) + message[7]),rMsg);
			}else{
				printf("Loc Address: %i\n",message[7]);
				Z21_M_LAN_X_LOCO_INFO(message[7],rMsg);
			}

			Z21E_BroadCaster(1,rMsg,0);

			return;
		}
		else if((XHeader & 0xFFF0) == HX_Z21_LAN_X_SET_LOCO_DRIVE){
			printf("HX_Z21_LAN_X_SET_LOCO_DRIVE\t");
			uint16_t adr;
			if((message[6] & 0xC0) > 0){
				adr = ((message[6] & 0x3F) << 8) + message[7];
			}else{
				adr = message[7];
			}
			printf("Loc Address: %i\n",adr);
			
			if(Engines[adr] == 0){
				Engines[adr] = create_Loc(adr);
			}
			Engines[adr]->fahrstufen = (Engines[adr]->fahrstufen & 0xF8) + (message[5] & 0x7);
			printf("Old speed:\t%i\t",Engines[adr]->speed & 0x7F);
			Engines[adr]->speed = message[8];
			printf("New speed:\t%i\n",Engines[adr]->speed & 0x7F);
			
			Z21_M_LAN_X_LOCO_INFO(adr,rMsg);

			Z21E_BroadCaster(1,rMsg,0);
			
			return;
		}
		else if(XHeader == HX_Z21_LAN_X_SET_LOCO_FUNCTION){
			printf("HX_Z21_LAN_X_SET_LOCO_FUNCTION\t");
			printPacket(message);
			uint16_t adr;
			if((message[6] & 0xC0) > 0){
				adr = ((message[6] & 0x3F) << 8) + message[7];
			}else{
				adr = message[7];
			}
			printf("Loc Address: %i\n",adr);
			
			if(Engines[adr] == 0){
				Engines[adr] = create_Loc(adr);
			}
			char function_adr = message[8] & 0x3F;
			char offset = 0;
			char bit = 0;
			if(function_adr == 0){
				bit = 4;
			}else if(function_adr == 2){
				bit = 1;
			}else if(function_adr == 3){
				bit = 2;
			}else if(function_adr == 4){
				bit = 3;
			}else if(function_adr >= 5 && function_adr <= 12){
				offset = 1;
				bit = function_adr - 5;
			}else if(function_adr >=13 && function_adr <= 20){
				offset = 2;
				bit = function_adr - 13;
			}else if(function_adr >= 21 && function_adr <= 28){
				offset = 3;
				bit = function_adr - 21;
			}
			
			if(((message[8] & 0xC0) >> 6) == 0){ //Off
				printf("OFF");
				Engines[adr]->f[offset] &= (0xFF ^ (1<< bit));
			}
			else if(((message[8] & 0xC0) >> 6) == 1){ //On
				printf("ON");
				Engines[adr]->f[offset] |= (1<< bit);
			}
			else if(((message[8] & 0xC0) >> 6) == 2){ //Toggle
				printf("TOGGLE");
				Engines[adr]->f[offset] ^= (1<< bit);
			}
			
			Z21_M_LAN_X_LOCO_INFO(adr,rMsg);

			Z21E_BroadCaster(1,rMsg,0);
			
			return;
		}
		else if(XHeader == HX_Z21_LAN_X_GET_TURNOUT_INFO){
			printf("HX_Z21_LAN_X_GET_TURNOUT_INFO\t");
			if((message[6] & 0xC0) > 0){
				printf("Turnout Address: %i\n",((message[6] << 8) + message[7]));
			}else{
				printf("Turnout Address: %i\n",message[7]);
			}
			
			rMsg->length = 0x9;
			rMsg->msg[0] = 0x9;
			rMsg->msg[2] = 0x40;
			rMsg->msg[4] = 0x43; //Fahrstufeninformation
			rMsg->msg[5] = message[5]; //LocMSB
			rMsg->msg[6] = message[6]; //LocLSB
			rMsg->msg[0xB] = XOR_Byte(rMsg); //CentralState
			
			return;
		}	
		else if((XHeader & 0xFFF0) == HX_Z21_LAN_X_SET_TURNOUT){
			printf("HX_Z21_LAN_X_SET_TURNOUT\t");
			if((message[6] & 0xC0) > 0){
				printf("Turnout Address: %i\n",(((message[6] & 0x3F) << 8) + message[7]));
			}else{
				printf("Turnout Address: %i\n",message[7]);
			}
			
			rMsg->length = 0xC;
			rMsg->msg[0] = 0xC;
			rMsg->msg[2] = 0x40;
			rMsg->msg[3] = message[6]; //LocMSB
			rMsg->msg[4] = message[7]; //LocLSB
			rMsg->msg[5] = 0x04; //Fahrstufeninformation
			rMsg->msg[6] = 0xCF; //Direction and speed
			rMsg->msg[7] = 0x00; //Fahrstufeninformation
			rMsg->msg[8] = 0x00; //Fahrstufeninformation
			rMsg->msg[9] = 0x00; //Fahrstufeninformation
			rMsg->msg[0xA] = 0x00; //Fahrstufeninformation
			rMsg->msg[0xB] = XOR_Byte(rMsg); //CentralState
			
			return;
		}
		printf("XHeader:\t%04x\n",XHeader);
	}
	else if(Header == H_Z21_LAN_SET_BROADCASTFLAGS){
		printf("H_Z21_LAN_SET_BROADCASTFLAGS\n");
		//No return message
		//copy 32 bit number in little endian
		memcpy(&Clients[ClientIP]->BroadcastFlags,&message[4], sizeof 4);

		return;
	}
	else if(Header == H_Z21_LAN_GET_BROADCASTFLAGS){
		printf("H_Z21_LAN_GET_BROADCASTFLAGS\n");
		uint32_t Flags = 0x00000001;
		rMsg->length = 8;
		rMsg->msg[0] = 0x08;
		rMsg->msg[2] = 0x51;
		memcpy(&rMsg->msg[4], &Flags, sizeof 4);
		rMsg->msg[8] = rMsg->msg[4]^rMsg->msg[5]^rMsg->msg[6]^rMsg->msg[7];
	}
	else if(Header == H_Z21_LAN_SYSTEMSTATE_GETDATA){
		printf("H_Z21_LAN_SYSTEMSTATE_GETDATA\n");
		//Return with H_Z21_LAN_SYSTEMSTATE_DATACHANGED
		int16_t MainCurrent = 0x0000;
		int16_t ProgCurrent = 0x0000;
		int16_t FilteredMainCurrent = 0x0000;
		int16_t Temperature = 0x0000;
		uint16_t SupplyVoltage = 0x0000;
		uint16_t VCCVoltage = 0x0000;
		rMsg->length = 0x14;
		rMsg->msg[0] = 0x14;
		rMsg->msg[2] = 0x51;
		memcpy(&rMsg->msg[4], &MainCurrent, sizeof 2);
		memcpy(&rMsg->msg[6], &ProgCurrent, sizeof 2);
		memcpy(&rMsg->msg[8], &FilteredMainCurrent, sizeof 2);
		memcpy(&rMsg->msg[0xA], &Temperature, sizeof 2);
		memcpy(&rMsg->msg[0xC], &SupplyVoltage, sizeof 2);
		memcpy(&rMsg->msg[0xE], &VCCVoltage, sizeof 2);
		rMsg->msg[0x10] = 0; //CentralState
		rMsg->msg[0x11] = 0; //CentralStateEx
		//rMsg->msg[0x13] = rMsg->msg[4]^rMsg->msg[5]^rMsg->msg[6]^rMsg->msg[7];
	}
	else if(Header == H_Z21_LAN_GET_HWINFO){
		printf("H_Z21_LAN_GET_HWINFO\n");
		uint32_t HW = HW_Type;
		uint32_t FW = FW_Version;
		rMsg->length = 0x0C;
		rMsg->msg[0] = rMsg->length;
		rMsg->msg[2] = 0x40;
		memcpy(&rMsg->msg[4], &HW, sizeof 4);
		memcpy(&rMsg->msg[8], &FW, sizeof 4);
		//rMsg->msg[0x0B] = rMsg->msg[4]^rMsg->msg[5]^rMsg->msg[6]^rMsg->msg[7]^
		//					rMsg->msg[8]^rMsg->msg[9]^rMsg->msg[0x0A]^rMsg->msg[0x0B];
		
		return;
	}
	else if(Header == H_Z21_LAN_GET_LOCOMODE){
		printf("H_Z21_LAN_GET_LOCOMODE\n");
		rMsg->length = 0x07;
		rMsg->msg[0] = rMsg->length;
		rMsg->msg[2] = 0x60;
		rMsg->msg[4] = message[4];
		rMsg->msg[5] = message[5];
		rMsg->msg[6] = 0;
		//rMsg->msg[0x0B] = rMsg->msg[4]^rMsg->msg[5]^rMsg->msg[6]^rMsg->msg[7]^
		//					rMsg->msg[8]^rMsg->msg[9]^rMsg->msg[0x0A]^rMsg->msg[0x0B];
		
		return;
	}
	else if(Header == H_Z21_LAN_SET_TURNOUTMODE){
		printf("H_Z21_LAN_SET_TURNOUTMODE\n");
		return;
	}
	else if(Header == H_Z21_LAN_GET_TURNOUTMODE){
		printf("H_Z21_LAN_GET_TURNOUTMODE\n");
		rMsg->length = 0x07;
		rMsg->msg[0] = rMsg->length;
		rMsg->msg[2] = 0x60;
		rMsg->msg[4] = message[4];
		rMsg->msg[5] = message[5];
		rMsg->msg[6] = 0;
		//rMsg->msg[0x0B] = rMsg->msg[4]^rMsg->msg[5]^rMsg->msg[6]^rMsg->msg[7]^
		//					rMsg->msg[8]^rMsg->msg[9]^rMsg->msg[0x0A]^rMsg->msg[0x0B];
		
		return;
	}
	else if(Header == H_Z21_LAN_SET_LOCOMODE){
		printf("H_Z21_LAN_SET_LOCOMODE\n");
		return;
	}

	printf("A unkown message is recieved :(\n\n");
	//printf("Data: %02x %02x %02x %02x \n",message[0],message[1],message[2],message[3]);
	printf("DataLen:\t%i\n",dataLen);
	printf("Header: \t%i\n\n",Header);
	
	printPacket(message);
}

void * server() {
	//Client List
	memset(Clients,0,255);

    struct sockaddr_in si_me, si_other;
     
    int i, slen = sizeof(si_other) , recv_len;
    char * buf = (char *)malloc(BUFSIZE);
     
    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }
     
    //keep listening for data
    while(1)
    {
        //printf("Waiting for data...");
        fflush(stdout);
         
        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *) &si_other, &slen)) == -1)
        {
            die("recvfrom()");
        }
         
        //print details of the client/peer and the data received
        //printf("Received packet from %i\n", (char)(si_other.sin_addr.s_addr >> 24), ntohs(si_other.sin_port));
        char IP_Addr = (si_other.sin_addr.s_addr >> 24);
        if(Clients[IP_Addr] == 0){
        	printf("New Client %i:%i\n",IP_Addr,si_other.sin_port);
        	Clients[IP_Addr] = (struct UDP_Client *)malloc(sizeof(struct UDP_Client));
        	Clients[IP_Addr]->clientID = IP_Addr;
        	Clients[IP_Addr]->clientPort = si_other.sin_port;
        	Clients[IP_Addr]->clientFlags = 1; //Connected
        	Clients[IP_Addr]->BroadcastFlags = 0;
        }
        if(Clients[IP_Addr]->clientFlags == 2){ //Lost-connection / Timeout
        	printf("Client reconnects\n");
        	Clients[IP_Addr]->clientFlags = 1;
        }
        //printf("Data: %02x %02x %02x %02x\n" , buf[0], buf[1], buf[2], buf[3]);
		struct UDP_return rMsg;
		rMsg.msg = (char *)malloc(100);
		while(1){
			Z21E_recv(buf,&rMsg,IP_Addr);
			
			
			//now reply the client with the same data
			if(rMsg.length != 0){
				//printFPacket(&rMsg);
				if (sendto(s, rMsg.msg, rMsg.length, 0, (struct sockaddr*) &si_other, slen) == -1)
				{
					die("sendto()");
				}
			}
			if(rMsg.msg[rMsg.length] != 0){
				buf = &(rMsg.msg[rMsg.length]);
			}else{
				break;
			}
		}
		free(rMsg.msg);
    }
 
    close(s);
    return 0;
}
