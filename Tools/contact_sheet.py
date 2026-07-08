"""Build labeled contact sheets of sliced sprites for visual verification."""
import os
import sys
from PIL import Image, ImageDraw

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SPR = os.path.join(ROOT, "Content", "Art", "Sprites")
OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, "Tools", "contact")
os.makedirs(OUT, exist_ok=True)

CELL = 128
PAD = 18

for sheet in sorted(os.listdir(SPR)):
    d = os.path.join(SPR, sheet)
    files = sorted(os.listdir(d))
    if not files:
        continue
    cols = 8
    rows = (len(files) + cols - 1) // cols
    canvas = Image.new("RGB", (cols * CELL, rows * (CELL + PAD)), (40, 40, 48))
    draw = ImageDraw.Draw(canvas)
    for i, fn in enumerate(files):
        img = Image.open(os.path.join(d, fn))
        img.thumbnail((CELL - 8, CELL - 8))
        x = (i % cols) * CELL
        y = (i // cols) * (CELL + PAD)
        canvas.paste(img, (x + 4, y + 4), img if img.mode == "RGBA" else None)
        label = fn.replace(sheet + "_", "").replace(".png", "")
        draw.text((x + 4, y + CELL - 2), label, fill=(255, 255, 120))
    canvas.save(os.path.join(OUT, f"contact_{sheet}.png"))
    print(f"contact_{sheet}.png ({len(files)} sprites)")
