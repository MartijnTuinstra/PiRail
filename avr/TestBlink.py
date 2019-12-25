import serial
import time

ser = serial.Serial('COM9', 500000, timeout=0.1)

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
	fun(i)
	i+= 1
	if i >= 4:
		i = 0
	read = ser.read(100)
	if read:
		print(read)