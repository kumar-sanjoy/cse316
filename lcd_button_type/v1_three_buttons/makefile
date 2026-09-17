MCU = atmega32
# ATmega32 factory-default fuses select the internal RC oscillator with
# no clock divider, so it really does run at 1MHz.
F_CPU = 1000000UL

CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size
PROGRAMMER = usbasp

TARGET = main
SRC = main.c lcd.c i2c.c

CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall

.PHONY: all flash clean

all: $(TARGET).hex

$(TARGET).elf: $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC)

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	$(SIZE) $<

flash: $(TARGET).hex
	avrdude -c $(PROGRAMMER) -p m32 -B 200 -F -U flash:w:$(TARGET).hex:i

diag.elf: diag.c i2c.c
	$(CC) $(CFLAGS) -o $@ diag.c i2c.c

diag.hex: diag.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	$(SIZE) $<

flash-diag: diag.hex
	avrdude -c $(PROGRAMMER) -p m32 -B 200 -F -U flash:w:diag.hex:i

clean:
	rm -f *.elf *.hex
