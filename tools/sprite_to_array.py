import numpy as np
from PIL import Image
import matplotlib.pyplot as plt

""" 
This code converts tiles with the following palette 
to a C array suitable for GBDK 2020.

  R,G,B        Palette color    GB Color Code
-----------------------------------------------
101,255,0       Transparent           0  (356)
224,248,207      Light Green          1  (679)
134,192,108      Dark Green           2  (434)
7,24,33            Black              3  (64)

  R,G,B        Palette color    GB Color Code
-----------------------------------------------
224,248,207      Light Green          0  (679)
134,192,108      Medium Green         1  (434)
48,104,50        Dark Green           2  (202)
7,24,33            Black              3  (64)


AsepriteValue      Palette color        GB Color Code
-----------------------------------------------------
      0             Transparent             0   (sprites)
      1             Light Green            1/0  (sprites/bkgs)
      2             Medium Green           2/1  (sprites/bkgs)
      3             Dark Green              2   (bkgs)
      4             Black                   3   (sprites/bkgs)

"""

gb_code_sprite = {356:0, 679:1, 434:2, 64:3}  # GBSutdio sprite pallete
gb_code_bkg = {679:0, 434:1, 202:2, 64:3}  # GBSutdio bkg pallete

gb_code = gb_code_bkg

im = np.asarray(Image.open("../assets/Background-export.png"))

if len(im.shape) > 2:
   im = im[:, :, :3]

pix_sum = im.sum(axis=2)

# Look for the value 356 (Transparent) in pix_sum to assign the sprite dictionary
if np.argwhere(pix_sum == 356).shape[0] > 0:
    gb_code = gb_code_sprite
else:
    gb_code = gb_code_bkg

for i in range(4):
  # Grab the first 8x8 tile
  tile = pix_sum[:8,8*i:8*(i+1)]

  # Print the values in the tile
  s = ""
  c_array = "const unsigned char sprite_data[] = {\n"
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

  print(s)
  print(f"{c_array[:-1]}")
  print("}")

