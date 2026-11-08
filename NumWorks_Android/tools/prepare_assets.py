"""Genere src/wallpaper_data.c (depuis assets/fond_ecran.jpg) et assets/icon.png.
Usage : pip install pillow  puis  python tools/prepare_assets.py"""
import os, sys
from PIL import Image, ImageOps, ImageDraw

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
JPG = os.path.join(ROOT, "assets", "fond_ecran.jpg")
OUT_C = os.path.join(ROOT, "src", "wallpaper_data.c")
ICON = os.path.join(ROOT, "assets", "icon.png")
DARKEN = 0.62   # assombrit le fond pour la lisibilite des icones (1.0 = inchange)

def rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def make_wallpaper():
    if not os.path.exists(JPG):
        print("assets/fond_ecran.jpg absent : degrade integre utilise.")
        return
    im = Image.open(JPG).convert("RGB")
    im = ImageOps.fit(im, (320, 240), Image.LANCZOS, centering=(0.5, 0.5))
    px = [rgb565(int(r * DARKEN), int(g * DARKEN), int(b * DARKEN)) for r, g, b in im.getdata()]
    with open(OUT_C, "w") as f:
        f.write("/* Genere par tools/prepare_assets.py - ne pas editer */\n#include <stdint.h>\n")
        f.write("const int wallpaper_present = 1;\nconst uint16_t wallpaper_pixels[76800] = {\n")
        for i in range(0, len(px), 16):
            f.write(",".join("0x%04X" % v for v in px[i:i + 16]) + ",\n")
        f.write("};\n")
    print("Fond d'ecran ecrit :", OUT_C, "(153600 octets en flash)")

def make_icon():
    im = Image.new("RGBA", (55, 56), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle((1, 1, 53, 54), radius=14, fill=(32, 36, 52, 255))
    for x, y in ((12, 12), (30, 12), (12, 30), (30, 30)):
        d.rounded_rectangle((x, y, x + 13, y + 13), radius=4, fill=(61, 125, 255, 255))
    im.save(ICON)
    print("Icone ecrite :", ICON)

make_wallpaper()
make_icon()
