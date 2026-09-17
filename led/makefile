MCU = atmega32
# ATmega32 factory-default fuses select the internal RC oscillator with
# no clock divider, so it really does run at 1MHz.
F_CPU = 1000000UL

CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size
PROGRAMMER = usbasp

TARGET = main

CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall

.PHONY: all flash clean

all: $(TARGET).hex

$(TARGET).elf: $(TARGET).c
	$(CC) $(CFLAGS) -o $@ $<

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	$(SIZE) $<

flash: $(TARGET).hex
	avrdude -c $(PROGRAMMER) -p m32 -B 200 -F -U flash:w:$(TARGET).hex:i

clean:
	rm -f *.elf *.hex