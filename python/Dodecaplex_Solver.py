import random
import numpy as np
import matplotlib.pyplot as plt
import mpl_toolkits.mplot3d as a3
import matplotlib.colors as colors
from sympy.utilities.iterables import multiset_permutations

PHI = (1+(5**0.5))/2
EDGE = 3 - (5**0.5)

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

vertices = tuple( \
tuple(unsigned_permutations(0,       0,              2,          2           ))+\
tuple(unsigned_permutations(PHI,     PHI,            PHI,        (PHI**(-2)) )) +\
tuple(unsigned_permutations(1,       1,              1,          (5**(0.5))  )) +\
tuple(unsigned_permutations((1/PHI), (1/PHI),        (1/PHI),    (PHI*PHI)   )) +\
tuple(unsigned_permutations(0,       (1/PHI),        PHI,        (5**(0.5)), even=True)) +\
tuple(unsigned_permutations(0,       (PHI**(-2)),    1,          (PHI**2),   even=True)) +\
tuple(unsigned_permutations((1/PHI), 1,              PHI,        2,          even=True)))
# last two are problmeatic (3 edges)

def plot_2d_projection(vertices):
    colors =  [(len([None for vx in vertices if np.linalg.norm(np.array(v)-np.array(vx)) < .765])**4)/10 \
                    for v in vertices]

    plt.figure(figsize=(8, 8))
    plt.scatter(vertices[:, 0], vertices[:, 2], c=colors, s=colors)
    for v, color in zip(vertices, colors):
        neighbors = np.array([vx-v for vx in vertices if np.linalg.norm(np.array(v)-np.array(vx)) < .765])
        if len(neighbors) == 5 : continue
        plt.quiver([v[0] for x in range(len(neighbors))], 
                   [v[1] for y in range(len(neighbors))], neighbors[:,0], neighbors[:,1], 
                   color=['red' for c in range(len(neighbors))], angles='xy', scale_units='xy', scale=1,
                   headwidth=4, headlength=3, width=0.002)

    plt.title(f'2D Projection of vertices')
    plt.grid(True)
    plt.axis('equal')
    plt.show()

def plot_3d_projection(vertices):
    scaled_vertices = vertices*(vertices[:,-1]**2)[:, np.newaxis]
    fig = plt.figure(figsize=(8, 8))
    ax = fig.add_subplot(projection='3d')
    ax.scatter3D(scaled_vertices[:, 0], scaled_vertices[:, 1], scaled_vertices[:, 2], s=0.1)
    threes, fours = 0,0
    for v,vs in zip(vertices, scaled_vertices):
        neighbors = np.array([sv-vs for sv, vx in zip(scaled_vertices, vertices) if ((EDGE-0.01) < np.linalg.norm(np.array(v)-np.array(vx)) < (EDGE+0.01))]) 
        color_new = random.choice(['red', 'green', 'blue', 'yellow', 'purple', 'cyan'])
        plt.quiver([vs[0] for x in range(len(neighbors))], 
                   [vs[1] for y in range(len(neighbors))], 
                   [vs[2] for z in range(len(neighbors))], neighbors[:,0], neighbors[:,1], neighbors[:,2],
                   color=[color_new for c in range(len(neighbors))], alpha=0.4)

    print("THREE: ", threes, " FOUR: ", fours)
    plt.title(f'3D Projection of vertices')
    plt.show()

def plot_triangles():
    """
    For every point: find it's neighbors. (there will be 48 redundant square neighbor points)

    Create triangles from the neighbors. Skip 7 points - they should be accounted last.

    Determine whether 7 points were safe to ignore (already accounted by preceeding triangles)

    """

    neighborhoods = {}

    for four_d_point in vertices:    
        neighborhood = [v for v in vertices if 0.0 < np.linalg.norm(np.array(v)-np.array(four_d_point)) < EDGE+0.01]
        neighborhoods[four_d_point] = neighborhood

    triangles = []
    blacklist= []
    for origin, neighbors in neighborhoods.items():
        if origin in blacklist: continue

        for a,b in ((0, 1), (0, 2), (0, 3), (1, 2), (1, 3), (2, 3)):
            triangles.append([origin, neighbors[a], neighbors[b]])
        for neighbor in neighbors:
            blacklist.append(neighbor)
    triangles = np.array(triangles)


    fig = plt.figure()

    ax = a3.Axes3D(fig)
    b = 2
    ax.set(xlim=(-b,b), ylim=(-b,b), zlim=(-b,b))
    fig.add_axes(ax)
    for x in range(150):
        tri = a3.art3d.Poly3DCollection([triangles[x, :, :3]]/(triangles[x, :,-1]**2)[:, np.newaxis])
        tri.set_color(colors.rgb2hex(np.random.rand(3)))
        tri.set_edgecolor('k')
        ax.add_collection3d(tri)
    plt.show()

plot_3d_projection(np.array(vertices))