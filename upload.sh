#!/bin/bash
echo "Programming stage..."
PROGRASPI=/home/aleksey/rpi/bootloader01/prograspi
FIRMWARE=/home/aleksey/rpi/firmware/build/firmware.hex

rm ./port
ln -s /dev/ttyUSB* ./port
sudo $PROGRASPI $FIRMWARE ./port
