import numpy as np
from sympy.utilities.iterables import multiset_permutations
from itertools import combinations

PHI = 1.61803

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
        sign_permutation = map( tuple, multiset_permutations(sign_permutation))
        sign_permutation = filter( calculate_parity, sign_permutation) if even else sign_permutation
        all_permutations.extend(sign_permutation)
    return {*all_permutations}

# Function to check if four points are co-planar in 4D
def are_coplanar_4d(p1, p2, p3, p4):
    matrix = np.array([
        [p2[i] - p1[i] for i in range(4)],
        [p3[i] - p1[i] for i in range(4)],
        [p4[i] - p1[i] for i in range(4)]
    ])
    return np.isclose(np.linalg.det(matrix.T @ matrix), 0)

vertices =\
list(unsigned_permutations(0,       0,              2,          2           )) +\
list(unsigned_permutations(PHI,     PHI,            PHI,        (PHI**(-2)) )) +\
list(unsigned_permutations(1,       1,              1,          (5**(0.5))  )) +\
list(unsigned_permutations((1/PHI), (1/PHI),        (1/PHI),    (PHI*PHI)   )) +\
list(unsigned_permutations(0,       (1/PHI),        PHI,        (5**(0.5)), even=True)) +\
list(unsigned_permutations(0,       (PHI**(-2)),    1,          (PHI**2),   even=True)) +\
list(unsigned_permutations((1/PHI), 1,              PHI,        2,          even=True))





# Find co-planar groups in 4D
coplanar_groups = []
import tqdm
""" while vertices:
    p1 = vertices.pop() """
for p1 in vertices:
    nearby_vertices = [v for v in vertices if np.linalg.norm(np.array(v)-np.array(p1)) < .765]
    """     coplanar_group = [p1]
    for p2, p3, p4 in tqdm.tqdm(list(combinations(nearby_vertices, 3))):
        if p1 != p2 and p1 != p3 and p1 != p4 and are_coplanar_4d(p1, p2, p3, p4):
            coplanar_group.append(p2)
            coplanar_group.append(p3)
            coplanar_group.append(p4) """
    # Remove duplicates and check the size of the group
    coplanar_group = list(set(nearby_vertices))
    if len(coplanar_group) > 2:
        coplanar_groups.append(coplanar_group)

# Remove duplicate groups
unique_coplanar_groups = []
for group in coplanar_groups:
    group_set = frozenset(group)
    if group_set not in unique_coplanar_groups:
        unique_coplanar_groups.append(group_set)

# Convert back to list of lists if needed
unique_coplanar_groups = [list(group) for group in unique_coplanar_groups]

print(f"Found {len(unique_coplanar_groups)} co-planar groups")
lengths = [len(x) for x in unique_coplanar_groups]
dicti = {}
for length in lengths:
    dicti[length] = dicti.get(length, 0)
    dicti[length] += 1
print(f"Lengths of each group : {dicti}")

