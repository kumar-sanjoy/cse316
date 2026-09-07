LED MATRIX LAB - HOW TO USE THESE FILES
========================================

Files:
  led_matrix.c   - the AVR C program (static / flash / rotate)
  hex_to_c.py    - converts hex codes into the C arrays led_matrix.c needs

What you'll be told in the lab, and where it goes
--------------------------------------------------
1. The SYMBOL to display
   -> Design it at https://xantorohara.github.io/led-matrix-editor/
   -> Copy the exported hex string
   -> Run hex_to_c.py, choose option 1, paste the hex
   -> Copy the generated `image[8] = {...}` from output.txt into
      led_matrix.c, replacing the existing `image` array

2. The ROTATION frames (for the rotating-display part)
   -> Design each frame of the animation the same way at the editor site
   -> Run hex_to_c.py, choose option 2, enter how many frames and paste
      each hex code in the order you want them to play
   -> Copy the generated `#define NUM_PHASES` and `image_rotate[...]`
      block into led_matrix.c, replacing the existing ones

3. The COLOR (red or green)
   -> This is a wiring choice, not a code change. Connect COL_PORT
      (PORTB by default) to the 8 pins for whichever color you were
      assigned (R1-R8 for red, G1-G8 for green). Leave the other
      8 column pins disconnected.

4. The DIRECTION (left or right rotate)
   -> In led_matrix.c, set ROTATE_FORWARD to 1 or 0. Try 1 first; if
      the symbol rotates the wrong way, switch it to 0. You don't need
      to touch hex_to_c.py or regenerate frames for this.

5. The PORTS assigned for rows/columns
   -> Change ROW_PORT/ROW_DDR and COL_PORT/COL_DDR at the top of
      led_matrix.c. Defaults are PORTA (rows) and PORTB (columns).
   -> If you're told to use PORTC and hit issues with unused pins,
      set DISABLE_JTAG to 1 (see note below).

Tuning brightness / speed
--------------------------
- ROW_DELAY_MS controls per-row on-time. Too low = dim, too high =
  visible flicker. Find a value that looks solid and bright by trial
  and error, as the spec says.
- STATIC_REFRESH_COUNT / FLASH_ON_REFRESH_COUNT / FLASH_OFF_REFRESH_COUNT /
  ROTATE_REFRESH_PER_PHASE control how long each mode/phase stays on
  screen before moving to the next thing. Raise or lower as needed.

JTAG note (only if you need PORTC)
------------------------------------
Some PORTC pins can't be used for I/O until JTAG is disabled. Either
uncheck the JTAG fuse bit when programming, or set DISABLE_JTAG to 1
in led_matrix.c, which writes to MCUCSR at startup to disable it in
software. Default FUSE byte (JTAG enabled): E199. With JTAG disabled:
E1D9. Fuse calculator: http://www.engbedded.com/fusecalc/

Quick checklist before you compile
------------------------------------
[ ] image[8] updated with the assigned symbol's hex
[ ] image_rotate[][] and NUM_PHASES updated with rotation frames
[ ] ROW_PORT/COL_PORT match how you wired the matrix
[ ] Correct 8 column pins wired for the assigned color
[ ] ROTATE_FORWARD set to match the assigned rotation direction
[ ] ROW_DELAY_MS tuned so the symbol looks bright and steady
[ ] In main(), only the part(s) you're demoing are uncommented
    (or leave all three to cycle through everything)
