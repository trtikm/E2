import matplotlib.image as mpimg
import numpy as np
import os


def load_mono_image(fname="./terrain.png"):
    if not os.path.isfile(fname):
        return None
    pixels = mpimg.imread(fname, "png")
    img = []
    for row in pixels:
        img.append([p[0] for p in row])
    return np.array(img)


def generate_batch(img, dx=1.0, dy=1.0, z_zero=0.0, z_scale=10, dname="./terrain_batch"):
    assert img.shape[0] > 1 and img.shape[1] > 1
    batch_fname = os.path.join(dname, "batches", "terrain.txt")
    vertices_fname = os.path.join(dname, "meshes", "terrain", "vertices.txt")
    indices_fname = os.path.join(dname, "meshes", "terrain", "indices.txt")
    normals_fname = os.path.join(dname, "meshes", "terrain", "normals.txt")
    diffuse_fname = os.path.join(dname, "meshes", "terrain", "diffuse.txt")
    os.makedirs(os.path.dirname(batch_fname), exist_ok=True)
    os.makedirs(os.path.dirname(vertices_fname), exist_ok=True)

    def reverse_rows(img):
        result = []
        for row in range(img.shape[0]):
            result.append([img[row][column] for column in range(img.shape[1])])
        result.reverse()
        return np.array(result)

    def get_vertex(iX):
        r01 = min(iX[0] / float(img.shape[0] - 1), 1.0)
        c01 = min(iX[1] / float(img.shape[1] - 1), 1.0)
        x = (c01 - 0.5) * (dx * img.shape[1])
        y = (r01 - 0.5) * (dy * img.shape[0])
        z = (img[iX[0]][iX[1]] - z_zero) * z_scale
        return np.array([x, y, z])

    def valid(iX):
        return iX[0] >= 0 and iX[0] < img.shape[0] and iX[1] >= 0 and iX[1] < img.shape[1]

    def normalised(u):
        return (1.0 / np.sqrt(np.dot(u, u))) * u

    img = reverse_rows(img)

    with open(vertices_fname, "w") as f:
        f.write(str(img.size) + "\n")
        for row in range(img.shape[0]):
            for column in range(img.shape[1]):
                X = get_vertex((row, column))
                for i in range(3):
                    f.write(str(X[i]) + "\n")

    with open(indices_fname, "w") as f:
        f.write("3\n")
        f.write(str(2 * (img.shape[0] - 1) * (img.shape[1] - 1)) + "\n")
        for row in range(img.shape[0] - 1):
            for column in range(img.shape[1] - 1):
                iA = (row + 0) * img.shape[1] + (column + 0)
                iB = (row + 1) * img.shape[1] + (column + 0)
                iC = (row + 1) * img.shape[1] + (column + 1)
                iD = (row + 0) * img.shape[1] + (column + 1)
                f.write(str(iA) + "\n")
                f.write(str(iC) + "\n")
                f.write(str(iB) + "\n")
                f.write(str(iA) + "\n")
                f.write(str(iD) + "\n")
                f.write(str(iC) + "\n")

    with open(normals_fname, "w") as f:
        f.write(str(img.size) + "\n")
        for row in range(img.shape[0]):
            for column in range(img.shape[1]):
                normal = np.array([0.0, 0.0, 0.0])
                A = get_vertex((row, column))
                coords = [(-1, 0), (-1, -1), (0, -1), (1, 0), (1, 1), (0, 1), (-1, 0)]
                for i in range(len(coords) - 1):
                    iB = (row + coords[i][0], column + coords[i][1])
                    iC = (row + coords[i+1][0], column + coords[i+1][1])
                    if valid(iB) and valid(iC):
                        B = get_vertex(iB)
                        C = get_vertex(iC)
                        u = C - A
                        v = B - A
                        w = normalised(np.cross(u, v))
                        normal += w
                normal = normalised(normal)
                for i in range(3):
                    f.write(str(normal[i]) + "\n")

    with open(diffuse_fname, "w") as f:
        f.write("4\n")
        f.write(str(img.size) + "\n")
        for row in range(img.shape[0]):
            for column in range(img.shape[1]):
                value = img[row][column]
                lo = 0.2
                hi = 0.8
                if value < 1.0 / 4.0:
                    U = [lo, lo, lo]
                    V = [lo, lo, hi]
                    value /= (1.0 / 4.0)
                elif value < 2.0 / 4.0:
                    U = [lo, lo, hi]
                    V = [lo, hi, lo]
                    value = (value - (1.0 / 4.0)) / (1.0 / 4.0)
                elif value < 3.0 / 4.0:
                    U = [lo, hi, lo]
                    V = [hi, hi, lo]
                    value = (value - (2.0 / 4.0)) / (1.0 / 4.0)
                else:
                    U = [hi, hi, lo]
                    V = [hi, hi, hi]
                    value = (value - (3.0 / 4.0)) / (1.0 / 4.0)
                for i in range(3):
                    w = max(0.0, min(U[i] + value * (V[i] - U[i]), 1.0))
                    f.write(str(w) + "\n")
                f.write("1.0\n")

    with open(batch_fname, "w") as f:
        f.write(
"""
mesh meshes/terrain
skins
{
    default
    {
        draw_state
        {
           use_alpha_blending              false
           alpha_blending_src_function     SRC_ALPHA
           alpha_blending_dst_function     ONE_MINUS_CONSTANT_ALPHA
           cull_face_mode                  BACK
        }
        alpha_testing
        {
           use_alpha_testing               false
           alpha_test_constant             0.0
        }
    }
}
"""
        )


def _main():
    img = load_mono_image()
    assert img is not None
    generate_batch(img)


if __name__ == '__main__':
    _main()
