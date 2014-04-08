###############################################################################
# Makefile for the project master4
###############################################################################

## General Flags
PROJECT = master4
MCU = atmega128
TARGET = master4.elf
CC = avr-gcc

CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=master4.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = LCD.o SPI.o RTC.o UART.o FSlave.o FMain.o FStandalone.o Keypad.o FApplication.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) master4.hex master4.eep master4.lss size

## Compile
LCD.o: ../LCD.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

SPI.o: ../SPI.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

RTC.o: ../RTC.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

UART.o: ../UART.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

FSlave.o: ../FSlave.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

FMain.o: ../FMain.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

FStandalone.o: ../FStandalone.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

Keypad.o: ../Keypad.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

FApplication.o: ../FApplication.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) master4.elf dep/* master4.hex master4.eep master4.lss master4.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

