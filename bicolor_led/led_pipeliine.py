#!/usr/bin/env python3
"""
led_pipeline.py
----------------
Turns a lab-format spec file (static image + N rotation stages, each as
"<color> <hex16>") into ready-to-paste C array definitions matching
bicolor_matrix_pa_pc_pd.c (image_red / image_green / image_rotate_red /
image_rotate_green).

Input file format
-----------------
    <color> <hex16>            # static image. 0 = red, 1 = green
    <N>                        # number of rotation stages
    <color> <hex16>            # stage 0
    <color> <hex16>            # stage 1
    ...                        # N lines total

Rules:
- Anything after "//" on a line is a comment and is stripped.
- Blank lines are ignored.
- <hex16> is 16 hex digits = 8 bytes = one byte per row, top row first.
- <color> is 0 (red) or 1 (green). Whichever color ISN'T given for an
  image gets an all-off (blank) row on that color's columns, since this
  format only lets one color be specified per image/stage.

Usage
-----
    python led_pipeline.py input.txt output.txt
"""

import sys

BLANK_BYTE = 0xFF  # active-low columns: all 1s = all off


def strip_comment(line: str) -> str:
    return line.split("//", 1)[0].strip()


def read_meaningful_lines(path):
    lines = []
    with open(path, "r") as f:
        for raw in f:
            cleaned = strip_comment(raw)
            if cleaned:
                lines.append(cleaned)
    return lines


def parse_entry(line: str):
    """'<color> <hex16>' -> (color: int, rows: list[int] of length 8)."""
    parts = line.split()
    if len(parts) != 2:
        raise ValueError(f"Malformed line (expected '<color> <hex16>'): {line!r}")
    color_str, hex_str = parts
    try:
        color = int(color_str)
    except ValueError:
        raise ValueError(f"Color must be an integer 0/1, got {color_str!r} in: {line!r}")
    if color not in (0, 1):
        raise ValueError(f"Color must be 0 (red) or 1 (green), got {color} in: {line!r}")
    hex_str = hex_str.strip()
    if len(hex_str) != 16:
        raise ValueError(f"Expected 16 hex digits (8 bytes), got {len(hex_str)} in: {line!r}")
    try:
        rows = [int(hex_str[i:i + 2], 16) for i in range(0, 16, 2)]
    except ValueError:
        raise ValueError(f"Non-hex character found in: {line!r}")
    return color, rows


def to_binary_literal(byte: int) -> str:
    return f"0b{byte:08b}"


def build_dual_color_rows(color, rows):
    """Given the single color+rows for an image, return (red_rows, green_rows),
    with the un-specified color filled with blank (all-off) rows."""
    red_rows = rows if color == 0 else [BLANK_BYTE] * 8
    green_rows = rows if color == 1 else [BLANK_BYTE] * 8
    return red_rows, green_rows


def emit_1d_array(name, rows):
    values = ", ".join(to_binary_literal(b) for b in rows)
    return f"volatile unsigned char {name}[8]\n= {{ {values} }};"


def emit_2d_array(name, phase_rows_list):
    row_lines = []
    for rows in phase_rows_list:
        values = ", ".join(to_binary_literal(b) for b in rows)
        row_lines.append(f"    {{ {values} }}")
    body = ",\n".join(row_lines)
    return f"volatile unsigned char {name}[NUM_PHASES][8]\n= {{\n{body}\n}};"


def main():
    if len(sys.argv) != 3:
        print("Usage: python led_pipeline.py <input.txt> <output.txt>")
        sys.exit(1)

    in_path, out_path = sys.argv[1], sys.argv[2]
    lines = read_meaningful_lines(in_path)

    if len(lines) < 2:
        raise ValueError("Input needs at least a static image line and a stage-count line.")

    idx = 0
    static_color, static_rows = parse_entry(lines[idx]); idx += 1
    static_red, static_green = build_dual_color_rows(static_color, static_rows)

    try:
        num_phases = int(lines[idx].split()[0])
    except ValueError:
        raise ValueError(f"Expected an integer stage count, got: {lines[idx]!r}")
    idx += 1

    if len(lines) - idx < num_phases:
        raise ValueError(
            f"Stage count says {num_phases}, but only {len(lines) - idx} "
            f"stage line(s) found after it."
        )

    rotate_red, rotate_green = [], []
    for _ in range(num_phases):
        color, rows = parse_entry(lines[idx]); idx += 1
        r_rows, g_rows = build_dual_color_rows(color, rows)
        rotate_red.append(r_rows)
        rotate_green.append(g_rows)

    out_chunks = [
        f"/* NUM_PHASES must be set to {num_phases} in the driver's CONFIG section. */",
        "",
        emit_1d_array("image_red", static_red),
        "",
        emit_1d_array("image_green", static_green),
        "",
        emit_2d_array("image_rotate_red", rotate_red),
        "",
        emit_2d_array("image_rotate_green", rotate_green),
        "",
    ]

    with open(out_path, "w") as f:
        f.write("\n".join(out_chunks))

    print(f"Wrote {out_path}: static image + {num_phases}-stage rotation.")


if __name__ == "__main__":
    main()