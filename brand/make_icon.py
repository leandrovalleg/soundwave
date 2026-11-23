# Dot-matrix W-wave icon. usage: python3 make_icon.py N out.svg   (N = dots per side)
import math, sys
N = int(sys.argv[1]); out = sys.argv[2]
size, pad = 512, 72
step = (size - 2 * pad) / (N - 1)
dots = []
for i in range(N):
    for j in range(N):
        x, y = i / (N - 1), j / (N - 1)
        d = min(math.hypot((x - t) * (N - 1), (y - (0.5 - 0.36 * math.cos(4 * math.pi * t))) * (N - 1))
                for t in [k / 400 for k in range(401)])
        lit = d < 0.62 and 0.06 <= x <= 0.94
        cx, cy = pad + i * step, pad + j * step
        dots.append(f'<circle cx="{cx:.1f}" cy="{cy:.1f}" r="{step * (0.38 if lit else 0.12):.1f}" fill="{"#111111" if lit else "#D6D6D6"}"/>')
red = f'<circle cx="{pad + (N - 1) * step:.1f}" cy="{pad:.1f}" r="{step * 0.38:.1f}" fill="#D71921"/>'
open(out, "w").write(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {size} {size}"><rect width="{size}" height="{size}" rx="112" fill="#FFFFFF"/>{"".join(dots)}{red}</svg>')
