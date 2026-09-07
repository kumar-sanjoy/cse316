/* =========================================================================
 * Bicolor 8x8 LED Matrix Driver - ATmega32
 * -------------------------------------------------------------------------
 * Generalized version of the reference solution. Everything you'd only
 * find out during the lab (port, color, direction, delay, symbol) is a
 * single change in the CONFIG section or the two pattern arrays below.
 * Fill the arrays using hex_to_c.py (see README.txt).
 * ========================================================================= */

#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>

/* =========================== CONFIG ==================================== */

/* Port driving the ROWS (anode, active HIGH). Change to match whatever
 * port the teacher tells you to use for rows. */
#define ROW_PORT   PORTA
#define ROW_DDR    DDRA

/* Port driving the COLUMNS for the color you were assigned (cathode,
 * active LOW). Physically wire only the 8 pins for that color
 * (either R1-R8 or G1-G8) to this port - the code doesn't need to know
 * which color it is, that's purely a wiring choice. */
#define COL_PORT   PORTD
#define COL_DDR    DDRD

/* Set to 1 only if you're using PORTC pins and need to disable JTAG
 * first (see README.txt). */
#define DISABLE_JTAG 0

/* Per-row on-time during multiplexing, in ms. Tune by trial and error:
 * too short -> dim symbol, too long -> visible row-by-row flicker. */
#define ROW_DELAY_MS 0.5

/* How many full-matrix refresh passes each mode/phase stays on screen.
 * Bigger number = longer on screen, not brighter (brightness is
 * ROW_DELAY_MS). Adjust to taste. */
#define STATIC_REFRESH_COUNT     150
#define FLASH_ON_REFRESH_COUNT   100
#define FLASH_OFF_REFRESH_COUNT  100
#define ROTATE_REFRESH_PER_PHASE 200

/* Number of frames in the rotation animation. Must equal the number of
 * rows in image_rotate[][] below - hex_to_c.py prints the matching
 * #define for you when you generate the array. */
#define NUM_PHASES 8

/* Which way to play the rotation frames.
 *   1 -> play phases 0, 1, 2, ... NUM_PHASES-1  (as generated)
 *   0 -> play phases NUM_PHASES-1, ... 1, 0     (reverse order)
 * If the lab assigns "left" but your frames were made for "right" (or
 * vice versa), just flip this instead of regenerating all the frames. */
#define ROTATE_FORWARD 1

/* ========================================================================= */

/* Static symbol for part 1 and 2 (static display + flash).
 * Replace with output from hex_to_c.py (option 1). */
volatile unsigned char image[8]
= { 0b11111111, 0b11100001, 0b11011101, 0b11011101, 0b11100001, 0b11111101, 0b11111101, 0b11111101 };


/* Rotation animation frames for part 3.
 * Replace with output from hex_to_c.py (option 2). Keep NUM_PHASES above
 * in sync with however many frames you generate. */
volatile unsigned char image_rotate[NUM_PHASES][8]
= {
    { 0b11111111, 0b11100001, 0b11011101, 0b11011101, 0b11100001, 0b11111101, 0b11111101, 0b11111101 },
    { 0b11111111, 0b11000011, 0b10111011, 0b10111011, 0b11000011, 0b11111011, 0b11111011, 0b11111011 },
    { 0b11111111, 0b10000111, 0b01110111, 0b01110111, 0b10000111, 0b11110111, 0b11110111, 0b11110111 },
    { 0b11111111, 0b00001111, 0b11101110, 0b11101110, 0b00001111, 0b11101111, 0b11101111, 0b11101111 },
    { 0b11111111, 0b00011110, 0b11011101, 0b11011101, 0b00011110, 0b11011111, 0b11011111, 0b11011111 },
    { 0b11111111, 0b00111100, 0b10111011, 0b10111011, 0b00111100, 0b10111111, 0b10111111, 0b10111111 },
    { 0b11111111, 0b01111000, 0b01110111, 0b01110111, 0b01111000, 0b01111111, 0b01111111, 0b01111111 },
    { 0b11111111, 0b11110000, 0b11101110, 0b11101110, 0b11110000, 0b11111110, 0b11111110, 0b11111110 }
};


/* All columns off (active LOW, so 1 = off). Used for the "flash" mode. */
volatile unsigned char blankscreen[8] =
{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

/* Row selector: a single '1' bit that walks down through the rows, one
 * HIGH row at a time (top -> bottom, then wraps back to top).
 * Renamed from the reference's "column_selector" since it actually
 * drives ROW_PORT, not the columns. */
volatile unsigned char row_selector = 0b10000000;

unsigned char next_row_selector(unsigned char current)
{
    unsigned char next = current >> 1;
    if (!next) {
        next = 0b10000000;   /* wrap back to row 1 */
    }
    return next;
}

/* One full multiplexed refresh of the matrix showing "pattern".
 * Call this repeatedly (see show_static/show_flash/show_rotation) to
 * keep an image visibly lit via persistence of vision. */
void show_pattern(volatile unsigned char pattern[8])
{
    unsigned char r = row_selector;
    ROW_PORT = r;
    for (int i = 0; i < 8; i++) {
        COL_PORT = pattern[i];
        _delay_ms(ROW_DELAY_MS);
        r = next_row_selector(r);
        ROW_PORT = r;
    }
}

/* Part 1: static display. */
void show_static(void)
{
    for (int i = 0; i < STATIC_REFRESH_COUNT; i++) {
        show_pattern(image);
    }
}

/* Part 2: flashing display. */
void show_flash(void)
{
    for (int i = 0; i < FLASH_ON_REFRESH_COUNT; i++) {
        show_pattern(image);
    }
    for (int i = 0; i < FLASH_OFF_REFRESH_COUNT; i++) {
        show_pattern(blankscreen);
    }
}

/* Part 3: rotating display (left or right, controlled by ROTATE_FORWARD). */
void show_rotation(void)
{
#if ROTATE_FORWARD
    for (int phase = 0; phase < NUM_PHASES; phase++) {
#else
    for (int phase = NUM_PHASES - 1; phase >= 0; phase--) {
#endif
        for (int j = 0; j < ROTATE_REFRESH_PER_PHASE; j++) {
            show_pattern(image_rotate[phase]);
        }
    }
}

int main(void)
{
    ROW_DDR = 0xFF;
    COL_DDR = 0xFF;

#if DISABLE_JTAG
    MCUCSR = (1 << JTD);
    MCUCSR = (1 << JTD);   /* must be written twice consecutively */
#endif

    while (1) {
        /* Comment out whichever parts you're not demoing right now.
         * Leaving all three uncommented just cycles through them. */
        // show_static();
        show_flash();
        // show_rotation();
    }
}
