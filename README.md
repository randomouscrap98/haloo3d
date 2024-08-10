# Haloo3d

A simple software renderer written in C which is made specifically for games
I might want to make. It is not made to be highly configurable or to follow any
kind of guidelines or APIs. It will most likely not be very pretty or performant.

It renders into a 16 bit buffer of A4R4G4B4, which you can do whatever you want 
with. 

## Testing

There's a lot of examples in the examples folder, but some of them may be
broken. I use the examples as a testing ground and if the library changes,
the "examples" break. I should probably name them "test_samples" or something.

### Scene.c

The main file for testing is currently [examples/scene.c](examples/scene.c). 
You can compile this with:

```
make examples/scene.exe
```

This will automatically compile the core library, the extended library, and
link it all into the scene example, producing the file you passed to make.
You can then run it like:

```
./examples/scene.exe model.obj texture.ppm camera.cam
```

The model is a standard wavefront obj file, though I wrote a custom parser
and it might not like all the files. The texture is any .ppm image. You can
convert an image to ppm with imagick:

```
convert texture.png texture.ppm
```

It should automatically be in the right format, but just in case it's not, 
these programs only load P6 binary ppm images.

The camera file is a simple flat file where each line has 5 floats:
- x offset
- y offset
- z offset
- yaw (radians)
- pitch (radians)

Don't believe the program if it says degrees: it's radians. There are a couple
python programs you can use to generate interesting cameras. Currently, I use
[examples/makecam2.py](examples/makecam2.py), which generates a camera which 
rotates around the "mountain" generated by `scene.c`. Your object is placed on 
top of the mountain, unscaled. I use this one as a control when testing 
performance.
