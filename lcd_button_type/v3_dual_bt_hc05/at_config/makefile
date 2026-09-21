MCU = atmega32
# Weak/default AT mode (HC-05 powered up normally, no button held) talks
# at whatever baud it's already configured for -- 9600 here -- which the
# factory-default 1MHz internal RC clock handles fine. For full AT mode
# (button held at power-up, forced 38400 baud) run `make fuse-8mhz` first
# and build with F_CPU=8000000UL instead (see main.c's BAUD selection).
F_CPU = 1000000UL

CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size
PROGRAMMER = usbasp

TARGET = main
SRC = main.c lcd.c i2c.c uart.c

CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall

.PHONY: all flash clean fuse-8mhz fuse-1mhz

all: $(TARGET).hex

$(TARGET).elf: $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC)

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	$(SIZE) $<

flash: $(TARGET).hex
	avrdude -c $(PROGRAMMER) -p m32 -B 200 -F -U flash:w:$(TARGET).hex:i

# Switch the chip's clock source to the internal 8MHz RC oscillator.
# Run this ONCE before flashing this at_config firmware. Low fuse 0xE4:
# same BODEN/SUT as the factory default, CKSEL changed to 8MHz internal RC.
fuse-8mhz:
	avrdude -c $(PROGRAMMER) -p m32 -B 200 -F -U lfuse:w:0xE4:m

# Revert the chip back to the factory-default 1MHz internal RC oscillator.
# Run this AFTER you're done with AT config and BEFORE reflashing the real
# v3_dual_bt_hc05 firmware, which assumes F_CPU = 1000000UL.
fuse-1mhz:
	avrdude -c $(PROGRAMMER) -p m32 -B 200 -F -U lfuse:w:0xE1:m

clean:
	rm -f *.elf *.hex
