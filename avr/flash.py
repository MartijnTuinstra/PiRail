#!/usr/bin/python3

import os
import argparse
import fnmatch

devices = ["atmega328p", "atmega64a", "atmega2560"]
projects = ["main", "main_spi", "net_buffer", "test"]

i = 1
for dev in devices:
  print("["+str(i)+"]  "+dev+"\t" , end="")
  i += 1
print("")
i = int(input("Which programmer? "))
print(devices[i-1])
dev = devices[i-1]

i = 1
for project in projects:
  print("["+str(i)+"]  "+project+"\t" , end="")
  i += 1
print("")
i = int(input("Which project? "))
print(projects[i-1])
f = projects[i-1]

import sys
import serial.tools.list_ports

if sys.platform.startswith("win"):
    _pdev = serial.tools.list_ports.comports()
else:
    _pdev = os.listdir("/dev/")
    _pdev = [d for d in _pdev if fnmatch.fnmatch(d, "ttyUSB*")]

i = 1
for d in _pdev:
  print("["+str(i)+"]  "+d+"\t", end="")
  i += 1
print("")
i = int(input("Which device? "))
print(_pdev[i-1])
pdev = _pdev[i-1]

c = ""

while(c != "n" and c != "N"):
	os.system("avrdude -p "+dev+" -P /dev/"+pdev+" -c arduino -b 57600 -v -U flash:w:build/"+f+"/"+dev+".flash.hex") 
	c = input("Again (y/n)? ")

