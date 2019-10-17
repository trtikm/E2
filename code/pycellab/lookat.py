import numpy
import math
import sys
import json
import time


def mkvec(coords):
    return numpy.array(coords)


def vdot(u, v):
    return numpy.dot(u, v)


def vlen(u):
    return math.sqrt(vdot(u, u))


def vunit(u):
    return (1.0 / vlen(u)) * u


def vangle(u, v):
    return math.acos(vdot(u, v) / (vlen(u) * vlen(v)))


def vdecompose(u, v):
    w = (vdot(u, v) / vdot(v, v)) * v
    return [w, u - w]


def vrand(lim=5.0):
    return mkvec([-lim + 2*lim*numpy.random.random(), -lim + 2*lim*numpy.random.random()])


numpy.random.seed(int(1000*time.time()) % 2**32-1)

dim = 2
P = [vrand(), vrand()]
Q = [vrand(), vrand()]
if len(sys.argv) > 1:
    if sys.argv[1].startswith("--random="):
        n = int(sys.argv[1][len("--random="):])
        assert n > 1
        P0 = vrand()
        P = [P0 + vrand(0.2) for _ in range(n)]
        Q0 = vrand()
        Q = [Q0 + vrand(1.0) for _ in range(n)]
    else:
        with open(sys.argv[1], "r") as f:
            config = json.load(f)
            dim = config["dim"]
            P = [mkvec(v) for v in config["P"]]
            Q = [mkvec(v) for v in config["Q"]]
V = [vunit(Q[k] - P[k]) for k in range(len(P))]


def initX():
    C = []
    for k in range(len(P)):
        for l in range(k+1, len(P)):
            S = 0.5 * (P[k] + P[l])
            w = 0.5 * (V[k] + V[l])
            d = 0.5 * (vlen(vdecompose(P[k]-S, w)[1]) + vlen(vdecompose(P[l]-S, w)[1]))
            a = 0.5 * (vangle(V[k], w) + vangle(V[l], w))
            t = d / math.tan(a)
            C.append(S + t * w)
    return sum(C[k] for k in range(len(C))) / len(C)


X0 = initX()
X = X0


def A(k):
    return vdot(V[k], X) - vdot(V[k], P[k])


def B(k):
    return vdot(X - P[k], X - P[k])


def F():
    w = 0.0
    for k in range(len(P)):
        w += pow(A(k), 2) / B(k)
    return w


def dF(k, j):
    a = A(k)
    b = B(k)
    assert abs(b) > 0.0001
    return (2*a / (b*b)) * (V[k][j]*b - (X[j] - P[k][j])*a)
    # return V[k][j] / A(k) - (X[j] - P[k][j]) / B(k)


def gradF():
    g = []
    for j in range(dim):
        w = 0.0
        for k in range(len(P)):
            w += dF(k, j)
        g.append(w)
    return numpy.array(g)


n = 0
E = 0
while True:
    E = F()
    if n > 50 or E > len(P) - 0.000001:
        break
    G = gradF()
    t = 0.5
    X = X + t * G
    n += 1


coord_names = ["x", "y", "z", "w"]
out_data = {
    "num_dimensions": dim,
    "points": [],
    "X0": list(X0),
    "X": list(X),
    "num_iterations": n,
    "error": len(P) - E
}
for k in range(len(P)):
    out_data["points"].append({coord_names[j]: P[k][j] for j in range(dim)})
    out_data["points"].append({coord_names[j]: Q[k][j] for j in range(dim)})
SZ = 0.025
for v in [mkvec([-SZ, -SZ]), mkvec([SZ, SZ]), mkvec([SZ, -SZ]), mkvec([-SZ, SZ])]:
    out_data["points"].append({coord_names[j]: (X0+v)[j] for j in range(dim)})
for v in [mkvec([0.0, -SZ]), mkvec([0.0, SZ]), mkvec([-SZ, 0.0]), mkvec([SZ, 0.0])]:
    out_data["points"].append({coord_names[j]: (X+v)[j] for j in range(dim)})

if len(sys.argv) > 2:
    with open(sys.argv[2], "w") as f:
        json.dump(out_data, f, indent=4, sort_keys=True)
else:
    print(json.dumps(out_data, indent=4, sort_keys=True))
