#!/usr/bin/env python3
"""
render_gif.py

Reads the .frames files produced by the C program (see src/recorder.c
for the file format) and renders them into looping GIF animations
in the assets/ folder.

The C side knows nothing about graphics, this script knows nothing
about sorting. The plain text frames file is the only contract
between the two.

Usage:
    python3 scripts/render_gif.py            # render all frames files
    python3 scripts/render_gif.py quick      # render only data/quick.frames

Requires: Pillow (pip install Pillow)
"""

import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

# ----------------------------------------------------------------------
# Appearance (GitHub dark theme friendly)
# ----------------------------------------------------------------------
WIDTH, HEIGHT = 720, 360
MARGIN = 24
TITLE_HEIGHT = 48

COLOR_BACKGROUND = "#0d1117"   # GitHub dark background
COLOR_BAR = "#388bfd"          # default bar
COLOR_HIGHLIGHT = "#f85149"    # elements being compared / swapped
COLOR_SORTED = "#3fb950"       # final green sweep
COLOR_TEXT = "#e6edf3"
COLOR_SUBTEXT = "#8b949e"

FONT_PATH = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"

# ----------------------------------------------------------------------
# Animation settings
# ----------------------------------------------------------------------
MAX_FRAMES = 140        # subsample long recordings down to this many frames
FRAME_DURATION_MS = 45  # delay between frames
END_PAUSE_MS = 1500     # how long the final sorted frame stays on screen

TITLES = {
    "bubble":    "Bubble Sort",
    "selection": "Selection Sort",
    "insertion": "Insertion Sort",
    "merge":     "Merge Sort",
    "quick":     "Quick Sort",
    "heap":      "Heap Sort",
}


def load_frames(path: Path):
    """Parse a .frames file into (n, list_of_frames).

    Each frame is a tuple (values, hl1, hl2) where values is a list
    of ints and hl1/hl2 are highlighted indices (-1 means none).
    """
    lines = path.read_text().strip().splitlines()
    n = int(lines[0])
    frames = []
    for line in lines[1:]:
        values_part, highlight_part = line.split("|")
        values = [int(v) for v in values_part.split()]
        hl1, hl2 = (int(h) for h in highlight_part.split())
        frames.append((values, hl1, hl2))
    return n, frames


def subsample(frames, limit):
    """Reduce a long recording to at most `limit` frames.

    Keeps the first and last frame and picks the rest evenly,
    so the animation stays smooth and the GIF stays small.
    """
    if len(frames) <= limit:
        return frames
    step = (len(frames) - 1) / (limit - 1)
    return [frames[round(i * step)] for i in range(limit)]


def draw_frame(title, values, n, hl1, hl2, sorted_upto, font, small_font):
    """Render a single frame as a Pillow image.

    sorted_upto: indices below this value are drawn green
    (used for the final "everything is sorted" sweep).
    """
    img = Image.new("RGB", (WIDTH, HEIGHT), COLOR_BACKGROUND)
    draw = ImageDraw.Draw(img)

    draw.text((MARGIN, 14), title, font=font, fill=COLOR_TEXT)
    draw.text((WIDTH - MARGIN - 130, 22), f"n = {n} elements",
              font=small_font, fill=COLOR_SUBTEXT)

    # Bar geometry: fill the area below the title, leave 2 px gaps.
    chart_top = TITLE_HEIGHT + 10
    chart_height = HEIGHT - chart_top - MARGIN
    slot = (WIDTH - 2 * MARGIN) / n
    bar_width = max(2, slot - 2)
    max_value = max(values)

    for i, value in enumerate(values):
        bar_height = max(3, (value / max_value) * chart_height)
        x0 = MARGIN + i * slot
        y0 = HEIGHT - MARGIN - bar_height
        x1 = x0 + bar_width
        y1 = HEIGHT - MARGIN

        if i < sorted_upto:
            color = COLOR_SORTED
        elif i in (hl1, hl2):
            color = COLOR_HIGHLIGHT
        else:
            color = COLOR_BAR

        draw.rectangle([x0, y0, x1, y1], fill=color)

    return img


def render(frames_path: Path, output_path: Path):
    """Render one .frames file into a looping GIF."""
    name = frames_path.stem
    title = TITLES.get(name, name.title())
    n, frames = load_frames(frames_path)
    frames = subsample(frames, MAX_FRAMES)

    font = ImageFont.truetype(FONT_PATH, 24)
    small_font = ImageFont.truetype(FONT_PATH, 15)

    images = [
        draw_frame(title, values, n, hl1, hl2, 0, font, small_font)
        for values, hl1, hl2 in frames
    ]

    # Final touch: a green sweep over the sorted array, 4 bars at a time.
    final_values = frames[-1][0]
    for upto in range(0, n + 1, 4):
        images.append(
            draw_frame(title, final_values, n, -1, -1, upto, font, small_font)
        )
    images.append(
        draw_frame(title, final_values, n, -1, -1, n, font, small_font)
    )

    durations = [FRAME_DURATION_MS] * (len(images) - 1) + [END_PAUSE_MS]

    images[0].save(
        output_path,
        save_all=True,
        append_images=images[1:],
        duration=durations,
        loop=0,        # 0 means "loop forever", so the GIF replays itself
        optimize=True,
    )
    size_kb = output_path.stat().st_size / 1024
    print(f"  {name:<9} -> {output_path}  ({len(images)} frames, {size_kb:.0f} KB)")


def main():
    root = Path(__file__).resolve().parent.parent
    data_dir = root / "data"
    assets_dir = root / "assets"
    assets_dir.mkdir(exist_ok=True)

    only = sys.argv[1] if len(sys.argv) > 1 else None
    frame_files = sorted(data_dir.glob("*.frames"))
    if only:
        frame_files = [f for f in frame_files if f.stem == only]

    if not frame_files:
        sys.exit("No .frames files found. Run ./sortviz first (see README).")

    print("Rendering GIFs:")
    for frames_path in frame_files:
        render(frames_path, assets_dir / f"{frames_path.stem}_sort.gif")


if __name__ == "__main__":
    main()
