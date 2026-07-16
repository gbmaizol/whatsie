#!/usr/bin/env python3
"""Compose Whatly README 'feature cards': a teal-gradient background + a
rounded, shadowed screenshot + the logo + a bold feature title. This is the
promo style used in the README; run it after capturing fresh screenshots.

Usage:
    python3 tools/make-cards.py [--shots DIR] [--out DIR]

--shots  directory holding the raw PNG screenshots (default tools/shots)
--out    where to write the cards (default docs/img)

Expected screenshots in --shots (any missing one is skipped):
    settings.png   the Settings dialog
    about.png      the About dialog
    chat.png       the main window / chat list (ideally with the privacy blur on)

Requires: Pillow  (pip install pillow) and a DejaVu Sans font.
"""
import os, sys, argparse
from PIL import Image, ImageDraw, ImageFont, ImageFilter

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LOGO = os.path.join(REPO, "dist/linux/hicolor/256x256/apps/net.shakaran.whatly.png")

# Brand palette (teal). Change these to re-skin every card.
C1 = (13, 148, 136)     # #0d9488 gradient top
C2 = (6, 78, 70)        # #064e46 gradient bottom
ACCENT = (45, 212, 191) # #2dd4bf underline
RIM = (243, 243, 243)

def _font(bold, size):
    for p in ("/usr/share/fonts/truetype/dejavu/DejaVuSans%s.ttf" % ("-Bold" if bold else ""),
              "/usr/share/fonts/truetype/liberation/LiberationSans%s.ttf" % ("-Bold" if bold else "-Regular")):
        if os.path.exists(p):
            return ImageFont.truetype(p, size)
    return ImageFont.load_default()

def lerp(a, b, t):
    return tuple(round(a[i] + (b[i] - a[i]) * t) for i in range(3))

def gradient(W, H):
    g = Image.new("RGB", (W, H))
    d = ImageDraw.Draw(g)
    for y in range(H):
        d.line([(0, y), (W, y)], fill=lerp(C1, C2, y / (H - 1)))
    streak = Image.new("L", (W, H), 0)
    ImageDraw.Draw(streak).polygon([(W*0.55,0),(W,0),(W,H),(W*0.30,H)], fill=40)
    streak = streak.filter(ImageFilter.GaussianBlur(120)).point(lambda v: int(v*0.35))
    return Image.composite(Image.new("RGB",(W,H),(255,255,255)), g, streak)

def trim_transparent(img):
    """Drop a window screenshot's semi-transparent drop-shadow border, so the
    rounded frame does not turn it into an opaque black edge. No-op for opaque
    images (e.g. CDP web captures)."""
    img = img.convert("RGBA")
    a = img.split()[3]
    # Keep only near-opaque pixels: a window screenshot's drop shadow fades from
    # translucent to transparent, so a low threshold leaves its darkest band as a
    # black-looking edge. 200 cuts the whole shadow and keeps just the window.
    bbox = a.point(lambda v: 255 if v > 200 else 0).getbbox()
    return img.crop(bbox) if bbox else img

def rounded_shadow(img, radius=22, pad=60, blur=34, alpha=150):
    w, h = img.size
    card = img.convert("RGBA")
    mask = Image.new("L", (w, h), 0)
    ImageDraw.Draw(mask).rounded_rectangle([0, 0, w-1, h-1], radius=radius, fill=255)
    card.putalpha(mask)
    canvas = Image.new("RGBA", (w+pad*2, h+pad*2), (0, 0, 0, 0))
    sh = Image.new("RGBA", canvas.size, (0, 0, 0, 0))
    ImageDraw.Draw(sh).rounded_rectangle([pad, pad+10, pad+w, pad+h+10], radius=radius, fill=(0,0,0,alpha))
    canvas.alpha_composite(sh.filter(ImageFilter.GaussianBlur(blur)))
    canvas.alpha_composite(card, (pad, pad))
    return canvas

def wrap(draw, text, font, maxw):
    words, lines, cur = text.split(), [], ""
    for w in words:
        t = (cur + " " + w).strip()
        if draw.textlength(t, font=font) <= maxw:
            cur = t
        else:
            lines.append(cur); cur = w
    if cur:
        lines.append(cur)
    return lines

