import serial
import time

ser = serial.Serial('COM9', 250000, timeout=1)

for i in range(0,10):
	print(".", end='' if i != 9 else '\n', flush=True)
	time.sleep(0.3)

while(1):
	ser.write(b'\x03\x11\x00\x04\x02\x00\xBD')
	read = ser.read(100)
	if read:
		print(read)