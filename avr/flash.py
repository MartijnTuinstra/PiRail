#!/usr/bin/python3

import os
import argparse
import fnmatch
import subprocess
import sys
import serial.tools.list_ports

devices = os.listdir("./build")

class SerialDevice():
  commands = ['-c', 'arduino',  '-b',  '57600']
  def __init__(self, name):
    self.name = name
    self.commands.extend(['-P', self.name])
  def __str__(self):
    return self.name

class USBTinyIsp():
  commands = ['-c', 'usbtiny']
  def __str__(self):
    return "usbtiny"

if sys.platform.startswith("win"):
    _pdev = [SerialDevice(d.device) for d in serial.tools.list_ports.comports()]
else:
    _pdev = os.listdir("/dev/")
    _pdev = [SerialDevice(f"/dev/{d}") for d in _pdev if fnmatch.fnmatch(d, "ttyUSB*")]

_pdev.append(USBTinyIsp())

i = 1
for d in _pdev:
  print("["+str(i)+"]  "+str(d)+"\t", end="")
  i += 1
print("")
i = int(input("Which device? "))
print(str(_pdev[i-1]), _pdev[i-1].commands)
pdev = _pdev[i-1]


c = ""

while(c != "n" and c != "N"):

  i = 1
  for dev in devices:
    print("["+str(i)+"]  "+dev+"\t" , end="")
    i += 1
  print("")
  i = int(input("Which programmer? "))
  print(devices[i-1])
  dev = devices[i-1]

  projects = os.listdir("./build/"+dev)
  projects = [p for p in projects if p.endswith("flash.hex")]

  i = 1
  for project in projects:
    print("["+str(i)+"]  "+project+"\t" , end="")
    i += 1
  print("")
  i = int(input("Which project? "))
  print(projects[i-1])
  f = projects[i-1]


  if sys.platform.startswith("win"):
    cmd = ['"C:\\Program Files (x86)\\Arduino\\hardware\\tools\\avr\\bin\\avrdude.exe"', '-C', '"C:\\Program Files (x86)\\Arduino\\hardware\\tools\\avr\\etc\\avrdude.conf"', '-p', dev, *pdev.commands, '-v', '-U', 'flash:w:build/'+dev+'/'+f+'/'+f+'.flash']
    print(" ".join(cmd))
    subprocess.call(cmd)
  else:
    cmd = ['"C:\\Program Files (x86)\\Arduino\\hardware\\tools\\avr\\bin\\avrdude.exe"', '-C', '"C:\\Program Files (x86)\\Arduino\\hardware\\tools\\avr\\etc\\avrdude.conf"', '-p', dev, *pdev.commands, '-v', '-U', 'flash:w:build/'+dev+'/'+f+'/'+f+'.flash']
    print(" ".join(cmd))
    print("avrdude -p "+dev+" "+" ".join(pdev.commands)+" -v -U flash:w:build/"+dev+"/"+f)
    os.system("avrdude -p "+dev+" "+" ".join(pdev.commands)+" -v -U flash:w:build/"+dev+"/"+f) 
  c = input("Again (y/n)? ")

