import os
import numpy as np
from PIL import Image
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
from matplotlib.collections import PatchCollection

collisions = np.zeros((20)*(18+2))
stride = 20  # Line width in tile index

def clear_collision_blocks():
    global collisions, ax, stride
    size = 8
    
    # Remove all collections from ax
    for c in ax.collections:
       c.remove()

    rectangles = []
    
    for ind in np.argwhere(collisions == 0).flatten():
        # Convert ind to x,y in tile space
        y = int(ind/stride) 
        x = ind % stride

        # Convert x,y into real space
        x *= 8
        y *= 8

        # Draw a red rectangle with alpha 0.25
        rectangles.append(Rectangle((x,y), size, size))

    pc = PatchCollection(rectangles, alpha=0)
    ax.add_collection(pc)

def mark_collision_blocks():
    global collisions, ax, stride, img
    size = 8
    
    # Remove all collections from ax
    for c in ax.collections:
       c.remove()

    rectangles = []
    
    for ind in np.argwhere(collisions != 0).flatten():
        # Convert ind to x,y in tile space
        y = int(ind/stride) 
        x = ind % stride

        # Convert x,y into real space
        x *= 8
        y *= 8

        # Draw a red rectangle with alpha 0.25
        rectangles.append(Rectangle((x,y), size, size))

    pc = PatchCollection(rectangles, facecolor="#ff0000", alpha=0.25, edgecolor="#ff0000")
    ax.add_collection(pc)

def onclick(event):
    global collisions, stride, fig

    # Get click location and convert to tile index in x,y
    x,y = event.xdata, event.ydata
    # print(x,y)
    x = int(x/8)  # [0, 20]
    y = int(y/8)  # [0, 18]
    # print(x,y)

    # Convert tile index to flat array index
    ind = stride*y + x 
    # print(ind)

    if collisions[ind] == 0:
      # Flag location as a collision block
      collisions[ind] = 1
      mark_collision_blocks()

    else:
      collisions[ind] = 0
      clear_collision_blocks()
      mark_collision_blocks()

    fig.canvas.draw()


filename = "Background.png"
root_path = r"../assets"
img_open = Image.open(os.path.join(root_path, filename))

img = np.zeros((160,160,4), dtype=int)
img[8:8+144, ] = np.asarray(img_open).astype(int)

# Divide the display (160x144) into 8x8 tiles
xticks = np.arange(0,160,8)
yticks = np.arange(0,144+16,8)

fig = plt.figure()
plt.imshow(img)
ax = plt.gca()

ax.set_xticks(xticks)
ax.set_yticks(yticks)
ax.grid(color="#ff0000")


# fig.canvas.mpl_connect("key_press_event", onkeypress)
fig.canvas.mpl_connect("button_press_event", onclick)

plt.show()

# response = input("Export collision array? ")
# if "y" in response.lower():
s = "unsigned char background_colliders [] = \n{"
for ind in range(collisions.shape[0]):
    value = hex(int(collisions[ind]))
    if ind % 20 == 0:
      s += f"\n  {value}, "
        
    else:
      s += f"{value}, "

s = s[:-2]
s += "\n};"

# print(s)
with open(f"../src/{os.path.splitext(filename.lower())[0]}_colliders.h", "w") as f:
   f.write(s)
