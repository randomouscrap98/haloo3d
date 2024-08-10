import os

FILENAME = "pycam.cam"
FRAMES = 40
ZDRIFT = 17 # Total distance to travel
ZSTART = 0.5
YAWDRIFT = 1.0

zper = ZDRIFT / FRAMES
yawper = YAWDRIFT / FRAMES

zofs = ZSTART
yaw = 0

with open(FILENAME, "w+") as f:
    for i in range(FRAMES):
        f.write("0 0 {} {} 0\n".format(zofs, yaw))
        zofs += zper
        yaw += yawper

print("Wrote to {}".format(FILENAME))
