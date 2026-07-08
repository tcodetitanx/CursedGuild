"""Slice AI-generated sprite sheets (neon pink #FF00FF background) into
individual transparent PNGs for Cursed Guild.

- Chroma-keys pixels near #FF00FF to transparent (soft edge + despill).
- Recursively splits each sheet along fully-magenta gutter rows/columns.
- Trims each cell to its content bounding box.

Output: Content/Art/Sprites/<sheet>/<sheet>_r<row>_c<col>.png
"""
import os
import numpy as np
from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
IMG_DIR = os.path.join(ROOT, "images")
OUT_ROOT = os.path.join(ROOT, "Content", "Art", "Sprites")

SHEETS = {
    "ui_frames_a":  "ChatGPT Image Jul 8, 2026, 12_40_42 PM (1).png",
    "icons":        "ChatGPT Image Jul 8, 2026, 12_40_43 PM (3).png",
    "bodies":       "ChatGPT Image Jul 8, 2026, 12_45_45 PM (1).png",
    "creatures":    "ChatGPT Image Jul 8, 2026, 12_45_45 PM (2).png",
    "enemies":      "ChatGPT Image Jul 8, 2026, 12_45_45 PM (3).png",
    "relics":       "ChatGPT Image Jul 8, 2026, 12_45_46 PM (4).png",
    "ui_frames_b":  "ChatGPT Image Jul 8, 2026, 12_45_46 PM (5).png",
}
# portraits sheet: explicit cell layout (white grid lines, uneven rows;
# row 4 holds 9 small enemy tiles). (y0, y1, [x boundaries])
SIX = [0, 209, 418, 626, 835, 1043, 1254]
NINE = [round(i * 1254 / 9) for i in range(10)]
PORTRAIT_SHEET = ("ChatGPT Image Jul 8, 2026, 12_40_43 PM (2).png", [
    (0, 237, SIX),
    (237, 460, SIX),
    (460, 665, SIX),
    (665, 860, NINE),
    (860, 1055, SIX),
    (1055, 1254, SIX),
])
BACKGROUND = "ChatGPT Image Jul 8, 2026, 12_40_43 PM (4).png"


def magenta_mask(arr):
    """Boolean mask of pixels considered background magenta."""
    r = arr[..., 0].astype(np.int32)
    g = arr[..., 1].astype(np.int32)
    b = arr[..., 2].astype(np.int32)
    d2 = (r - 255) ** 2 + g ** 2 + (b - 255) ** 2
    return d2 < 130 ** 2


def chroma_key(img):
    """RGBA image with magenta removed (soft alpha + despill)."""
    arr = np.array(img.convert("RGB"), dtype=np.uint8)
    r = arr[..., 0].astype(np.float32)
    g = arr[..., 1].astype(np.float32)
    b = arr[..., 2].astype(np.float32)
    d = np.sqrt((r - 255) ** 2 + g ** 2 + (b - 255) ** 2)
    # alpha ramp: fully transparent below t0, opaque above t1
    t0, t1 = 100.0, 210.0
    a = np.clip((d - t0) / (t1 - t0), 0.0, 1.0)
    # despill: on semi-transparent fringe pull red/blue toward green level
    fringe = a < 1.0
    cap = g + 60.0
    r2 = np.where(fringe, np.minimum(r, cap), r)
    b2 = np.where(fringe, np.minimum(b, cap), b)
    out = np.stack([r2, g, b2, a * 255.0], axis=-1).astype(np.uint8)
    return Image.fromarray(out, "RGBA")


def find_segments(is_content_line, min_size=10, min_gutter=4):
    """Split a boolean line-profile into content segments separated by gutters."""
    segs = []
    n = len(is_content_line)
    i = 0
    while i < n:
        if is_content_line[i]:
            j = i
            while j < n and is_content_line[j]:
                j += 1
            if j - i >= min_size:
                segs.append((i, j))
            i = j
        else:
            i += 1
    # merge segments separated by tiny gutters (anti-noise)
    merged = []
    for s in segs:
        if merged and s[0] - merged[-1][1] < min_gutter:
            merged[-1] = (merged[-1][0], s[1])
        else:
            merged.append(list(s))
    return [tuple(m) for m in merged]


