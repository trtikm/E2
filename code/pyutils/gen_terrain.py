import matplotlib.image as mpimg
import numpy as np
import os
import plot


def make_uniform_mono_image(width, height, value):
    return np.zeros((height, width)) + value


def make_random_mono_image(width, height, lo, hi):
    # The cast is only for PyCharm
    return np.array(lo + (hi - lo) * np.random.random((height, width)))


def load_mono_image(fname="./terrain.png"):
    if not os.path.isfile(fname):
        return None
    pixels = mpimg.imread(fname, "png")
    img = []
    for row in pixels:
        img.append([p[0] for p in row])
    return np.array(img)


def save_mono_image(pixels, fname="./terrain.png"):
    img = []
    for row in pixels:
        img.append([[x, x, x] for x in row])
    mpimg.imsave(fname, np.array(img), format="png")


def save_histogram(hist, fname="./terrain_hist.png", title="", xaxis_name="", yaxis_name=""):
    with plot.Plot(fname, title, xaxis_name, yaxis_name) as h:
        h.histogram(hist)


def get_minmax_image_values(img):
    lo = 100000000000.0
    hi = -100000000000.0
    for row in range(img.shape[0]):
        for column in range(img.shape[1]):
            lo = min(lo, img[row][column])
            hi = max(hi, img[row][column])
    return lo, hi


def make_histogram_from_image(img, nbuckets=256):
    assert nbuckets > 1
    lo, hi = get_minmax_image_values(img)
    all_same = abs(hi - lo) < 0.001
    h = {}
    for row in range(img.shape[0]):
        for column in range(img.shape[1]):
            bucket = 0 if all_same is True else int(((img[row][column] - lo) / (hi - lo)) * (nbuckets - 1) + 0.5)
            if bucket in h:
                h[bucket] += 1
            else:
                h[bucket] = 1
    return [(lo + (k / float(nbuckets - 1)) * (hi - lo), v) for k, v in h.items()]


def normalise_image(img):
    lo, hi = get_minmax_image_values(img)
    for row in range(img.shape[0]):
        for column in range(img.shape[1]):
            value = (img[row][column] - lo) / (hi - lo)
            img[row][column] = max(0.0, min(value, 1.0))
    return img


def process_ring_neighbourhood(
        img,
        row,
        column,
        func_per_cell=lambda v, s: v + s,
        func_finish=lambda s, n: s / n,
        row_radius=1,
        column_radius=1,
        use_modulo_coords=True,
        init_value=0.0
        ):
    assert row >= 0 and row < img.shape[0] and column >= 0 and column < img.shape[1]
    assert isinstance(row_radius, int) and row_radius > 0 and isinstance(column_radius, int) and column_radius > 0
    result = init_value
    for r_seek in range(-row_radius, row_radius + 1):
        for c_seek in range(-column_radius, column_radius + 1):
            if r_seek != 0 or c_seek != 0:
                if use_modulo_coords is True:
                    r = (row + r_seek + img.shape[0]) % img.shape[0]
                    c = (column + c_seek + img.shape[1]) % img.shape[1]
                    result = func_per_cell(img[r][c], result)
                else:
                    r = row + r_seek
                    c = column + c_seek
                    if r >= 0 and r < img.shape[0] and c >= 0 and c < img.shape[1]:
                        result = func_per_cell(img[r][c], result)
    return func_finish(result, (2 * row_radius + 1) * (2 * column_radius + 1) - 1)


def for_each_pixel(img, func=lambda r, c, p: p):
    for row in range(img.shape[0]):
        for column in range(img.shape[1]):
            img[row][column] = func(row, column, img[row][column])
    return img


def apply_average(img, row_radius=1, column_radius=1):
    return for_each_pixel(img, lambda r, c, _: process_ring_neighbourhood(img, r, c, row_radius=row_radius, column_radius=column_radius))


def amplify(img, exp=2.0):
    return for_each_pixel(img, lambda r, c, p: p**exp)


def apply_merge(img, merge_img, func_merge=lambda u, v: (u + v) / 2.0):
    assert img.shape == merge_img.shape
    return for_each_pixel(img, lambda r, c, p: func_merge(p, merge_img[r][c]))


def stretch_image(src_img, dst_img, blend_func=lambda src, dst: src):
    assert dst_img.shape[0] > 1 and dst_img.shape[1] > 1
    for r_dst in range(dst_img.shape[0]):
        for c_dst in range(dst_img.shape[1]):
            r01 = r_dst / float(dst_img.shape[0] - 1)
            c01 = c_dst / float(dst_img.shape[1] - 1)
            r_src = min(int(r01 * (src_img.shape[0] - 1) + 0.5), src_img.shape[0] - 1)
            c_src = min(int(c01 * (src_img.shape[1] - 1) + 0.5), src_img.shape[1] - 1)
            dst_img[r_dst][c_dst] = blend_func(src_img[r_src][c_src], dst_img[r_dst][c_dst])
    return dst_img


def make_terrain_seed_image(width, height):
    assert isinstance(width, int) and width > 0 and isinstance(height, int) and height > 0
    img = make_random_mono_image(width, height, 0.0, 1.0)
    for i in range(2):
        amplify(apply_average(amplify(img, 2), 3, 3), 1.5)
    return img


def make_terrain_image(
        width,
        height,
        base_width_mult=0.5,
        base_height_mult=0.5,
        nb_width=3,
        nb_height=3,
        merge_coef=0.01
        ):
    img = make_uniform_mono_image(width, height, 0.0)
    src_img = make_terrain_seed_image(int(base_width_mult * width), int(base_height_mult * height))
    stretch_image(src_img, img)
    apply_average(img, nb_width, nb_height)
    bump_img = make_random_mono_image(width, height, 0.0, 1.0)
    apply_average(bump_img, nb_width, nb_height)
    apply_merge(img, bump_img, lambda u, v: u + merge_coef * v)
    return normalise_image(img)


def _main():
    np.random.seed()
    img = make_terrain_image(100, 100)
    save_mono_image(normalise_image(img))
    save_histogram(make_histogram_from_image(img))


if __name__ == '__main__':
    _main()
