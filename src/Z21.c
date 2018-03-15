#define Z21_IP "192.168.2.92"
#define Z21_PORT 4129

#define _BSD_SOURCE

/*
    Simple udp client
*/
#include<stdio.h> //printf

#include<string.h> //memset
#include<stdlib.h> //exit(0);
//#include <fcntl.h> // for open
#include <unistd.h> // for close
#include<sys/socket.h>
#include <netinet/in.h>
#include<arpa/inet.h>

#include "./../lib/Z21.h"
#include "./../lib/status.h"
#include "./../lib/trains.h"


char Z21_prio_list[05][30] = {{""}};
char Z21_send_list[10][30] = {{""}};

struct sockaddr_in si_other;
int fd_Z21_server, slen=sizeof(si_other);

void die(char *s)
{
    perror(s);
    exit(1);
}

void Z21_recv(char message[100]);

void Z21_client(){
    int i;
    char buf[BUFLEN];
    char message[BUFLEN];

    if ( (fd_Z21_server=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);

    if (inet_aton(SERVER , &si_other.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    while(1)
    {
        printf("Enter message : ");
        gets(message);

        //send the message
        if (sendto(fd_Z21_server, message, strlen(message) , 0 , (struct sockaddr *) &si_other, slen)==-1)
        {
            die("sendto()");
        }

        //receive a reply and print it
        //clear the buffer by filling null, it might have previously received data
        memset(buf,'\0', BUFLEN);
        //try to receive some data, this is a blocking call
        if (recvfrom(fd_Z21_server, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1)
        {
            die("recvfrom()");
        }

        Z21_recv(buf);
    }

    close(fd_Z21_server);
    return;
}

void Z21_recv(char message[100]){
	int dataLen = message[0] + message[1] << 8;
	int Header  = message[2] + message[3] << 8;

	printf("A message is recieved :)\n\n");

	printf("DataLen:\t%i\n",dataLen);
	printf("Header:\t%i\n\n",Header);

  for(int i = 0;i<96;){
    printf("%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\n",message[i++],message[i++],message[i++],message[i++],message[i++],message[i++],message[i++],message[i++]);
  }
  printf("%i\t%i\t%i\t%i\n",message[96],message[97],message[98],message[99]);
  /*
	for(int i = 0;i<dataLen;i++){
		printf("[%02x]",message[i]);
	}*/

	//All different headers
	if(Header == 0x10 && dataLen >= 8){
		//LAN_GET_SERIAL_NUMBER
		int Serial_number = message[4] + message[5] << 8 + message[6] << 16 + message[7] << 24;
		printf("The Serial number of the Z21-base station is:\n%x\n",Serial_number);
	}
	else if(Header == 0x1A && dataLen >= 12){
		//LAN_GET_HWINFO
    int HW_number = message[4] + message[5] << 8 + message[6] << 16 + message[7] << 24;
    printf("The Hardware number of the Z21-base station is:\n%x\n",HW_number);

    int SW_number = message[8] + message[9] << 8 + message[10] << 16 + message[11] << 24;
		printf("The Software number of the Z21-base station is:\n%x\n",SW_number);
	}
	else if(Header == 0x40 && dataLen >= 5){
    int X_header = message[4];
		if(X_header == 0x43 && dataLen >= 8){
			//LAN_X_TURNOUT_INFO
      int FAdr = message[5] + message[6] << 8;
      printf("TURNOUT INFO\n");
      printf("Function Address:\t%x\n",FAdr);
      printf("Turnout position:\t%x\n\n",message[7] & 3);
		}
    else if(X_header == 0x61){
      if(message[5] == 0x00){
  			//LAN_X_BC_TRACK_POWER_OFF
        printf("BC TRACK POWER OFF\n");
  		}
      else if(message[5] == 0x01){
  			//LAN_X_BC_TRACK_POWER_ON
        printf("BC TRACK POWER ON\n");
  		}
      else if(message[5] == 0x02){
  			//LAN_X_BC_PROGRAMMING_MODE
        printf("BC PROGRAMMING MODE\n");
  		}
      else if(message[5] == 0x08){
  			//LAN_X_BC_TRACK_SHORT_CIRCUIT
        printf("BC TRACK SHORT CIRCUIT\n");
  		}
      else if(message[5] == 0x12){
  			//LAN_X_CV_NACK_SC
        printf("CV Programming Failed (CV NACK SC)\n");
  		}
      else if(message[5] == 0x13){
  			//LAN_X_CV_NACK
        printf("CV No ACKnolegment\n");
      }
      else if(message[5] == 0x82){
  			//LAN_X_UNKNOWN_COMMAND
        printf("UNKOWN COMMAND\n");
  		}
      else if(message[5] == 0x22){
  			//LAN_X_STATUS_CHANGED
        printf("LAN STATUS CHANGED\n");
        printf("Centrale status: 0x%x\n",message[6]);

        /*
        Bitmasken für Zentralenstatus:
        #define csEmergencyStop 0x01 // Der Nothalt ist eingeschaltet
        #define csTrackVoltageOff 0x02 // Die Gleisspannung ist abgeschaltet
        #define csShortCircuit 0x04 // Kurzschluss
        #define csProgrammingModeActive 0x20 // Der Programmiermodus ist aktiv
        */
  		}
      else if(message[5] == 0x21){
  			//LAN_X_GET_VERSION
        printf("X-Bus Version\n");
        printf("\t%x.%x\n",message[6] >> 4,message[6] & 0xF);
  		}
      else if(message[5] == 0x14){
  			//LAN_X_CV_RESULT
        printf("CV Read\n");
        printf("CV Adr:\t%i\n",(message[6] << 8 + message[7]));
        printf("Value:\t%i\n",message[8]);
      }
		}
    else if(X_header == 0x81 && dataLen >= 7){
			//LAN_X_BC_STOPPED
      printf("EMERGENCY STOP // BC_STOPPED\n");
		}
    else if(X_header == 0xEF && dataLen >= 7){
			//LAN_X_LOCO_INFO
      printf("LOCO INFO\n");
      char data[15];
      memset(data,0,14);
      for(int i = 5;i < (dataLen - 1);i++){
        printf("DB%02i:\t%x\n",message[i]);
        data[i-5] = message[i];
      }

      printf("\nLoc Address:     \t%04i",((data[0] & 0x3F) << 8) + data[1]);
      printf("\nX-Bus Hand block:\t%i",data[2] & 0x8);
      printf("\nSpeed step:      \t%03i",data[2] & 0x7);
      printf("\nDirection:       \t%i",data[3] & 0x80);
      printf("\nMM speed:        \t%03i",data[3] & 0x7F);
      printf("\n'Doppeltraktion':\t%i",data[4] & 0x40);
      printf("\n'Smartsearch':   \t%i",data[4] & 0x20);
      printf("\n\nF00:\t%i\tLight",data[4] & 0x10);
      printf("\nF01:\t%i",data[4] & 0x1);
      printf("\nF02:\t%i",data[4] & 0x2);
      printf("\nF03:\t%i",data[4] & 0x4);
      printf("\nF04:\t%i",data[4] & 0x8);
      printf("\nF05:\t%i",data[5] & 0x1);
      printf("\nF06:\t%i",data[5] & 0x2);
      printf("\nF07:\t%i",data[5] & 0x4);
      printf("\nF08:\t%i",data[5] & 0x8);
      printf("\nF09:\t%i",data[5] & 0x10);
      printf("\nF10:\t%i",data[5] & 0x20);
      printf("\nF11:\t%i",data[5] & 0x40);
      printf("\nF12:\t%i",data[5] & 0x80);
      printf("\nF13:\t%i",data[6] & 0x1);
      printf("\nF14:\t%i",data[6] & 0x2);
      printf("\nF15:\t%i",data[6] & 0x4);
      printf("\nF16:\t%i",data[6] & 0x8);
      printf("\nF17:\t%i",data[6] & 0x10);
      printf("\nF18:\t%i",data[6] & 0x20);
      printf("\nF19:\t%i",data[6] & 0x40);
      printf("\nF20:\t%i",data[6] & 0x80);
      printf("\nF21:\t%i",data[7] & 0x1);
      printf("\nF22:\t%i",data[7] & 0x2);
      printf("\nF23:\t%i",data[7] & 0x4);
      printf("\nF24:\t%i",data[7] & 0x8);
      printf("\nF25:\t%i",data[7] & 0x10);
      printf("\nF26:\t%i",data[7] & 0x20);
      printf("\nF27:\t%i",data[7] & 0x40);
      printf("\nF28:\t%i",data[7] & 0x80);
      printf("\n\nExtra Data\n08:\t");
      for(int j = 8;j<14;j){
        printf(BYTE_TO_BINARY_PATTERN,BYTE_TO_BINARY(data[j++]));
        printf("\n%02i\t",j);
      }
      WS_TrainData(data);

      printf("\n");
		}
    else if(X_header == 0xF3 && message[5] == 0x0A){
			//LAN_X_GET_FIRMWARE_VERSION
      printf("FIRMWARE VERSION\n");
      printf("v%x.%x\n",message[6],message[7]);
		}
	}
	else if(Header == 0x51 && dataLen >= 8){
		//LAN_GET_BROADCASTFLAGS
    int Flags = message[4] + message[5] << 8 + message[6] << 16 + message[7] << 24;
    printf("BROADCASTFLAGS\t%x\n",Flags);
	}
	else if(Header == 0x60 && dataLen >= 7){
		//LAN_GET_LOCOMODE
    printf("LOCO MODE\n");
    printf("Address:\t%x\n",(message[4] << 8 + message[5]));
    printf("Modues: \t%i\n",message[6]);
	}
	else if(Header == 0x70 && dataLen >= 7){
		//LAN_GET_TURNOUTMODE
    printf("TURNOUT MODE\n");
    printf("Address:\t%x\n",(message[4] << 8 + message[5]));
    printf("Modues: \t%i\n",message[6]);
	}
	else if(Header == 0x80 && dataLen >= 15){
		//LAN_RMBUS_DATACHANGED
    printf("Block detection\n");
    if(message[4] == 0){
      printf("Address 1 to 10\n");
    }

    for(int i = 5;i<(dataLen-1);i++){
      printf("Status detector %02i\t%x\n",(i-5),message[i]);
    }
	}
	else if(Header == 0x84 && dataLen >= 20){
		//LAN_SYSTEMSTATE_DATACHANGED

    int i = 4;
    short MainCurrent = message[i++] + message[i++] << 8;
    short ProgCurrent = message[i++] + message[i++] << 8;
    short FilteredMainCurrent = message[i++] + message[i++] << 8;
    short Temperature = message[i++] + message[i++] << 8;
    unsigned short SupplyVoltage = message[i++] + message[i++] << 8;
    unsigned short VCCVoltage = message[i++] + message[i++] <<8;
    unsigned char CentralState = message[i++];
    unsigned char CentralStateEx = message[i++];

    /*
    Bitmasken für CentralState:
    #define csEmergencyStop 0x01 // Der Nothalt ist eingeschaltet
    #define csTrackVoltageOff 0x02 // Die Gleisspannung ist abgeschaltet
    #define csShortCircuit 0x04 // Kurzschluss
    #define csProgrammingModeActive 0x20 // Der Programmiermodus ist aktiv
    Bitmasken für CentralStateEx:
    #define cseHighTemperature 0x01 // zu hohe Temperatur
    #define csePowerLost 0x02 // zu geringe Eingangsspannung
    #define cseShortCircuitExternal 0x04 // am externen Booster-Ausgang
    #define cseShortCircuitInternal 0x08 // am Hauptgleis oder Programmiergleis
    */
    printf("MainCurrent\t%imA\n",MainCurrent);
    printf("ProgCurrent\t%imA\n",ProgCurrent);
    printf("FilteredMainCurrent\t%imA\n",FilteredMainCurrent);
    printf("Temperature\t%i C\n",Temperature);
    printf("SupplyVoltage\t%imV\n",SupplyVoltage);
    printf("VCCVoltage\t%imV\n",VCCVoltage);
    printf("CentralState\t%x\n",CentralState);
    printf("CentralStateEx\t%x\n",CentralStateEx);
	}
	else if(Header == 0x88){
		//LAN_RAILCOM_DATACHANGED
    printf("RAILCOM DATACHANGED\nNot supported\n");
	}
	else if(Header == 0xA0){
		//LAN_LOCONET_Z21_RX
    printf("LOCONET RX\nNot supported\n");
	}
	else if(Header == 0xA1){
		//LAN_LOCONET_Z21_TX
    printf("LOCONET TX\nNot supported\n");
	}
	else if(Header == 0xA2){
		//LAN_LOCONET_FROM_LAN
    printf("LOCONET FROM LAN\nNot supported\n");
	}
	else if(Header == 0xA3){
		//LAN_LOCONET_DISPATCH_ADDR
    printf("LOCONET DISPATCH ADDR\nNot supported\n");
	}
	else if(Header == 0xA4){
		//LAN_LOCONET_DETECTOR
    printf("LOCONET DETECTOR\nNot supported\n");
	}
}

void Z21_send(int Header,char data[30]){

}

void Z21_GET_LOCO_INFO(int DCC_Adr){
  /*                  X-Header  DB0   DB1     DB2     XOR-Byte
  0x09 0x00 0x40 0x00 0xE3      0xF0  Adr_MSB Adr_LSB XOR-Byte*/

  if(fd_Z21_server != 0){
    char data[10];
    data[0] = 9;
    data[2] = 0x40;
    data[4] = 0xE3;
    data[5] = 0xF0;
    data[6] = (DCC_Adr & 0x3F00) >> 8;
    data[7] = DCC_Adr & 0xFF;
    data[8] = data[4] ^ data[5] ^ data[6] ^ data[7];


    if (sendto(fd_Z21_server, data, data[0], 0 , (struct sockaddr *) &si_other, slen)==-1){
        printf("failed sendto()\n");
    }
  }else{
    printf("No Z21 server connected\n");
    char data[10];
	memset(data,0,15);
	//Loc-Address
    data[0] = (DCC_Adr & 0x3F00) >> 8;
    data[1] = DCC_Adr & 0xFF;
	//X-Bus Handler 0x8, Speedstep 0x7
    data[2] = 0b00000100;
	//Direction 0x80, Speed 0x7F
    data[3] = (DCC_train[DCC_Adr]->dir << 7) + (DCC_train[DCC_Adr]->cur_speed & 0x7F);
	//Doppeltraktion 0x40, smartsearch 0x20, F0 0x10, F1 0x1, F2 0x2, F3 0x4, F4 0x8
    data[4] = 0b00000000;
	// F12 F11 F10  F9  F8  F7  F6  F5
    data[5] = 0b00000000;
	// F20 F19 F18 F17  F16 F15 F14 F13
    data[6] = 0b00000000;
	// F28 F27 F26 F25  F24 F23 F22 F21
    data[7] = 0b00000000;
    printf("DCC_Adr:0x%x\t=>\t%x %x",DCC_Adr,data[0],data[1]);
    WS_TrainData(data);
  }
}

void Z21_SET_LOCO_DRIVE(int DCC_Adr,char steps,_Bool dir,char drive){
  /*                  X-Header  DB0   DB1     DB2     DB3       XOR-Byte
  0x0A 0x00 0x40 0x00 0xE4      0x1S  Adr_MSB Adr_LSB RVVVVVVV  XOR-Byte*/

  if(fd_Z21_server != 0){
    char data[10];
    data[0] = 0x0A;
    data[2] = 0x40;
    data[4] = 0xE4;
    data[5] = 0x10;
    if(steps == 28){
      data[5] += 0x2;
    }else if(steps == 128){
      data[5] += 0x3;
    }
    data[6] = (DCC_Adr & 0x3F00) >> 8;
    data[7] = DCC_Adr & 0xFF;
    data[8] = (dir << 7) + (drive & 0x7F);
    data[9] = data[4] ^ data[5] ^ data[6] ^ data[7] ^ data[8];


    if (sendto(fd_Z21_server, data, data[0], 0 , (struct sockaddr *) &si_other, slen)==-1)
    {
        printf("Failed sendto()\n");
    }
  }else{
    printf("No Z21 server connected\n");
  }
}

void Z21_SET_LOCO_FUNCTION(int DCC_Adr,char function_nr,char switch_type){
  /*                  X-Header  DB0   DB1     DB2     DB3     XOR-Byte
  0x0A 0x00 0x40 0x00 0xE4      0xF8  Adr_MSB Adr_LSB TTNNNNN XOR-Byte*/

  if(fd_Z21_server != 0){
    char data[10];
    data[0] = 0x0A;
    data[2] = 0x40;
    data[4] = 0xE4;
    data[5] = 0xF8;
    data[6] = (DCC_Adr & 0x3F00) >> 8;
    data[7] = DCC_Adr & 0xFF;
    data[8] = (switch_type << 6) + (function_nr & 0x3F);
    data[9] = data[4] ^ data[5] ^ data[6] ^ data[7] ^ data[8];


    if (sendto(fd_Z21_server, data, data[0], 0 , (struct sockaddr *) &si_other, slen)==-1)
    {
        printf("Failed sendto()\n");
    }
  }else{
    printf("No Z21 server connected\n");
  }
}
