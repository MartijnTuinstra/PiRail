import serial
import time

ser = serial.Serial('/dev/serial0', 500000, timeout=0.1)

RNet_OPC_DEV_ID        =0x01
RNet_OPC_SetEmergency  =0x02
RNet_OPC_RelEmergency  =0x03
RNet_OPC_PowerOFF      =0x04
RNet_OPC_PowerON       =0x05
RNet_OPC_ResetALL      =0x06
RNet_OPC_ACK           =0x7F
#IO
RNet_OPC_SetOutput     =0x10
RNet_OPC_SetAllOutput  =0x11
RNet_OPC_ReadInput     =0x13
RNet_OPC_ReqReadInput  =0x14
RNet_OPC_ReadAll       =0x15
#Settings
RNet_OPC_ChangeID      =0x50
RNet_OPC_ChangeNode    =0x51
RNet_OPC_SetBlink      =0x52
RNet_OPC_SetPulse      =0x53
RNet_OPC_SetCheck      =0x54

RNet_OPC_ReadEEPROM    =0x60
RNet_msg_len_NotWhole  =0xFF

class FIFOBuf:
    def __init__(self):
        self.buf = b''

    def add(self, data):
        self.buf += data

    def read(self, n):
        ret = self.data[0:n]
        self.data = self.data[n:]
        return ret

    def getLength(self):
        if(self.buf[0] == RNet_OPC_DEV_ID or
           self.buf[0] == RNet_OPC_SetEmergency or
           self.buf[0] == RNet_OPC_RelEmergency or
           self.buf[0] == RNet_OPC_PowerON or
           self.buf[0] == RNet_OPC_PowerOFF or
           self.buf[0] == RNet_OPC_ResetALL):
            return 1
        elif (self.buf[0] == RNet_OPC_ACK):
            return 2
        elif(self.buf[0] == RNet_OPC_ChangeID or
             self.buf[0] == RNet_OPC_SetCheck):
            return 4
        elif (self.buf[0] == RNet_OPC_SetOutput or
              self.buf[0] == RNet_OPC_ChangeNode):
            return 5
        elif(self.buf[0] == RNet_OPC_SetBlink):
            return 8
        elif(self.buf[0] == RNet_OPC_ReadAll):
            if(len(self.data) < 3):
                 return -1  # Not enough bytes
            return 4+self.buf[1];
        elif(self.buf[0] == RNet_OPC_SetAllOutput):
            if(len(self.buf) < 4):
                 return -1  # Not enough bytes
            return (self.buf[2] + 1) / 2 + 4;
        elif(self.buf[0] == RNet_OPC_ReadInput):
            if(len(self.data) < 4):
                 return -1  # Not enough bytes
            print("ORI\n")
            return self.buf[2] + 4
        else:
            return 1


for i in range(0,10):
	print(".", end='' if i != 9 else '\n', flush=True)
	time.sleep(0.3)

def fun(i):
	if i == 0:
		ser.write(b'\x03\x11\x00\x04\x00\x00\xBF')
		ser.write(b'\x04\x11\x00\x04\x00\x00\xBF')
		ser.write(b'\x05\x11\x00\x04\x00\x00\xBF')
	elif i == 1:
		ser.write(b'\x03\x11\x00\x04\x01\x00\xBE')
	elif i == 2:
		ser.write(b'\x04\x11\x00\x04\x01\x00\xBE')
	elif i == 3:
		ser.write(b'\x05\x11\x00\x04\x01\x00\xBE')

i = 0
while(1):
    buf = FIFOBuf()
    fun(i)
    i+= 1
    if i >= 4:
        i = 0
    buf.add(ser.read(100))
    if buf.buf:
        l = buf.getLength()
        print(l, buf.read(l))
