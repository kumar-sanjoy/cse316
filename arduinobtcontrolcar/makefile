# MCU settings
MCU = atmega32
F_CPU = 1000000UL

# Tools
CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size
AVRDUDE = avrdude
PROGRAMMER = usbasp

# Files
TARGET = main
SRC = main.c i2c.c lcd.c

# Compiler flags
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall

# Default target
all: $(TARGET).hex

# Compile
$(TARGET).elf: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@

# Generate HEX
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	$(SIZE) $<

# Flash to ATmega32
flash: $(TARGET).hex
	$(AVRDUDE) -c $(PROGRAMMER) -p m32 -F -U flash:w:$(TARGET).hex:i

# Clean build files
clean:
	rm -f $(TARGET).elf $(TARGET).hex
