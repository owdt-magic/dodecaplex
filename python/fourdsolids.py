import numpy as np
from math import isclose
from mathhelpers import *

PHI = (1+(5**0.5))/2
EDGE_120CELL = 3 - (5**0.5)
EDGE_600CELL = 1/PHI
RADIUS_120CELL = (6-(5**0.5))**0.5
DISTANCE_600CELL = PHI-1
#LATENT_WARP = PHI/2

DODECAPLEX_POINTS = [
    (0,       0,              2,          2           ),
    (PHI,     PHI,            PHI,        (PHI**(-2)) ),
    (1,       1,              1,          (5**(0.5))  ),
    ((1/PHI), (1/PHI),        (1/PHI),    (PHI*PHI)   ),
    (0,       (1/PHI),        PHI,        (5**(0.5))  ),
    (0,       (PHI**(-2)),    1,          (PHI**2)    ),
    ((1/PHI), 1,              PHI,        2           ),
]     # https://en.wikipedia.org/wiki/120-cell

TETRAPLEX_POINTS = [
    (0,      0,          0,      1   ),
    (0.5,    0.5,        0.5,    0.5 ),
    (0,      (0.5*PHI),  0.5,    (0.5/PHI))
]    # https://en.wikipedia.org/wiki/600-cell

D_EVENS = [0,0,0,0,1,1,1] 
T_EVENS = [0,0,1]

def test_initialization():
    for point, length, even in zip(DODECAPLEX_POINTS, [24, 64, 64, 64, 96, 96, 192], D_EVENS):
        assert len(unsigned_permutations(point, even=even)) == length
    for point, length, even in zip(TETRAPLEX_POINTS, [8, 16, 96], T_EVENS):
        assert len(unsigned_permutations(point, even=even)) == length

def _assemble_perms(points, evens):
    out = []
    for p,e in zip(points, evens):
        out.extend([
                tuple(NamedFloat(x, get_str_repr(x)) for x in q) 
                    for q in unsigned_permutations(p,e)])
    return tuple(out)

def gen_dodecaplex_vertices():
    return _assemble_perms(DODECAPLEX_POINTS, D_EVENS)

def gen_tetraplex_vertices():
    return _assemble_perms(TETRAPLEX_POINTS, T_EVENS)

def get_seperation(origin, other):
    return np.linalg.norm(np.array(origin)-np.array(other))

def get_neighbors(origin, others):
    neighbor_indeces = tuple( i for i, other in enumerate(others) if isclose(get_seperation(origin, other), DISTANCE_600CELL))
    assert len(neighbor_indeces) == 12
    return neighbor_indeces

def yield_dodecahedrons_from_dodecaplex(dodecaplex_4d_verts, tetraplex_4d_verts):
    for ov in tetraplex_4d_verts:
        cell_points = set(v for v in dodecaplex_4d_verts if isclose(get_seperation(v, ov), RADIUS_120CELL))
        assert len(cell_points) == 20
        yield cell_points

def get_neighbor_map(tetraplex_4d_verts):
    return {i : get_neighbors(center, tetraplex_4d_verts) for i, center in enumerate(tetraplex_4d_verts)}

if __name__ == "__main__":
    test_initialization()