import numpy as np
# Vertices of a regular 4-simplex
points_4d = np.array([
    [1, 1, 1, -1/np.sqrt(5)],
    [1, -1, -1, -1/np.sqrt(5)],
    [-1, 1, -1, -1/np.sqrt(5)],
    [-1, -1, 1, -1/np.sqrt(5)],
    [0, 0, 0, 4/np.sqrt(5)]
])

# Calculate the centroid
centroid_4d = np.mean(points_4d, axis=0)

# Calculate distances from the centroid to each point
distances_4d = np.linalg.norm(points_4d - centroid_4d, axis=1)

print("Centroid (4D):", centroid_4d)
print("Distances from centroid (4D):", distances_4d)
