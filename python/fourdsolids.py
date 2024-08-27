import numpy as np
from math import isclose
from sympy.utilities.iterables import multiset_permutations

PHI = (1+(5**0.5))/2
EDGE_120CELL = 3 - (5**0.5)
EDGE_600CELL = 1/PHI
RADIUS_120CELL = (6-(5**0.5))**0.5
DISTANCE_600CELL = PHI - 1

LATENT_WARP = (5**0.5)-1
RADIUS_600CELL_GEN2 = 0.5877852522924731

class NamedFloat(float):
    def __new__(cls, value, name=None):
        obj = super().__new__(cls, value)
        obj.name = name  # Add a custom attribute
        return obj

    def __repr__(self):
        return self.name

def are_close(t1, t2, tol=1e-4):
    return all(isclose(x, y, abs_tol=tol) for x, y in zip(t1, t2))

def calculate_parity(permutation):
    inversions = 0
    n = len(permutation)
    for i in range(n):
        for j in range(i + 1, n):
            if permutation[i] > permutation[j]:
                inversions += 1
    return inversions % 2 == 0

def unsigned_permutations(vert, even=False):
    a,b,c,d = vert
    all_permutations = []
    for x in range(2**4):
        binary = bin(x)
        binary = '0000'+binary.replace('b', '')
        
        sign_permutation = [a*(-1)**(int(binary[-1])+1), b*(-1)**(int(binary[-2])+1), 
                            c*(-1)**(int(binary[-3])+1), d*(-1)**(int(binary[-4])+1)]
        these_permutations = []
        for index_perm in multiset_permutations(list(range(4))):
            if calculate_parity(index_perm) or not even:
                these_permutations.append(tuple(sign_permutation[i] for i in index_perm))
        all_permutations.extend(set(these_permutations))
    return {*all_permutations}

DODECAPLEX_POINTS = [
    (0,       0,              2,          2           ),
    (PHI,     PHI,            PHI,        (PHI**(-2)) ),
    (1,       1,              1,          (5**(0.5))  ),
    ((1/PHI), (1/PHI),        (1/PHI),    (PHI*PHI)   ),
    (0,       (1/PHI),        PHI,        (5**(0.5))  ),
    (0,       (PHI**(-2)),    1,          (PHI**2)    ),
    ((1/PHI), 1,              PHI,        2           ),
]
TETRAPLEX_POINTS = [
    (0,      0,          0,      1   ),
    (0.5,    0.5,        0.5,    0.5 ),
    (0,      (0.5*PHI),  0.5,    (0.5/PHI))
]
D_EVENS = [0,0,0,0,1,1,1]
T_EVENS = [0,0,1]

def test_initialization():
    for point, length, even in zip(DODECAPLEX_POINTS, [24, 64, 64, 64, 96, 96, 192], D_EVENS):
        assert len(unsigned_permutations(point, even=even)) == length
    for point, length, even in zip(TETRAPLEX_POINTS, [8, 16, 96], T_EVENS):
        assert len(unsigned_permutations(point, even=even)) == length
        

def get_str_repr(v):
    o = ' ' if v >= 0 else '-'
    v = abs(v)
    if   v == 1.618033988749895:    return f'   {o}Φ'
    elif v == 0.38196601125010515:  return f'{o}1/Φ²'
    elif v == 2.23606797749979:     return f'  {o}√5'
    elif v == 0.6180339887498948:   return f' {o}1/Φ'
    elif v == 2.618033988749895:    return f'  {o}Φ²'
    elif v == 0.3090169943749474:   return f'{o}1/2Φ'
    elif v == 0.8090169943749475:   return f' {o}Φ/2'
    elif v == 1:                    return f'   {o}1'
    elif v == 0:                    return f'   {o}0'
    elif v == 0.5:                  return f' {o}1/2'
    elif v == 2.0:                  return f'   {o}2'
    else: return o+str(v)

def gen_dodecaplex_vertices():
    out = []
    for p,e in zip(DODECAPLEX_POINTS, D_EVENS):
        out.extend([
                tuple(NamedFloat(x, get_str_repr(x)) for x in q) 
                    for q in unsigned_permutations(p,e)])
    return tuple(out)

