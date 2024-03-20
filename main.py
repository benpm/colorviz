import meshplot as mp
import numpy as np

import xml.etree.ElementTree as ET

def load_color_profile(file_path):
    tree = ET.parse(file_path)
    root = tree.getroot()
    vertices = [float(x) for x in root.find("Tags").find("gamutBoundaryDescType").find("Vertices").find("PCSValues").text.split()]
    vertices = np.reshape(vertices, (-1, 3))
    print(vertices)
    triangles = np.array([[int(x) for x in t.text.split()] for t in root.find("Tags").find("gamutBoundaryDescType").find("Triangles")])
    print(triangles)
    return vertices, triangles

def main():
    vertices, triangles = load_color_profile("profiles/sRGB_D65_MAT.xml")
    mp.plot(vertices, triangles, return_plot=True)

if __name__ == "__main__":
    main()