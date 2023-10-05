import os
import numpy as np
from PIL import Image

"""
Read in PNG file and split the image into 8x8 tiles.

We need to generate 2 files:
    (1) Pixel information for all unique tiles that make up the background.
    (2) Map that contains the tile index from (1) to construct the map in the PNG
"""

