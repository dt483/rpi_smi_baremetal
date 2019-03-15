#!/bin/bash
echo "Programming stage..."
#PROGRASPI=/home/aleksey/rpi/bootloader01/prograspi
SRCPATH=$PWD
BUILDPATH=$PWD-build
cd $BUILDPATH && make	
cd $SRCPATH	
FIRMWARE=$BUILDPATH/firmware.img
echo "Transferring file $FIRMWARE"
rm ./port
ln -s /dev/ttyUSB* ./port
#sudo $PROGRASPI $FIRMWARE ./port
DEV=./port
sudo chown $LOGNAME:$LOGNAME ./port

sudo stty -F $DEV 115200
sudo sz $FIRMWARE > $DEV < $DEV --xmodem

sudo minicom -D $DEV;
#rm ./port
#blablablabl
