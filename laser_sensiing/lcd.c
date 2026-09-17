#include "lcd.h"
#include "i2c.h"
#include <util/delay.h>

#define LCD_ADDR 0x27 // Set to 0x3F if your PCF8574 backpack requires it

#define LCD_RS 0x01
#define LCD_EN 0x04
#define LCD_BL 0x08

static void lcd_send_nibble(uint8_t nibble, uint8_t rs)
{
    uint8_t data = (nibble << 4) | LCD_BL;

    if (rs)
        data |= LCD_RS;

    if (!i2c_start(LCD_ADDR << 1)) return;

    // Present data with EN LOW
    i2c_write(data);
    _delay_us(2);

    // Pull EN HIGH
    i2c_write(data | LCD_EN);
    _delay_us(2);

    // Pull EN LOW (falling edge latches data)
    i2c_write(data & ~LCD_EN);
    _delay_us(50);

    i2c_stop();
}

static void lcd_send(uint8_t value, uint8_t rs)
{
    lcd_send_nibble(value >> 4, rs);
    lcd_send_nibble(value & 0x0F, rs);
}

void lcd_init(void)
{
    i2c_init();

    // Force PCF8574 outputs to a known LOW state (EN, RS, backlight) as fast
    // as possible: on power-up, before any I2C traffic, the PCF8574 defaults
    // ALL pins HIGH, which glitches the LCD's EN/RS/data lines and can latch
    // garbage characters before we get a chance to reset it. Retry briefly in
    // case the bus isn't ready for the very first transaction yet.
    for (uint8_t attempt = 0; attempt < 5; attempt++) {
        if (i2c_start(LCD_ADDR << 1)) {
            i2c_write(0x00);
            i2c_stop();
            break;
        }
        _delay_ms(5);
    }

    _delay_ms(50); // HD44780 requires >=15-40ms after Vcc stabilizes before the first instruction

    // HD44780 8-bit software reset sequence (3x 0x03 writes) -- resyncs the
    // controller regardless of what state it was left in.
    lcd_send_nibble(0x03, 0);
    _delay_ms(10);

    lcd_send_nibble(0x03, 0);
    _delay_ms(2);

    lcd_send_nibble(0x03, 0);
    _delay_ms(2);

    // Switch to 4-bit mode
    lcd_send_nibble(0x02, 0);
    _delay_ms(5);

    // Display configuration
    lcd_send(0x28, 0); // 4-bit, 2-line, 5x8 font
    lcd_send(0x08, 0); // Display OFF
    lcd_clear();       // Clear screen -- wipes any garbage left in DDRAM
    lcd_send(0x06, 0); // Entry mode: increment cursor
    lcd_send(0x0C, 0); // Display ON, cursor OFF
    lcd_set_cursor(0, 0); // Deterministic cursor position every boot
}

void lcd_clear(void)
{
    lcd_send(0x01, 0);
    _delay_ms(5); // Increased delay for slower LCD controllers
}

void lcd_set_cursor(uint8_t row, uint8_t col)
{
    uint8_t address = (row == 0) ? (0x00 + col) : (0x40 + col);
    lcd_send(0x80 | address, 0);
}

void lcd_print(const char *str)
{
    while (*str)
        lcd_send(*str++, 1);
}
