from sympy.utilities.iterables import multiset_permutations

PHI = (1+(5**0.5))/2
EDGE_120CELL = 3 - (5**0.5)
EDGE_600CELL = 1/PHI
RADIUS_120CELL = (6-(5**0.5))**0.5

def calculate_parity(permutation):
    inversions = 0
    n = len(permutation)
    for i in range(n):
        for j in range(i + 1, n):
            if permutation[i] > permutation[j]:
                inversions += 1
    return inversions % 2 == 0

def unsigned_permutations(a,b,c,d, even=False):
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

def test_initialization():
    assert len(unsigned_permutations(0,       0,              2,          2           )) == 24
    assert len(unsigned_permutations(PHI,     PHI,            PHI,        (PHI**(-2)) )) == 64
    assert len(unsigned_permutations(1,       1,              1,          (5**(0.5))  )) == 64
    assert len(unsigned_permutations((1/PHI), (1/PHI),        (1/PHI),    (PHI*PHI)   )) == 64
    assert len(unsigned_permutations(0,       (1/PHI),        PHI,        (5**(0.5)), even=True)) == 96
    assert len(unsigned_permutations(0,       (PHI**(-2)),    1,          (PHI**2),   even=True)) == 96
    assert len(unsigned_permutations((1/PHI), 1,              PHI,        2,          even=True)) == 192

    assert len(unsigned_permutations(0,      0,          0,      1   )) == 8
    assert len(unsigned_permutations(0.5,    0.5,        0.5,    0.5 )) == 16
    assert len(unsigned_permutations(0,      (0.5*PHI),  0.5,    (0.5/PHI), even=True)) == 96

def gen_dodecaplex_vertices():
    return tuple( \
        tuple(unsigned_permutations(0,       0,              2,          2           )) +\
        tuple(unsigned_permutations(PHI,     PHI,            PHI,        (PHI**(-2)) )) +\
        tuple(unsigned_permutations(1,       1,              1,          (5**(0.5))  )) +\
        tuple(unsigned_permutations((1/PHI), (1/PHI),        (1/PHI),    (PHI*PHI)   )) +\
        tuple(unsigned_permutations(0,       (1/PHI),        PHI,        (5**(0.5)), even=True)) +\
        tuple(unsigned_permutations(0,       (PHI**(-2)),    1,          (PHI**2),   even=True)) +\
        tuple(unsigned_permutations((1/PHI), 1,              PHI,        2,          even=True)))

def gen_tetraplex_vertices():
    return tuple( \
        tuple(unsigned_permutations(0,      0,          0,      1   )) +\
        tuple(unsigned_permutations(0.5,    0.5,        0.5,    0.5 )) +\
        tuple(unsigned_permutations(0,      (0.5*PHI),  0.5,    (0.5/PHI), even=True)))
