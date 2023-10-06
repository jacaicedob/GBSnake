import os
import numpy as np
from PIL import Image
from numpy.lib.stride_tricks import as_strided

""" 
This code converts tiles with the following palettes 
to a C array suitable for GBDK 2020.

https://gbdev.io/pandocs/Tile_Data.html

Sprites: RGB
  R,G,B        Palette color    GB Color Code
-----------------------------------------------
101,255,0       Transparent           0  (356)
224,248,207      Light Green          1  (679)
134,192,108      Dark Green           2  (434)
7,24,33            Black              3  (64)

Backgrounds: RGB
  R,G,B        Palette color    GB Color Code
-----------------------------------------------
224,248,207      Light Green          0  (679)
134,192,108      Medium Green         1  (434)
48,104,50        Dark Green           2  (202)
7,24,33            Black              3  (64)

Indexed PNGs (Sprites & Backgrounds)
AsepriteValue      Palette color        GB Color Code
-----------------------------------------------------
      0             Transparent             0   (sprites)
      1             Light Green            1/0  (sprites/bkgs)
      2             Medium Green           2/1  (sprites/bkgs)
      3             Dark Green              2   (bkgs)
      4             Black                   3   (sprites/bkgs)

"""
def convert_tile_to_vram_data(tile, gb_code, debug=False):
    s = ""
    c_array = ""
    for row in range(tile.shape[0]):
        msb = '0b'
        lsb = '0b'
        for col in range(tile.shape[1]):
            try:
              code = gb_code[tile[row,col]]
            except:
                # Find closes key to value
                value = tile[row, col]
                keys = np.array(list(gb_code.keys()))
                key_ind = np.argmax(1/np.abs(keys - value))
                code = gb_code[keys[key_ind]]

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
    if debug:
       print(s)

    return c_array 


def get_sprite_array(im, array_name, gb_code, debug=False):
    ntiles = int(im.size/(8*8))  # Total number of 8x8 tiles = npixels/64
    stride = int(im.shape[1]/8)

    c_array = f"const unsigned char {array_name}_data[] = "
    c_array += "{\n"
    for i in range(ntiles):
        c_array += "  "
        # Grab the first 8x8 tile
        irow = int(i / stride)
        icol = i % stride

        tile = im[8*irow:8*(irow+1),8*icol:8*(icol+1)]
        c_array += convert_tile_to_vram_data(tile, gb_code, debug)

        c_array += "\n"

    c_array = c_array[:-2] + "\n};"

    return c_array

def get_background_data_and_map(im, name, gb_code, debug=False):
    """
    Split the image into 8x8 tiles.

    We need to generate 2 files:
        (1) Pixel information for all unique tiles that make up the background.
        (2) Map that contains the tile index from (1) to construct the map in the PNG
    """
    ntiles = int(im.size/(8*8))  # Total number of 8x8 tiles = npixels/64
    stride = int(im.shape[1]/8)

    map_inds = np.arange(ntiles)









fn_path = "../assets/Snake-spritesheet.png"
fn_path = "../assets/Background.png"

filename = os.path.splitext(os.path.split(fn_path)[-1])[0].lower()
filename = filename.replace("-","_")
filename = filename.replace(" ","_")

gb_code_sprite_rgb = {356:0, 679:1, 434:2, 64:3}  # GBSutdio sprite pallete
gb_code_bkg_rgb = {679:0, 434:1, 202:2, 64:3}  # GBSutdio bkg pallete
gb_code_sprite_indexed = {0:0, 1:1, 2:2, 4:3}  # GBSutdio sprite pallete
gb_code_bkg_indexed = {1:0, 2:1, 3:2, 4:3}  # GBSutdio bkg pallete

im = np.asarray(Image.open(fn_path))

if len(im.shape) > 2:
    # RGB image with alpha channel
    # Drop alpha channel and reduce to 2D by summing the RGB channels
    im = im[:,:,:3]
    im = im.sum(axis=2)

    # Look for the RGB sum of 356 to assign the sprite dictionary (transparency color)
    if len(np.argwhere(im == 356)) > 0:
      image_type = "sprite"
      gb_code = gb_code_sprite_rgb
    else:
      image_type = "background"
      gb_code = gb_code_bkg_rgb

else:
    # Aseprite Indexed PNG
    # Look for the Aseprite index 3 to assign the background dictionary
    if len(np.argwhere(im == 3)) > 0:
      image_type = "background"
      gb_code = gb_code_bkg_indexed
    else:
      image_type = "sprite"
      gb_code = gb_code_sprite_indexed

if image_type == "sprite":
    array_string = get_sprite_array(im, filename, gb_code, debug=True)
    print(array_string)

else:
    pass




# ntiles = int(im.size/(8*8))  # Total number of 8x8 tiles = npixels/64
# rowtiles = int(im.shape[0]/8)
# coltiles = int(im.shape[1]/8)

# s = ""
# c_array = f"const unsigned char {filename}_data[] = "
# c_array += "{\n"

# for i in range(ntiles):
  # c_array += "  "
  # # Grab the first 8x8 tile
  # irow = int(i / coltiles)
  # icol = i % coltiles

  # tile = im[8*irow:8*(irow+1),8*icol:8*(icol+1)]

  # # Print the values in the tile
  # for row in range(tile.shape[0]):
      # msb = '0b'
      # lsb = '0b'
      # for col in range(tile.shape[1]):
          # try:
            # code = gb_code[tile[row,col]]
          # except:
              # # Find closes key to value
              # value = tile[row, col]
              # keys = np.array(list(gb_code.keys()))
              # key_ind = np.argmax(1/np.abs(keys - value))
              # code = gb_code[keys[key_ind]]

          # msb += str((code & 0x2) >> 1)
          # lsb += str((code & 0x1))

      # msb = int(msb, 2)
      # lsb = int(lsb, 2)

      # if lsb < 16:
        # s += hex(lsb).replace("0x", "0")
        # c_array += hex(lsb).replace("0x", "0x0").upper() + ","
      # else:  
        # s += hex(lsb).replace("0x", "")
        # c_array += hex(lsb).upper() + ","

      # s += " "

      # if msb < 16:
        # s += hex(msb).replace("0x", "0")
        # c_array += hex(msb).replace("0x", "0x0").upper() + ","
      # else:  
        # s += hex(msb).replace("0x", "")
        # c_array += hex(msb).upper() + ","

      # s += " "
  # s += "\n"
  # c_array = c_array + "\n"

# c_array = c_array[:-1] + "\n}"
# print(s)
# print(c_array)