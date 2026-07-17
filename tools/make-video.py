#!/usr/bin/env python3
"""Build a short promo video from the Whatly feature cards.

A 720p slideshow: each card gets a slow zoom (Ken Burns) and cards crossfade
into one another, over a soft, procedurally-generated ambient pad. Because the
music is synthesised here from sine waves it is entirely royalty-free — swap in
a track of your own (e.g. from the YouTube Audio Library) before publishing if
you want something richer.

    python3 tools/make-video.py [--out whatly-intro.mp4]

The snapcraft.io listing's "Video" field takes a URL (YouTube/Vimeo), not a
file — upload the .mp4 there and paste the link.

Requires: ffmpeg.
"""
import os, sys, subprocess, argparse

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
IMG = os.path.join(REPO, "docs/img")

# Narrative order — intro, the hero screens, the headline features, then the
# "available everywhere" outro.
SEQUENCE = [
    "banner.png",
    "card-main-dark.png", "card-main-light.png",
    "card-chat.png", "card-themes.png", "card-scheduled.png",
    "card-accounts.png", "card-lock.png", "card-spellcheck.png",
    "card-shortcuts.png", "card-tray.png", "card-notifications.png",
    "card-installers.png",
]

D = 3.0     # seconds each card is shown (including its outgoing transition)
T = 0.7     # crossfade duration
W, H, FPS = 1280, 720, 30

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=os.path.join(REPO, "whatly-intro.mp4"))
    a = ap.parse_args()

    imgs = [os.path.join(IMG, n) for n in SEQUENCE if os.path.exists(os.path.join(IMG, n))]
    n = len(imgs)
    if n < 2:
        sys.exit("need at least two cards")
    total = n * D - (n - 1) * T
    zf = int((D + T) * FPS)  # frames per still (a little longer than shown)

    cmd = ["ffmpeg", "-y"]
    for p in imgs:
        cmd += ["-loop", "1", "-t", f"{D:.3f}", "-i", p]

    fc = []
    # Each still: fit to frame on a dark backdrop, then a slow zoom-in.
    for i in range(n):
        fc.append(
            f"[{i}:v]scale={W}:{H}:force_original_aspect_ratio=decrease,"
            f"pad={W}:{H}:(ow-iw)/2:(oh-ih)/2:color=0x0b3d38,setsar=1,fps={FPS},"
            f"zoompan=z='min(zoom+0.0008,1.10)':d={zf}:s={W}x{H}:fps={FPS},"
            f"format=yuv420p[v{i}]")
    # Crossfade chain.
    prev = "v0"
    for i in range(1, n):
        out = f"x{i}" if i < n - 1 else "vout"
        off = i * (D - T)
        fc.append(f"[{prev}][v{i}]xfade=transition=fade:duration={T}:offset={off:.3f}[{out}]")
        prev = out

    # Royalty-free ambient pad: a soft A-major-ish stack of sines with a slow
    # tremolo, a little echo for space, gentle low-pass, and fade in/out.
    chord = "+".join(f"{amp}*sin(2*PI*{f}*t)" for f, amp in
                     [(110, 0.12), (164.81, 0.10), (220, 0.09), (277.18, 0.07), (329.63, 0.05)])
    fc.append(
        f"aevalsrc='{chord}':s=44100:d={total:.3f}[a0];"
        f"[a0]tremolo=f=0.15:d=0.4,aecho=0.8:0.7:400|700:0.3|0.2,"
        f"lowpass=f=2600,afade=t=in:st=0:d=1.5,"
        f"afade=t=out:st={total-2:.3f}:d=2,volume=0.7[aout]")

    cmd += ["-filter_complex", ";".join(fc),
            "-map", "[vout]", "-map", "[aout]",
            "-c:v", "libx264", "-pix_fmt", "yuv420p", "-r", str(FPS),
            "-c:a", "aac", "-b:a", "160k", "-movflags", "+faststart",
            "-shortest", a.out]

    print(f"==> {n} cards, {total:.1f}s ->", os.path.relpath(a.out, REPO))
    subprocess.run(cmd, check=True)
    print("done:", a.out)

if __name__ == "__main__":
    main()