def split_recursive(mask, box, axis, depth, cells):
    """mask: True where content (non-magenta). box=(x0,y0,x1,y1)."""
    x0, y0, x1, y1 = box
    sub = mask[y0:y1, x0:x1]
    if axis == 0:  # split into rows
        profile = sub.mean(axis=1) > 0.008
    else:          # split into columns (higher threshold: touching sprites)
        profile = sub.mean(axis=0) > 0.02
    segs = find_segments(profile)
    if not segs:
        return
    if len(segs) == 1 and depth > 0:
        s0, s1 = segs[0]
        if axis == 0:
            nbox = (x0, y0 + s0, x1, y0 + s1)
        else:
            nbox = (x0 + s0, y0, x1 + 0 + s1 - (x1 - x0) + (x1 - x0), y1)
            nbox = (x0 + s0, y0, x0 + s1, y1)
        if depth >= 4 or nbox == box:
            cells.append(nbox)
        else:
            split_recursive(mask, nbox, 1 - axis, depth + 1, cells)
        return
    for (s0, s1) in segs:
        if axis == 0:
            nbox = (x0, y0 + s0, x1, y0 + s1)
        else:
            nbox = (x0 + s0, y0, x0 + s1, y1)
        if depth >= 4:
            cells.append(nbox)
        else:
            split_recursive(mask, nbox, 1 - axis, depth + 1, cells)


def slice_sheet(name, filename):
    path = os.path.join(IMG_DIR, filename)
    img = Image.open(path).convert("RGB")
    arr = np.array(img)
    content = ~magenta_mask(arr)
    cells = []
    split_recursive(content, (0, 0, img.width, img.height), 0, 0, cells)

    # group cells into rows by y-center so naming is stable r<row>_c<col>
    cells.sort(key=lambda c: (c[1], c[0]))
    rows = []
    for c in cells:
        cy = (c[1] + c[3]) / 2
        placed = False
        for row in rows:
            ry = sum((cc[1] + cc[3]) / 2 for cc in row) / len(row)
            if abs(cy - ry) < max(40, (c[3] - c[1]) * 0.5):
                row.append(c)
                placed = True
                break
        if not placed:
            rows.append([c])
    rows.sort(key=lambda row: min(c[1] for c in row))

    # subdivide cells that clearly merged neighbours (wider than 1.7x row median)
    for row in rows:
        row.sort(key=lambda c: c[0])
        widths = sorted(c[2] - c[0] for c in row)
        med = widths[len(widths) // 2]
        fixed = []
        for (x0, y0, x1, y1) in row:
            w = x1 - x0
            n = max(1, round(w / med)) if w > med * 1.7 else 1
            for k in range(n):
                fixed.append((x0 + w * k // n, y0, x0 + w * (k + 1) // n, y1))
        row[:] = fixed

    keyed = chroma_key(img)
    out_dir = os.path.join(OUT_ROOT, name)
    os.makedirs(out_dir, exist_ok=True)
    count = 0
    for ri, row in enumerate(rows, 1):
        row.sort(key=lambda c: c[0])
        for ci, (x0, y0, x1, y1) in enumerate(row, 1):
            crop = keyed.crop((x0, y0, x1, y1))
            # trim to alpha bbox
            bbox = crop.getchannel("A").getbbox()
            if bbox is None:
                continue
            crop = crop.crop(bbox)
            if crop.width < 12 or crop.height < 12:
                continue
            crop.save(os.path.join(out_dir, f"{name}_r{ri}_c{ci}.png"))
            count += 1
    print(f"{name}: {count} sprites ({len(rows)} rows) from {img.width}x{img.height}")


def slice_layout(name, filename, layout, inset=7):
    """Slice using explicit row/column boundaries; chroma-key each cell.
    Cells are inset a few px to drop the white grid border lines."""
    path = os.path.join(IMG_DIR, filename)
    img = Image.open(path).convert("RGB")
    keyed = chroma_key(img)
    out_dir = os.path.join(OUT_ROOT, name)
    os.makedirs(out_dir, exist_ok=True)
    count = 0
    for ri, (y0, y1, xs) in enumerate(layout, 1):
        for ci in range(len(xs) - 1):
            box = (xs[ci] + inset, y0 + inset, xs[ci + 1] - inset, y1 - inset)
            crop = keyed.crop(box)
            bbox = crop.getchannel("A").getbbox()
            if bbox is None:
                continue
            crop = crop.crop(bbox)
            if crop.width < 12 or crop.height < 12:
                continue
            crop.save(os.path.join(out_dir, f"{name}_r{ri}_c{ci+1}.png"))
            count += 1
    print(f"{name}: {count} sprites (layout)")


def main():
    os.makedirs(OUT_ROOT, exist_ok=True)
    for name, fn in SHEETS.items():
        slice_sheet(name, fn)
    slice_layout("portraits", PORTRAIT_SHEET[0], PORTRAIT_SHEET[1])
    # background: copy straight through (no chroma key)
    bg_dir = os.path.join(ROOT, "Content", "Art", "Backgrounds")
    os.makedirs(bg_dir, exist_ok=True)
    Image.open(os.path.join(IMG_DIR, BACKGROUND)).convert("RGB").save(
        os.path.join(bg_dir, "guild_hall.png"))
    print("background: guild_hall.png")


if __name__ == "__main__":
    main()
