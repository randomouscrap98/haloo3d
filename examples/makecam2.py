import os
import math

# This one rotates around the center point
FILENAME = "pycam2.cam"
PITCH = 0.65
FRAMES = 60 # will make a full revolution in this time
YSTART = 3
RADIUS = 10

with open(FILENAME, "w+") as f:
    for i in range(FRAMES):
        ang = i * 2 * math.pi / FRAMES;
        zofs = RADIUS * math.cos(ang)
        xofs = RADIUS * math.sin(ang)
        f.write("{} {} {} {} {}\n".format(xofs, YSTART, zofs, -ang, PITCH))

print("Wrote to {}".format(FILENAME))
