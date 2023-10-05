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

"""

gb_code = {356:0, 679:1, 434:2, 64:3}  # GBSutdio sprite pallete
gb_code_bkg = {679:0, 434:1, 202:2, 64:3}  # GBSutdio bkg pallete

gb_code = gb_code_bkg

im = np.asarray(Image.open("../assets/Background.png"))

###### IMPROVEMENT:
###### im = im[:,:,:3]  # Removes the alpha channel entirely
###### pix_sum = im.sum(axis=2)  # Add R+G+B values
###### Iterate over pix_sum tiles to convert into GB values

for i in range(4):
  # Grab the first 8x8 tile
  tile = im[:8,8*i:8*(i+1),:3]

  # Print the values in the tile
  s = ""
  c_array = "const unsigned char sprite_data[] = {\n"
  for r in range(tile.shape[0]):
      msb = '0b'
      lsb = '0b'
      for c in range(tile.shape[1]):
          code = gb_code[tile[r,c].sum()]
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

