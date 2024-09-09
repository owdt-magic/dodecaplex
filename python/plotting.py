import pprint
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

def plot_3d_projection(vertices, EDGE=EDGE_120CELL, scale=3):
    vertices = np.array(vertices)
    fig = plt.figure(figsize=(8, 8))
    ax = fig.add_subplot(projection='3d')
    ax.scatter3D(vertices[:, 0], vertices[:, 1], vertices[:, 2], s=0.1)
    for ov,osv in zip(vertices, vertices):
        neighbors = np.array([sv-osv for v, sv in zip(vertices, vertices) if ((EDGE-0.01) < np.linalg.norm(np.array(ov)-np.array(v)) < (EDGE+0.01))]) 
        color_new = random.choice(['red', 'green', 'blue', 'yellow', 'purple', 'cyan'])
        if len(neighbors) < 3: 
            continue
        plt.quiver([osv[0] for x in range(len(neighbors))], 
                   [osv[1] for y in range(len(neighbors))], 
                   [osv[2] for z in range(len(neighbors))], neighbors[:,0], neighbors[:,1], neighbors[:,2],
                   color=[color_new for c in range(len(neighbors))], alpha=0.4)

    ax.set_xlim(-scale, scale)
    ax.set_ylim(-scale, scale)
    ax.set_zlim(-scale, scale)

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

def plot_isolated_cells(dodecaplex_4d_verts, tetraplex_4d_verts, transformation=None):

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
        if transformation is not None: np20verts = np20verts@transformation
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
        
def characterize_displacements(d4vs, t4vs, neighbor_map):    
    all20sets = list(yield_dodecahedrons_from_dodecaplex(d4vs, t4vs))
    isolated_sides = {i :
        { n: 
            {'points' : sub20set & all20sets[n],
            'displacement' : tuple(t-c for c,t in zip(center,t4vs[n]))}
            for n in neighbor_map[i]
        } for (i, sub20set), center in zip(enumerate(all20sets), t4vs)
    }
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

def plot_translated_neighbor_maps(t4vs):        
    neighbor_map = get_neighbor_map(t4vs)

    for o_idx in range(len(t4vs)):

        starting_origins = []
        neighbor_origins = []
        for n_idx in neighbor_map[o_idx]:
            starting_origins.append(t4vs[n_idx])
            neighbor_origins.append(np.array([t4vs[nn_idx] for nn_idx in neighbor_map[n_idx]]))
        starting_origins = np.array(starting_origins)
        
        for n_idx, (nPoints, oPoint) in enumerate(zip(neighbor_origins, starting_origins)):
            nMean = np.mean(nPoints, axis=0)
            assert are_close((nMean)*(2/PHI), oPoint)
            fPoints = (nPoints-nMean[np.newaxis, :])*(2/PHI)
            print(f"origin: {o_idx}, neighbor: {n_idx}")
            plot_3d_projection(fPoints, scale = 0.66)


if __name__ == "__main__":
    d4vs, t4vs = gen_dodecaplex_vertices(), gen_tetraplex_vertices()
    nmap = get_neighbor_map(t4vs)
    set20s = list(yield_dodecahedrons_from_dodecaplex(d4vs, t4vs))

    """     fig = plt.figure(figsize=(8, 8))
    ax = fig.add_subplot(projection='3d') """

    colorlist = ['red', 'green', 'blue']
    cell_idx = 0

    def add_dodec(points):
        for ov in points:            
            quiverArrows = np.array([v-ov for v in points if isclose(get_seperation(ov, v), EDGE_120CELL)])
            if quiverArrows.size > 0:
                plt.quiver([ov[0] for x in range(len(quiverArrows))], 
                        [ov[1] for y in range(len(quiverArrows))], 
                        [ov[2] for z in range(len(quiverArrows))], quiverArrows[:,0], quiverArrows[:,1], quiverArrows[:,2],
                        color=colorlist[(cell_idx)%len(colorlist)], alpha=1.0)        


    org_20_set = set20s[0]
    all_vert_lut = np.zeros((600), np.uint8)
    rotation_lut = {}
    colors_ = ['red', 'green', 'blue']
    for adj_idx in nmap[0]:
        print(f'Neighbor {adj_idx}')
        adj_20_set  = set20s[adj_idx]
        adj_cent    = t4vs[adj_idx]
        adj_rot     = mat_180_around(adj_cent[:3])
        adj_wall    = adj_20_set & org_20_set

        adj_wall_list = list(adj_wall)
        clockwise_wall = adj_wall_list.copy()
        mean_wall = np.mean(np.array(adj_wall_list), axis=0)
        """ 
        clockwise_wall.sort(key=lambda x: 
                            np.inner(np.array(x)-mean_wall,
                            np.array(adj_wall_list[0])-mean_wall)
                            )
        for start, end in zip(clockwise_wall, clockwise_wall[1:]+[clockwise_wall[0]]):
            plt.quiver(start[0]+adj_cent[0],start[1]+adj_cent[1],start[2]+adj_cent[2], end[0]-start[0],end[1]-start[1],end[2]-start[2], color=colors_[adj_idx%len(colors_)])
         """
        rot_axis            = adj_cent[:3]
        rot_mat             = mat_180_around(rot_axis)
        adj_20_arr          = np.array(list(adj_20_set))
        dst_20_arr          = np.array(list(org_20_set))
        org_20_arr          = np.copy(dst_20_arr)
        dst_20_arr[:,:3]    = np.matmul(rot_mat, dst_20_arr[:,:3].T).T

        idx_map = np.zeros((20), np.uint)
        for in_adj_idx, adj_point in enumerate(adj_20_arr):
            for dst_idx, dst_point in enumerate(dst_20_arr):
                if np.all(np.linalg.norm(np.cross(np.array(rot_axis), np.array(adj_point-dst_point)[:3])) < 0.01):
                    idx_map[in_adj_idx] = dst_idx
        
        print(adj_20_arr.shape)
        print(dst_20_arr.shape)

        adj_20_arr_old = np.copy(adj_20_arr)

        for t,f in enumerate(idx_map):
            adj_20_arr[f, :] = adj_20_arr_old[t, :]

        fourdee = np.linalg.lstsq(adj_20_arr, dst_20_arr)
        print(fourdee)
        #a @ x = b

        plot_3d_projection(np.concatenate((adj_20_arr@fourdee[0],adj_20_arr)))
        
        

    """ ax.set_xlim(-3, 3)
    ax.set_ylim(-3, 3)
    ax.set_zlim(-3, 3) """

    # plt.show()