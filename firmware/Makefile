all : flash

TARGET:=src/main
CH32FUN:=./ch32fun/ch32fun
TARGET_MCU:=CH32V003

ADDITIONAL_C_FILES+=src/charlie.c src/systick.c
EXTRA_CFLAGS:=-I./include

LDFLAGS+=-Wl,--print-memory-usage

include $(CH32FUN)/ch32fun.mk

# using ardulink programmer
# FLASH_COMMAND:=$(MINICHLINK)/minichlink -3 -C ardulink -c /dev/tty.usbmodem123451 -w $(TARGET).bin 0x08000000 -T

# using usb bootloader
FLASH_COMMAND:=$(MINICHLINK)/minichlink -C b003boot -e -w $(TARGET).bin flash -b


flash : cv_flash
clean : cv_clean

local:
	gcc src/main.c src/charlie.c -I include/
	./a.out
	rm a.out