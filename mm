
ARMGNU ?= arm-none-eabi

AOPS = --warn --fatal-warnings 
COPS = -Wall -Werror -O0 -nostdlib -nostartfiles -ffreestanding 
PROGRASPI = /home/aleksei/rpi/raspberrypi/bootloader01/prograspi
#DEVICE = /dev/ttyUSB2




all : clean gcc upload

gcc : blinker01.hex blinker01.bin

clean :
	rm -f *.o
	rm -f *.bin
	rm -f *.hex
	rm -f *.elf
	rm -f *.list
	rm -f *.img
	rm -f *.bc
	rm -f *.clang.opt.s
	rm -f ./port

vectors.o : vectors.s
	$(ARMGNU)-as vectors.s -o vectors.o

blinker01.o : blinker01.c
	$(ARMGNU)-gcc $(COPS) -c blinker01.c -o blinker01.o

blinker01.elf : memmap vectors.o blinker01.o 
	$(ARMGNU)-ld vectors.o blinker01.o -T memmap -o blinker01.elf
	$(ARMGNU)-objdump -D blinker01.elf > blinker01.list

blinker01.bin : blinker01.elf
	$(ARMGNU)-objcopy blinker01.elf -O binary blinker01.bin
	$(ARMGNU)-objcopy blinker01.elf -O binary kernel.img

blinker01.hex : blinker01.elf
	$(ARMGNU)-objcopy blinker01.elf -O ihex blinker01.hex

upload: blinker01.hex
	ln -s /dev/ttyUSB* ./port
	sudo $(PROGRASPI) blinker01.hex ./port
	





