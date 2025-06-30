# https://gamedev.stackexchange.com/a/23705

import random
import math
from PIL import Image

for seed_i, seed in enumerate(['hero of law', 'hegend of lelda', 'objection hyah']):
    rand = random.Random(seed)

    perm = list(range(256))
    rand.shuffle(perm)
    perm += perm
    perm += perm
    perm += perm
    dirs = [(math.cos(a * 2.0 * math.pi / 256),
             math.sin(a * 2.0 * math.pi / 256))
             for a in range(256)]

    def noise(x, y, per_x, per_y):
        def surflet(grid_x, grid_y):
            dist_x = abs(x - grid_x)
            dist_y = abs(y - grid_y)
            poly_x = 1 - 6*dist_x**5 + 15*dist_x**4 - 10*dist_x**3
            poly_y = 1 - 6*dist_y**5 + 15*dist_y**4 - 10*dist_y**3
            hashed = perm[perm[int(grid_x) % per_x] + int(grid_y) % per_y]
            grad = (x-grid_x)*dirs[hashed][0] + (y-grid_y)*dirs[hashed][1]
            return poly_x * poly_y * grad

        return (surflet(int(x), int(y)) + surflet(int(x)+1, int(y)) +
                surflet(int(x), int(y)+1) + surflet(int(x)+1, int(y)+1))

    def fBm(x, y, per_x, per_y, octs):
        val = 0
        for o in range(octs):
            val += 0.5**o * noise(x*2**o, y*2**o, per_x*2**o, per_y*2**o)
        return val

    freq = 1/4
    octs = 5
    data = []
    w = 16
    h = 128
    for y in range(h):
        for x in range(w):
            data.append(fBm(x*freq, y*freq, int(w*freq), int(h*freq), octs))

    # normalize 0.5-1.0
    m1 = min(data)
    m2 = max(data)
    if seed_i in [0, 1]:
        low = 0.5
        high = 1.0
        data = [int((((x - m1) / (m2 - m1)) * (high - low) + low) * 0xff) for x in data]
    else:
        low = 0.0
        high = 1.0
        data = [int(((((x - m1) / (m2 - m1)) * (high - low) + low) ** 8) * 0xff) for x in data]

    # convert to RGB because the "L" format is weird
    if seed_i in [0, 1]:
        data = [x | (x << 8) | (x << 16) | 0xff000000 for x in data]
    else:
        data = [(x << 24) | 0xffffff for x in data]

    # convert to Image and save
    im = Image.new('RGBA', (w, h))
    im.putdata(data, w, h)
    im.save(f'texture_{seed_i}.png')

    # for preview: make three copies side-by-side so it tiles properly
    preview_scale = 128

    im2 = Image.new('RGB', (w * 3, h))
    im2.paste(im, (0, 0))
    im2.paste(im, (w, 0))
    im2.paste(im, (w * 2, 0))

    # then stretch them, and crop out the middle one to use as the preview
    preview = im2.resize((w * 3 * preview_scale, h), Image.Resampling.LANCZOS)
    preview = preview.crop((w * preview_scale, 0, w * 2 * preview_scale, h))
    preview.save(f'preview_{seed_i}.png')
