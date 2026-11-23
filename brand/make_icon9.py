# 9-dot icon variants. usage: python3 make_icon9.py VARIANT out.svg
import math, sys
v = sys.argv[1]; out = sys.argv[2]
N, size, pad = 9, 512, 80
step = (size - 2 * pad) / (N - 1)
bg, on, off, red = "#FFFFFF", "#111111", "#D6D6D6", "#D71921"
if v in ("dark", "dark-red"): bg, on, off = "#111111", "#FFFFFF", "#3A3A3A"
def curve(t):
    if v == "sharp":   # triangle W: straight strokes
        u = (4 * t) % 2; return 0.14 + 0.72 * (u if u <= 1 else 2 - u) if True else 0
    return 0.5 - 0.36 * math.cos(4 * math.pi * t)
dots = []
for i in range(N):
    for j in range(N):
        x, y = i / (N - 1), j / (N - 1)
        d = min(math.hypot((x - t) * (N - 1), (y - curve(t)) * (N - 1)) for t in [k / 400 for k in range(401)])
        lit = d < (0.5 if v == "thin" else 0.62) and 0.06 <= x <= 0.94
        cx, cy = pad + i * step, pad + j * step
        col = (red if v == "red-wave" else on) if lit else off
        if v == "dark-red" and lit: col = red
        r = step * (0.40 if lit else 0.12)
        if v == "square":
            s = r * 1.7; dots.append(f'<rect x="{cx-s/2:.1f}" y="{cy-s/2:.1f}" width="{s:.1f}" height="{s:.1f}" rx="{s*0.18:.1f}" fill="{col}"/>')
        elif v == "nogrid" and not lit: continue
        else: dots.append(f'<circle cx="{cx:.1f}" cy="{cy:.1f}" r="{r:.1f}" fill="{col}"/>')
acc = "" if v in ("red-wave", "dark-red") else f'<circle cx="{pad + (N - 1) * step:.1f}" cy="{pad:.1f}" r="{step * 0.40:.1f}" fill="{red}"/>'
open(out, "w").write(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {size} {size}"><rect width="{size}" height="{size}" rx="112" fill="{bg}"/>{"".join(dots)}{acc}</svg>')
