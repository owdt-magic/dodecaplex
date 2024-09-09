import numpy as np
from math import isclose
from sympy.utilities.iterables import multiset_permutations

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

def mat_180_around(invec3):
    invec3 = np.array(invec3)
    x, y, z = invec3/np.linalg.norm(invec3)
    return np.array([
        [2*x*x-1,   2*y*x,      2*z*x   ],
        [2*x*y,     2*y*y-1,    2*z*y   ],
        [2*x*z,     2*y*z,      2*z*z-1 ]
    ])