def gen_tetraplex_vertices():
    out = []
    for p,e in zip(TETRAPLEX_POINTS, T_EVENS):
        out.extend([
                tuple(NamedFloat(x, get_str_repr(x)) for x in q) 
                    for q in unsigned_permutations(p,e)])
    return tuple(out)


def get_seperation(origin, other):
    return np.linalg.norm(np.array(origin)-np.array(other))

def get_neighbors(origin, others):
    neighbor_indeces = set( i for i, other in enumerate(others) if isclose(get_seperation(origin, other), DISTANCE_600CELL))
    assert len(neighbor_indeces) == 12
    return neighbor_indeces

def yield_dodecahedrons_from_dodecaplex(dodecaplex_4d_verts, tetraplex_4d_verts):
    for ov in tetraplex_4d_verts:
        cell_points = set(v for v in dodecaplex_4d_verts if isclose(get_seperation(v, ov), RADIUS_120CELL))
        assert len(cell_points) == 20
        yield cell_points

if __name__ == "__main__":
    test_initialization()
            
    import pprint

    d4vs, t4vs = gen_dodecaplex_vertices(), gen_tetraplex_vertices()
    isolated_cells = {i : sub20set for i, sub20set in enumerate(yield_dodecahedrons_from_dodecaplex(d4vs, t4vs))}    
    neighbor_map = {i : get_neighbors(center, t4vs) for i, center in enumerate(t4vs)}
    isolated_sides = {i :
        { n: 
            {'points' : sub20set & isolated_cells[n],
             'displacement' : tuple(t-c for c,t in zip(center,t4vs[n]))}
            for n in neighbor_map[i]
        } for (i, sub20set), center in zip(isolated_cells.items(), t4vs)
    }

    pprint.pprint(isolated_cells[0])
    pprint.pprint(isolated_cells[66])
    
    def characterize_displacements():
        unique_displacements = []
        all_displacements = []

        for _, cell_data in isolated_sides.items():
            for _, side_data in cell_data.items():
                if not any( [are_close(other_disp, side_data['displacement']) for other_disp in unique_displacements] ):
                    unique_displacements.append(side_data['displacement'])
                all_displacements.append(side_data['displacement'])

                        
        # there are 120 unique relative displacements....
        # while there are 1440 total displacements

        print(len(unique_displacements))
        print(len(all_displacements))

    o_idx = 0
    starting_origins = []
    neighbor_origins = []
    for n_idx in neighbor_map[o_idx]:
        starting_origins.append(t4vs[n_idx])
        neighbor_origins.append(np.array([t4vs[nn_idx] for nn_idx in neighbor_map[n_idx]]))
    starting_origins = np.array(starting_origins)

    for nPoints, oPoint in zip(neighbor_origins, starting_origins):
        A_mean = np.mean(nPoints, axis=0)

        print('------')
        print([np.linalg.norm(x) for x in nPoints-A_mean[np.newaxis, :]])
        print(np.mean(nPoints-A_mean[np.newaxis, :], axis=0))

    """ pprint.pprint(neighbor_origins)
    pprint.pprint(starting_origins) """


    #to have a neighbor become centered...
    # there are 12 unique reflections....
    # 13 configurations are blended based upon proximity to the centers....
    # (well technically you blend all 12 evenly and you have the default!)
    # the reflections are trivial to make... the corespondeces are tricky...
    # reflect over the faces, effectively making 12 'spun' dodecaplexes.

    # recursively going through the neighbors....
    # for the first step... choose from the 12 initial sides...
        # you automatically have the first translation pair.
        # looping throug the sides, you keep track of accounted cells using the
        # displacement sums...
        # 

        #choose a side, get the displacement to it's center,
        # apply the displacement to the vertices of the tetraplex.
            #

    # APPLY REFLECTION TO THE ORIGINAL TETRAPLEX.
    # using the original tetraplex, you displace all the cells, and they will correspond with
    # the reflected points (unless something has gone wrong??)

    # saving the reflection with the corresponding cell can serve as enough information
    # to inferr the interpolation points...
            
            #every matching displacement will demonstrate another solved correspondence..