def card(out, shot_path, title, subtitle, W=1280, H=720):
    bg = gradient(W, H).convert("RGBA")
    d = ImageDraw.Draw(bg)
    shot = trim_transparent(Image.open(shot_path))
    r = min(int(W*0.52)/shot.width, (H-150)/shot.height)
    shot = shot.resize((int(shot.width*r), int(shot.height*r)), Image.LANCZOS)
    framed = rounded_shadow(shot)
    bg.alpha_composite(framed, (int(W*0.05)-40, (H-framed.height)//2))
    logo = Image.open(LOGO).convert("RGBA").resize((104, 104), Image.LANCZOS)
    tx = int(W*0.60)
    bg.alpha_composite(logo, (tx, 60))
    d.text((tx+120, 78), "Whatly", font=_font(True, 52), fill=(255, 255, 255))
    tfont, sfont = _font(True, 62), _font(False, 30)
    y = 250
    for ln in wrap(d, title, tfont, W-tx-70):
        d.text((tx, y), ln, font=tfont, fill=(255, 255, 255)); y += 74
    d.rounded_rectangle([tx, y+8, tx+120, y+16], radius=4, fill=ACCENT); y += 42
    for ln in wrap(d, subtitle, sfont, W-tx-70):
        d.text((tx, y), ln, font=sfont, fill=(200, 240, 235)); y += 40
    bg.convert("RGB").save(out); print("  wrote", os.path.relpath(out, REPO))

def banner(out, W=1280, H=380):
    bg = gradient(W, H).convert("RGBA")
    d = ImageDraw.Draw(bg)
    lsz, gap, tf = 150, 28, _font(True, 96)
    tw = d.textlength("Whatly", font=tf)
    x0 = (W - (lsz + gap + tw)) // 2
    cy = int(H*0.42)
    bg.alpha_composite(Image.open(LOGO).convert("RGBA").resize((lsz, lsz), Image.LANCZOS), (int(x0), int(cy-lsz/2)))
    tb = d.textbbox((0, 0), "Whatly", font=tf)
    d.text((x0+lsz+gap, cy-(tb[3]-tb[1])/2-tb[1]), "Whatly", font=tf, fill=(255, 255, 255))
    st, sf = "A feature-rich desktop client for WhatsApp Web", _font(False, 34)
    d.text(((W-d.textlength(st, font=sf))//2, int(H*0.72)), st, font=sf, fill=(205, 242, 237))
    bg.convert("RGB").save(out); print("  wrote", os.path.relpath(out, REPO))

import colorsys
# Chat themes (name, hue) — mirrors src/chattheme.cpp. None = WhatsApp green.
THEMES = [("WhatsApp", None), ("Barbie pink", 335), ("Dusty rose", 345), ("Lavender", 270),
          ("Violet", 292), ("Sky blue", 205), ("Deep ocean", 222), ("Teal", 186), ("Mint", 152),
          ("Coral", 8), ("Peach", 24), ("Gold", 44), ("Crimson", 352), ("Graphite", 220)]

def _theme_color(h):
    if h is None:
        return (37, 211, 102)             # WhatsApp green
    l, s = (0.32, 0.10) if h == 220 else (0.55, 0.62)   # graphite is a grey
    return tuple(int(x*255) for x in colorsys.hls_to_rgb(h/360, l, s))

def themes_panel(out, W=660, H=720, ss=2):
    W, H = W*ss, H*ss
    im = Image.new("RGB", (W, H), (24, 32, 34)); d = ImageDraw.Draw(im)
    d.text((30*ss, 24*ss), "Chat themes", font=_font(True, 30*ss), fill=(235, 245, 243))
    pad, top, cols, rows = 30*ss, 90*ss, 2, 7
    cw = (W-pad*2-20*ss)//cols; ch = (H-top-pad)//rows
    for i, (name, h) in enumerate(THEMES):
        x = pad + (i % cols)*(cw+20*ss); y = top + (i//cols)*ch
        d.rounded_rectangle([x, y, x+cw, y+ch-12*ss], radius=14*ss, fill=(15, 20, 22))
        sw = 54*ss; sy = y+(ch-12*ss-sw)//2
        d.rounded_rectangle([x+14*ss, sy, x+14*ss+sw, sy+sw], radius=12*ss, fill=_theme_color(h))
        d.text((x+14*ss+sw+16*ss, y+(ch-12*ss)//2-16*ss), name, font=_font(True, 21*ss), fill=(225, 235, 233))
    im.resize((W//ss, H//ss), Image.LANCZOS).save(out)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--shots", default=os.path.join(REPO, "tools/shots"))
    ap.add_argument("--out", default=os.path.join(REPO, "docs/img"))
    a = ap.parse_args()
    os.makedirs(a.out, exist_ok=True)
    banner(os.path.join(a.out, "banner.png"))
    # Chat-themes card is drawn from the theme list, not captured.
    tp = os.path.join(a.shots, "themes.png"); os.makedirs(a.shots, exist_ok=True)
    themes_panel(tp)
    card(os.path.join(a.out, "card-themes.png"), tp, "Fourteen chat themes",
         "Recolour WhatsApp Web itself — pick a hue, keep your photos and avatars untouched.")
    jobs = [
        ("chat.png",     "card-chat.png",     "Themes and a privacy blur",
         "Recolour WhatsApp Web with 14 chat themes, and blur the chats until you hover so nobody reads over your shoulder."),
        ("settings.png", "card-settings.png", "Every feature is a setting",
         "Themes, wallpaper, privacy blur, spell-check, tray and more — all toggles."),
        ("about.png",    "card-about.png",    "Report a bug in one click",
         "F1 opens About; the bug report is pre-filled with version, memory and logs."),
    ]
    for shot, name, title, sub in jobs:
        p = os.path.join(a.shots, shot)
        if os.path.exists(p):
            card(os.path.join(a.out, name), p, title, sub)
        else:
            print("  skip", name, "(missing", os.path.relpath(p, REPO) + ")")

if __name__ == "__main__":
    main()
