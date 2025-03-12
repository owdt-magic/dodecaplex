import glob
import svgelements
import xml.etree.ElementTree as ET

import numpy as np
import matplotlib.pyplot as plt

def plot_rhombuses(data):
  """
  Plots an Nx4 numpy array as a bunch of rhombuses in matplotlib.

  Args:
    data: An Nx4 numpy array, where each row represents a rhombus 
          with the four values being the x, y coordinates of the 
          top, right, bottom, and left vertices respectively.
  """
  fig, ax = plt.subplots()
  for row in data:
    x = row[[0, 2, 4, 6, 0]]  # Close the shape by repeating the first point
    y = row[[1, 3, 5, 7, 1]]  # Close the shape by repeating the first point
    ax.plot(x, y, 'b-')  # Connect points with blue lines

  ax.set_aspect('equal')  # Ensure rhombuses look like rhombuses
  plt.show()

for svg_path in glob.glob('/Users/owdt/SHADER/python/svgs/*.svg'):
    svg = svgelements.SVG.parse(svg_path)

    # Access elements and attributes
    print(svg_path)
    print(svg.width)
    print(svg.height)
    group_element = list(svg.elements()).pop(1)
    print(group_element.transform)
    group_string = group_element.string_xml()
    group_etree = ET.ElementTree(ET.fromstring(group_string))
    corners = []
    for path_element in group_etree.getroot().findall('path'):
        path_string = path_element.get('d')
        sub_strings = path_string.split()
        assert len(sub_strings) == 9
        c1 = list(float(x) for x in sub_strings[1].split(','))
        c2 = list(float(x) for x in sub_strings[3].split(','))
        c3 = list(float(x) for x in sub_strings[5].split(','))
        c4 = list(float(x) for x in sub_strings[7].split(','))

        if sub_strings[2] == 'l':
            c2[0] += c1[0]
            c2[1] += c1[1]
        if sub_strings[4] == 'l':
            c3[0] += c2[0]
            c3[1] += c2[1]
        if sub_strings[6] == 'l':
            c4[0] += c3[0]
            c4[1] += c3[1]            
        corners.append([*c1,*c2,*c3,*c4])

    existing_xs = [0]
    existing_ys = [0]
    for r, rhombus in enumerate(corners):
        for p in range(4):
            closest_x = min([rhombus[2*p]-ox for ox in existing_xs])
            closest_y= min([rhombus[2*p+1]-oy for oy in existing_ys])
            existing_xs.append(rhombus[2*p])
            existing_ys.append(rhombus[2*p+1])
            if closest_y > 0:
                print(closest_y)
            if closest_x > 0:
                print(closest_x)
