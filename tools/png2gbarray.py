import os
import numpy as np
from PIL import Image

""" 
This code converts tiles with the following palette 
to a C array suitable for GBDK 2020.

ONLY ASEPRITE EXPORTED PNGs ARE SUPPORTED.

AsepriteValue      Palette color        GB Color Code
-----------------------------------------------------
      0             Transparent             0   (sprites)
      1             Light Green            1/0  (sprites/bkgs)
      2             Medium Green           2/1  (sprites/bkgs)
      3             Dark Green              2   (bkgs)
      4             Black                   3   (sprites/bkgs)

"""
"""
Read in PNG file and split the image into 8x8 tiles.

We need to generate 2 files:
    (1) Pixel information for all unique tiles that make up the background.
    (2) Map that contains the tile index from (1) to construct the map in the PNG
"""
fn_path = "../assets/Snake-spritesheet.png"
filename = os.path.splitext(os.path.split(fn_path)[-1])[0].lower()
filename = filename.replace("-","_")
filename = filename.replace(" ","_")

gb_code_sprite = {0:0, 1:1, 2:2, 4:3}  # GBSutdio sprite pallete
gb_code_bkg = {1:0, 2:1, 3:2, 4:3}  # GBSutdio bkg pallete

im = np.asarray(Image.open(fn_path))

# Look for the Aseprite index 3 to assign the background dictionary
if len(np.argwhere(im == 3)) > 0:
   gb_code = gb_code_bkg
else:
   gb_code = gb_code_sprite

ntiles = int(im.size/(8*8))  # Total number of 8x8 tiles = npixels/64

s = ""
c_array = f"const unsigned char {filename}_data[] = "
c_array += "{\n"

for i in range(ntiles):
  c_array += "  "
  # Grab the first 8x8 tile
  tile = im[:8,8*i:8*(i+1)]

  # Print the values in the tile
  for row in range(tile.shape[0]):
      msb = '0b'
      lsb = '0b'
      for col in range(tile.shape[1]):
          code = gb_code[tile[row,col]]
          msb += str((code & 0x2) >> 1)
          lsb += str((code & 0x1))

      msb = int(msb, 2)
      lsb = int(lsb, 2)

      if lsb < 16:
        s += hex(lsb).replace("0x", "0")
        c_array += hex(lsb).replace("0x", "0x0").upper() + ","
      else:  
        s += hex(lsb).replace("0x", "")
        c_array += hex(lsb).upper() + ","

      s += " "

      if msb < 16:
        s += hex(msb).replace("0x", "0")
        c_array += hex(msb).replace("0x", "0x0").upper() + ","
      else:  
        s += hex(msb).replace("0x", "")
        c_array += hex(msb).upper() + ","

      s += " "
  s += "\n"
  c_array = c_array + "\n"

c_array = c_array[:-1] + "\n}"
print(s)
print(c_array)