import random
import numpy as np
import matplotlib.pyplot as plt
import mpl_toolkits.mplot3d as a3
import matplotlib.colors as colors
from fourdsolids import *

def plot_2d_projection(vertices):
    vertices = np.array(vertices)
    plt.figure(figsize=(8, 8))
    plt.scatter(vertices[:, 0], vertices[:, 2])
    for v in vertices:
        neighbors = np.array([vx-v for vx in vertices if np.linalg.norm(np.array(v)-np.array(vx)) < .765])
        plt.quiver([v[0] for x in range(len(neighbors))], 
                   [v[1] for y in range(len(neighbors))], neighbors[:,0], neighbors[:,1], 
                   color=['red' for c in range(len(neighbors))], angles='xy', scale_units='xy', scale=1,
                   headwidth=4, headlength=3, width=0.002)

    plt.title(f'2D Projection of vertices')
    plt.show()

def plot_3d_projection(vertices, EDGE=EDGE_120CELL):
    vertices = np.array(vertices)
    #scaled_vertices = vertices/abs(vertices[:,-1])[:, np.newaxis]
    scaled_vertices = vertices/(PHI**(vertices[:,-1]/10))[:, np.newaxis]
    fig = plt.figure(figsize=(8, 8))
    ax = fig.add_subplot(projection='3d')
    ax.scatter3D(scaled_vertices[:, 0], scaled_vertices[:, 1], scaled_vertices[:, 2], s=0.1)
    for ov,osv in zip(vertices, scaled_vertices):
        neighbors = np.array([sv-osv for v, sv in zip(vertices, scaled_vertices) if ((EDGE-0.01) < np.linalg.norm(np.array(ov)-np.array(v)) < (EDGE+0.01))]) 
        color_new = random.choice(['red', 'green', 'blue', 'yellow', 'purple', 'cyan'])
        plt.quiver([osv[0] for x in range(len(neighbors))], 
                   [osv[1] for y in range(len(neighbors))], 
                   [osv[2] for z in range(len(neighbors))], neighbors[:,0], neighbors[:,1], neighbors[:,2],
                   color=[color_new for c in range(len(neighbors))], alpha=0.4)

    plt.title(f'3D Projection of vertices')
    plt.show()

def plot_triangles(vertices, EDGE=EDGE_120CELL):

    neighborhoods = {}

    for ov in vertices:    
        neighborhood = [v for v in vertices if EDGE-0.01 < np.linalg.norm(np.array(v)-np.array(ov)) < EDGE+0.01]
        neighborhoods[ov] = neighborhood

    triangles = []
    blacklist= []
    items = list(neighborhoods.items())
    random.shuffle(items)

    for origin, neighbors in items:
        #if origin in blacklist: continue

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
    for x in range(len(triangles)):
        tri = a3.art3d.Poly3DCollection([triangles[x, :, :3]]/(triangles[x, :,-1]**2)[:, np.newaxis])
        tri.set_color(colors.rgb2hex(np.random.rand(3)))
        tri.set_edgecolor('k')
        ax.add_collection3d(tri)
    plt.show()

def plot_isolated_cells(dodecaplex_4d_verts, tetraplex_4d_verts):

    fig = plt.figure(figsize=(8, 8))
    ax = fig.add_subplot(projection='3d')
    count = 0
    all_d_verts = yield_dodecahedrons_from_dodecaplex(dodecaplex_4d_verts, tetraplex_4d_verts)

    color_dict = {}
    origin = np.array(random.choice(tetraplex_4d_verts))

    the_colors = ['black', 'orange', 'purple', 'magenta', 'green', 'red', 'black', 'yellow', 'grey', 'magenta', 'orange']

    np20sOff, os, svs, numpyDisps = [],[],[],[]

    for set20verts, center in zip(all_d_verts, tetraplex_4d_verts):
        np20verts = np.array(list(set20verts))
        np20sOff.append(np20verts-origin[np.newaxis,:])
        disp =  np.linalg.norm(np.array(center)-origin)
        numpyDisps.append(disp)

    minDisp = min(list(numpyDisps))
    avrgDisp = sum(set(list(numpyDisps)))/len(set(list(numpyDisps)))

    for np20off, disp in zip(np20sOff, numpyDisps):
        
        if disp > avrgDisp: continue
        
        color_new = color_dict.get(disp, the_colors[len(color_dict.keys())%len(the_colors)])
        color_dict[disp] = color_new
        
        for ov in np20off:
            quiverArrows = np.array([v-ov for v in np20off if isclose(get_seperation(ov, v), EDGE_120CELL)])
            plt.quiver([ov[0] for x in range(len(quiverArrows))], 
                    [ov[1] for y in range(len(quiverArrows))], 
                    [ov[2] for z in range(len(quiverArrows))], quiverArrows[:,0], quiverArrows[:,1], quiverArrows[:,2],
                    color=[color_new for c in range(len(quiverArrows))], alpha=(1./(disp*2)**4) if disp!=minDisp else 1)
        count += 1

    ax.set_xlim(-3, 3)
    ax.set_ylim(-3, 3)
    ax.set_zlim(-3, 3)

    plt.title(f'3D Projection of vertices with {count} primitives')
    plt.show()


if __name__ == "__main__":
    d4vs, t4vs = gen_dodecaplex_vertices(), gen_tetraplex_vertices()

    plot_isolated_cells(d4vs, t4vs)