"""
hex_to_c.py
-----------
Converts hex strings exported from the LED Matrix Editor
(https://xantorohara.github.io/led-matrix-editor/) into the C array
format used by led_matrix.c.

Run it and follow the prompts. It writes output.txt with ready-to-paste
C code for either:
  1) a single static image        -> image[8]
  2) a rotation animation         -> image_rotate[N][8]  (+ NUM_PHASES)

Notes:
  - Each hex string is one 8x8 frame (16 hex chars, 2 per row), exactly
    what the matrix editor site exports.
  - Bits are inverted automatically: columns are active LOW (a column
    pin sinks current to ground to turn its LED on), so a '1' bit in
    the editor becomes a '0' in the generated C code.
  - For a rotation animation, enter phases in the order you want them
    to play. You do NOT need to reverse anything here for left vs.
    right rotation - that's controlled by the ROTATE_FORWARD macro in
    led_matrix.c. If the animation plays the wrong way, flip that
    macro instead of regenerating frames.
"""


def get_rev_bin(hex_byte):
    """Convert a 2-char hex string to an inverted 8-bit binary literal."""
    b = bin(int(hex_byte, 16))[2:].zfill(8)
    inverted = ''.join('1' if c == '0' else '0' for c in b)
    return "0b" + inverted


def get_row_activations(hex_string):
    """Convert one 16-char hex frame into a C-style '{ 0b..., ... }' row list."""
    hex_string = hex_string[::-1]
    rows = []
    for i in range(0, 16, 2):
        byte = hex_string[i + 1] + hex_string[i]
        rows.append(get_rev_bin(byte))
    return "{ " + ", ".join(rows) + " }"


def get_static_code(hex_string):
    return "volatile unsigned char image[8]\n= " + get_row_activations(hex_string) + ";\n"


def get_rotation_code(hex_list):
    n = len(hex_list)
    lines = [get_row_activations(h) for h in hex_list]
    body = ",\n    ".join(lines)
    return (
        f"#define NUM_PHASES {n}\n\n"
        f"volatile unsigned char image_rotate[NUM_PHASES][8]\n= {{\n    {body}\n}};\n"
    )


def write_txt(filename, text):
    with open(filename, 'w', encoding='utf-8') as f:
        f.write(text)


def main():
    print("LED Matrix hex -> C converter")
    print("1) Static image")
    print("2) Rotation animation (multiple phases)")
    choice = input("Choose 1 or 2: ").strip()

    if choice == "1":
        hex_string = input("Paste the 16-char hex code: ").strip()
        output = get_static_code(hex_string)

    elif choice == "2":
        n = int(input("How many phases? ").strip())
        hex_list = []
        for i in range(n):
            h = input(f"Paste hex code for phase {i}: ").strip()
            hex_list.append(h)
        output = get_rotation_code(hex_list)

    else:
        print("Invalid choice.")
        return

    write_txt("output.txt", output)
    print("\nOutput written to output.txt - paste it into led_matrix.c\n")
    print(output)


if __name__ == "__main__":
    main()
