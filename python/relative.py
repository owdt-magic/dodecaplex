import glob
import svgelements
import xml.etree.ElementTree as ET

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
    for path_element in group_etree.getroot().findall('path'):
        pass#print(path_element.get('d'))
