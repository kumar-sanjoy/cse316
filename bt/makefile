# MCU settings
MCU = atmega32
F_CPU = 1000000UL

# Tools
CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size
PROGRAMMER = usbasp

# Files
TARGET = main

# Compiler flags
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall

# Default target
all: $(TARGET).hex

# Compile
$(TARGET).elf: $(TARGET).c
	$(CC) $(CFLAGS) -o $@ $<

# Generate HEX
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	$(SIZE) $<

# Flash to ATmega32
flash: $(TARGET).hex
	avrdude -c $(PROGRAMMER) -p m32 -B 32 -U flash:w:$(TARGET).hex

# Clean build files
clean:
	rm -f *.elf *.hex