import matplotlib.pyplot as plt
import numpy as np
from math import pi, sin, cos
PHI = (1+(5**0.5))/2

H = 1
W = (H*PHI*cos(pi/10))/(PHI+0.5)

origin, base, corner_l, corner_r = (
    (0,0), (0,H), (W, H), (-W, H)
)


LW = cos(pi/5)
SW = sin(pi/5)
LT = cos(pi/10)
ST = sin(pi/10)

R1 = LT / (LT + SW)
R2 = LT / (2*LT + SW)
R3 = 2*ST / (2*LW + 2)
R4 = 2*LT / (2*LT + SW)
R5 = 2*LW / (2*LW + 2)

T1 = LW / (LW + 1)
T2 = 1 / (1 + 2*LW + ST)
T3 = (1 + LW) / (1 + 2*LW + ST)
T4 = SW / (2*SW + LT)
T5 = (2*LW + 1) / (2*LW + 2)
T6 = (2*LW + 1) / (2*LW + 1 + ST)
T7 = LT / (LT + 2*SW)

ang = 2*pi/5
rot = np.array([[cos(ang), -sin(ang)], [sin(ang), cos(ang)]])

starting_coords = {
    'pattern_1.svg' : ([
        (0,0), 
        (W*R1, H*R1), (-W*R1, H*R1), 
        (W*R3, H*R2), (-W*R3, H*R2),
        (0, H*R4), 
        (W*R5, H), (-W*R5, H), (W, H), (-W, H),
        np.matmul(np.array((-W*R3, H*R2)), rot)
    ], [(0,10,3), (10,3,1), 
        (0,4,3), (4,3,5),(2,4,5), (1,3,5), 
        (2,5,7), (1,5,6), (5,6,7),
        (7,9,2), (6,8,1)
        ]),
    'pattern_2.svg' : ([
        (0,0), (W*T1, H*T1), (-W*T1, H*T1), (0, H*T2),
        (W*T4, H*T3), (-W*T4, H*T3), 
        (W*T5, H*T5), (-W*T5, H*T5),
        (0,H*T6),
        np.matmul(np.array((0, H*T2)), rot),
        (W*T7, H), (-W*T7, H),
        (W, H), (-W, H)
    ], [(0,3,9),(3,9,1),         
        (4,5,3),
        (5,3,2), (4,3,1),
        (5,2,7), (4,1,6), (5,8,4),
        (8,4,10), (4,10,6),
        (11,5,8), (11,5,7), (11,8,10),
        (10,12,6), (11,13,7)
        ]),
    'pattern_5.svg' : ([
        (0,0), (W, H), (-W, H), 
        (0, (2*H)/(PHI+1)),
        np.matmul(np.array((0, (2*H)/(PHI+1))), rot)
    ], [(1,3,2), (0,3,4), (3,4,1)])
}

coordinates, triangles = starting_coords['pattern_2.svg']


# Plotting
plt.figure(figsize=(8, 8))
for rot_num in range(5):
    for triangle in triangles:
        triangle_coords = np.array([coordinates[i] for i in triangle]+[coordinates[triangle[0]]])
        for _ in range(rot_num):
            triangle_coords = np.matmul(triangle_coords, rot)
        x, y = zip(*triangle_coords)
        plt.plot(x, y, marker='o')

plt.axhline(0, color='gray', linewidth=0.5, linestyle='--')
plt.axvline(0, color='gray', linewidth=0.5, linestyle='--')
plt.gca().set_aspect('equal', adjustable='box')
plt.legend()
plt.title("Triangles Visualization")
plt.xlabel("X-axis")
plt.ylabel("Y-axis")
plt.grid(True)
plt.show()
