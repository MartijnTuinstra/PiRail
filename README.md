# PiRail
Raspberry Pi Rail Software

This software package is for a modeltrain enthusiasts that want to automate trains on their layout.
It is especially created for club layout that do not have specified order.

This package contains
- Board designs
- Firmware for the arduino's
- Software Engine
- Web interface
- Z21 Emulator & firmware for arduino

# Requirements

## Hardware

- 100MB storage space
- 100MB min RAM size
- 1GHz 2core ARM or better adviced

- Z21 Command Unit
- A Railway
- DCC trains

## Software

- PHP5
- wiringPi
- openssl

# Installation

To install and launch the Engine:
```
make
./baan
```

To install and launch the Emulator:
```
cd Z21_Emulator
gcc emulator.c -std=c99 -lpthread -g -o Z21.out
./Z21.out
```

# Web interface

Make sure to drop the 'web' folder into your webserver. This package has no php webserver.

Browse to the index.php. This is the main interface.
From here you can:
- add/change train and so called Modules
- Control trains
- Control the layout
