# MCU settings
MCU     = atmega32
# ATmega32 factory-default fuses select the internal RC oscillator with
# no clock divider, so it really does run at 1MHz.
F_CPU   = 1000000UL

# Tools
CC      = avr-gcc
OBJCOPY = avr-objcopy
SIZE    = avr-size
PROGRAMMER = usbasp

# Files
TARGET  = main

# Compiler flags
CFLAGS  = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall

.PHONY: all flash check clean

all: $(TARGET).hex

$(TARGET).elf: $(TARGET).c
	$(CC) $(CFLAGS) -o $@ $<

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	$(SIZE) $<

# Non-destructive: only reads the device signature, writes nothing.
# Use this first to check if the chip answers at all (ISP alive),
# without risking a corrupted flash write.
check:
	avrdude -c $(PROGRAMMER) -p m32 -B 200

# Flashes the blink test. If PB0 blinks after this, the chip is alive
# and executing code correctly.
flash: $(TARGET).hex
	avrdude -c $(PROGRAMMER) -p m32 -B 200 -F -U flash:w:$(TARGET).hex:i

clean:
	rm -f *.elf *.hex
