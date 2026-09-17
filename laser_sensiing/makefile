MCU = atmega32
F_CPU = 4000000UL

CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude

TARGET = main

CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall

SRC = main.c i2c.c lcd.c adc.c

all: $(TARGET).hex

$(TARGET).elf: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

flash: $(TARGET).hex
	$(AVRDUDE) -c usbasp -p m32 -B 10 -U flash:w:$(TARGET).hex:i

# Print the chip's current fuse bytes without changing anything.
read-fuses:
	$(AVRDUDE) -c usbasp -p m32 -B 10 -U lfuse:r:-:h -U hfuse:r:-:h

# Enable the internal Brown-out Detector so a power cut always forces a full,
# clean reset (main()/lcd_init() run from scratch) instead of the MCU limping
# along in a corrupted state after a partial Vcc dip.
#
# Only touches the BODEN/BODLEVEL bits (fuse low byte, bits 6:7). The clock
# select / startup-time bits (bits 0:5, same byte) are read from the chip and
# written back completely unchanged, so this cannot alter the clock source or
# brick ISP access.
bod-enable:
	@CUR=$$($(AVRDUDE) -c usbasp -p m32 -B 10 -U lfuse:r:-:h -q -q 2>/dev/null | tr -d '\r\n'); \
	if [ -z "$$CUR" ]; then \
		echo "ERROR: could not read the current lfuse (is the USBasp/board connected?)."; \
		echo "Refusing to guess a fuse value blind - aborting."; \
		exit 1; \
	fi; \
	NEW=$$(printf '0x%02X' $$(( 0x$$CUR & 0x3F )) ); \
	echo "Current lfuse: 0x$$CUR"; \
	echo "New lfuse (BOD enabled @4.0V; SUT/CKSEL bits preserved as-is): $$NEW"; \
	$(AVRDUDE) -c usbasp -p m32 -B 10 -U lfuse:w:$$NEW:m; \
	echo "Verifying:"; \
	$(AVRDUDE) -c usbasp -p m32 -B 10 -U lfuse:r:-:h

clean:
	rm -f $(TARGET).elf $(TARGET).hex

.PHONY: all flash read-fuses bod-enable clean
