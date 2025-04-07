import cv2
import numpy as np

# --- Configurable ---
canvas_size = (600, 600, 3)
epsilon = 10  # pixel threshold for merging vertices
radius = 5

# --- State ---
vertices = []          # Unique vertex list
indices = []           # List of triangle indices (VAO-style)
current_triangle = []  # Building triangle (indices only)

window_name = "Triangle VAO Tool"

def find_or_add_vertex(x, y):
    """Return the index of a nearby vertex, or add new one."""
    for i, (vx, vy) in enumerate(vertices):
        if (vx - x)**2 + (vy - y)**2 < epsilon**2:
            return i  # Found existing vertex
    vertices.append([x, y])
    return len(vertices) - 1  # New vertex

def draw_canvas():
    img = np.ones(canvas_size, dtype=np.uint8) * 255

    # Draw all triangles
    for tri in indices:
        pts = np.array([vertices[i] for i in tri], np.int32).reshape((-1, 1, 2))
        cv2.fillConvexPoly(img, pts, color=(200, 100, 255))
        for i in tri:
            x, y = vertices[i]
            cv2.circle(img, (x, y), radius, (0, 0, 0), -1)

    # Draw in-progress triangle
    for i in current_triangle:
        x, y = vertices[i]
        cv2.circle(img, (x, y), radius, (255, 0, 0), -1)

    if len(current_triangle) == 3:
        pts = np.array([vertices[i] for i in current_triangle], np.int32).reshape((-1, 1, 2))
        cv2.polylines(img, [pts], isClosed=True, color=(0, 0, 255), thickness=2)

    return img

def mouse_callback(event, x, y, flags, param):
    global current_triangle, indices
    if event == cv2.EVENT_LBUTTONDOWN:
        idx = find_or_add_vertex(x, y)
        current_triangle.append(idx)
        if len(current_triangle) == 3:
            indices.append(current_triangle.copy())
            current_triangle = []

cv2.namedWindow(window_name)
cv2.setMouseCallback(window_name, mouse_callback)

while True:
    frame = draw_canvas()
    cv2.imshow(window_name, frame)

    key = cv2.waitKey(1) & 0xFF
    if key == 27:  # ESC
        break
    elif key == ord('c'):  # Clear
        vertices.clear()
        indices.clear()
        current_triangle.clear()
    elif key == ord('s'):  # Save VAO data
        print("\n--- Export VAO Data ---")
        print("Vertices:")
        for v in vertices:
            print(f"{float( '%.3g'%(v[0]/canvas_size[0]))}f, {float( '%.3g'%(v[1]/canvas_size[1]))}f,")
        print("\nTriangle Indices:")
        for tri in indices:
            print(', '.join(str(t) for t in tri)+',')

cv2.destroyAllWindows()
