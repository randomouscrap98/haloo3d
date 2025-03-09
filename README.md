# Haloo3d

<img src="https://haloopdy.com/blog/resources/baseline_fast_default.gif" alt="a 3d software renderer">

A simple, modular 3D software renderer written in C99. It has a simple shader-esque
design for rendering 2d triangles, along with some idiosyncratic "batteries-included"
extras for performing 3d calculations, reading textures from `.ppm` files, 
reading models from `.obj` files, drawing sprites, printing text to screen, etc.
Though if you have your own libraries for all those, it's extremely easy
to put together your own system with only pieces of mine.

This library does not follow any normal graphics design patterns such as
opengl/etc. It holds onto no global state and nothing is even remotely
"fixed function". It's not an engine; getting a project off the ground with this
will take more effort. What you get in return is the ability to make incredibly 
fine-tuned rendering engines highly suited for specific tasks, and the ability 
to create very lightweight renderers with only exactly what you need.

This library is lightweight enough to compile and run on a **Pentium 2** at 
playable speeds. Though it can't go much older, as it uses floats for 
everything, necesitating an FPU. This may change in the distant future.

## Quick start

If you want to immediately jump into code, I recommend taking a look at 
[tests/t4_modelview.c](tests/t4_modelview.c).
It showcases creating a simple shader function using the library's macros,
loading model `.obj` files and texture `.ppm` files, creating a framebuffer,
creating the perspective / model / final view matrix, iterating over 
model triangles, clipping triangles, setting up initial shader values, 
and finally rendering triangles.

You can compile any of the tests from the main folder with a command
like the following:

```
# To compile "t1_simpletris.c", you'd do this from the repo root:
make tests/t1_simpletris.exe
```

Beyond the tests, I don't have traditional examples yet, just these
tests that I use to test the library. Some may not compile; I don't 
always catch the errors in tests as I change the library. If they compile
however, they can serve as a decent guide for some of the features in the 
library and how to use it.

## Structure

The library is split into parts based on your needs. The main rendering library
is just a single header, [haloo3d.h](haloo3d.h). The header is extremely simple,
including code for efficiently walking the pixels of a 2d triangle, some basic framebuffers,
and some helpful color macros. If you have your own model, texture, and 3d math libraries, 
you could use just this header as your rendering code.

I also include:
- A 3d math library: [haloo3d_3d.h](haloo3d_3d.h), [haloo3d_3d.c](haloo3d_3d.c)
- An .obj loading library: [haloo3d_obj.h](haloo3d_obj.h)
- A helper library for ppm textures, some memory management, etc: [haloo3d_ex.h](haloo3d_ex.h), [haloo3d_ex.c](haloo3d_ex.c)
- A library to connect haloo3d with [unigi](https://git.lumen.sh/Fierelier/unigi),
  an intermediary graphics frontend: [haloo3d_unigi.h](haloo3d_unigi.h), [haloo3d_unigi.c](haloo3d_unigi.c)
  - Not required if you're using SDL/etc

You can plug them all together to get a functioning 3d software renderer.

If you want the entire library, extras and all, wrapped up in a nice single object, 
you can do a simple `make` to create `haloo3d.a`, then include it when linking. Example:

```
cd haloo3d
make
# Assuming you supply these parts:
gcc -c main.c
gcc -o myprogram main.o haloo3d.a -lm
```

For more information on building, you can inspect the [Makefile](Makefile).
Building the tests is basically the exact command you'll want to use
when building your own program. When using the extended library
functionality, I would recommend using the link time optimization flag `-flto`.

Obviously you don't need to create `haloo3d.a` if you're JUST using the 
rendering functions; just include the `haloo3d.h` header and that's all you need.

## Rendering triangles

The main library `haloo3d.h` includes only what's necessary to *create*
a triangle drawing function using macros, and some framebuffer structs for
8 bit, 16 bit, and 32 bit pixels. The framebuffers assume no color format
internally, and you don't even have to use them when you create your own function.

Yes, there's no actual triangle drawing function. Instead, it is assumed
you will be interpolating linear values across spans of horizontal
pixels. Macros are provided to greatly simplify the process of 
creating the triangle drawing function. This effectively lets you write
a "shader", which is why no color format or even framebuffer struct is
assumed. I'm aware this may be a misuse of macros for a "modern c
programmer". I hope you will forgive my transgression.

### Basic flat color triangle example

For example, here is how you might write code 
that draws a flat color triangle to a framebuffer, saving it
to a `.ppm` file:

```c
#include "haloo3d.h"
#include "haloo3d_ex.h" // Only for writing framebuffer to file

void triangle(h3d_rastervert *rv, h3d_fb *buf) {
  // This is a wrapper macro around several other macros. Only for simple shaders.
  // This begins a pixel loop, where each iteration is one pixel at a position
  // in a 1D buffer indicated by bufi
  H3DTRI_EASY_BEGIN(rv, buf->width, buf->height, linpol, 0, bufi) {
    buf->buffer[bufi] = 0xFFFF; // Set each pixel in the tri to white
  }
  H3DTRI_SCAN_END(buf->width);
}

#define WIDTH 320
#define HEIGHT 240

int main() {
  uint16_t screenbuf[WIDTH*HEIGHT] = {0};
  h3d_fb screen = {  
    .width = WIDTH,
    .height = HEIGHT,
    .buffer = screenbuf
  }; // For convenience; haloo3d.h doesn't require a framebuffer

  h3d_rasterface rv;
  rv[0].pos[H3DX] = 20;
  rv[0].pos[H3DY] = 20;
  rv[1].pos[H3DX] = 20;
  rv[1].pos[H3DY] = 220;
  rv[2].pos[H3DX] = 270;
  rv[2].pos[H3DY] = 100;

  triangle(rv, &screen);

  h3d_fb_writeppmfile(&screen, "example.ppm", h3d_fb_out_A1R5G5B5);
}
```

The main thing to note here is that the triangle function is calling two
macros. The first one, `H3DTRI_EASY_BEGIN`, is generating quite a bit
of code in order to set up a loop which you write the body for. This
is the core of your shader: that body will be called once for every
pixel in the triangle. Notice that the macro doesn't actually take
a color format, framebuffer, or anything. It's just setting up a
loop; how you write your pixels is up to you. Heck I don't know,
you could use it for some kind of enemy AI that scans tiles 
in a triangle shape if you wanted to.

`H3DTRI_EASY_BEGIN` is actually a wrapper that calls the "real"
macros underneath. Most basic shaders can utilize this wrapper
without issue, even perspective-correct texture mapped triangles.

### More on the macros

`H3DTRI_EASY_BEGIN` is masking a lot of code. A "proper" triangle 
function would instead call all these macros:

```c
H3DTRI_CLAMP(rf, width, height); // Very stupid safety function; not required if you know what you're doing
H3DTRI_BEGIN(rf, sv, parea); // Triangle setup code
H3DTRI_SCAN_BEGIN(sv, parea, linpol, numlin, width, height, bufi) {
  // Your own per-row code. Unused in H3DTRI_EASY_BEGIN
}
H3DTRI_SHADER(bufi) {
  // You per-pixel code. This is the body for H3DTRI_EASY_BEGIN
  H3DTRI_LINPOL(linpol, numlin); // Call this at the end of the shader if using linear interpolants
}
H3DTRI_SCAN_END(width);
```

`rf`, `width`, `height`, and `numlin` are **values you pass in**.
- **rf**: the rasterface struct you're working on
- **width**: the width of whatever buffer you're working with
- **height**: the height of whatever buffer you're working with
- **numlin**: the number of linear interpolants you're working with

The rest are just **names** you want to give the setup variables, as you 
might want to use them in your shader.
- **sv**: the sorted vertexes from top to bottom (array of pointers)
- **parea**: 2 times the area of the triangle. Degenerate triangles (area=0)
  are automatically removed for you. May be negative before the call to
  `H3DTRI_SCAN_BEGIN`; always positive after
- **linpol**: the float array holding calculated linear interpolants per pixel
- **bufi**: calculated index into 1D array of size width * height

## Linear interpolants

For speed, old-school 3d renderers precalculated as much as possible before
the critical inner loop, then only used simple addition to step through
values per pixel. Sequences for which the relationship between subsequent values
can be expressed as constant addition or subtraction are called "linear", and
quite a few 3D effects can be achieved with simple linear interpolation. 

For instance, you can linearly interpolate over (1/z) for each vertex
across the triangle and get depth information at each pixel for a depth
buffer without having to divide per-pixel. You can linearly interpolate over
u/v coordinates per vertex for affine texture mapping. Or, you can linearly
interpolate for u/z and v/z for each vertex for perspective correct textures,
but you pay for it by requiring a divide per pixel to remove the z value from the
interpolants.

To use linear interpolants, you set them up per-vertex in your 
`h3d_rasterface` struct alongside the positions you normally set, 
then within your triangle function, you indicate the number of interpolants
you have and call the linear interpolant update macro `H3DTRI_LINPOL`.
For higher speed, it is better to call the macro specific to the number
of interpolants you have, such as `H3DTRI_LINPOL3`.

For example, here is a basic triangle function which uses 3 linear
interpolants per vertex to represent RGB vertex colors in a range from
0 to 1, shading the triangle with a mix of the colors based on the location:

```c
// Taken from tests/t1_simpletris.c
void triangle_vcol(h3d_rastervert *rv, h3d_fb *buf) {
  H3DTRI_EASY_BEGIN(rv, buf->width, buf->height, linpol, 3, bufi) {
    buf->buffer[bufi] =
        H3DC_A4R4G4B4(0xF, (uint16_t)(15 * linpol[0]),
                      (uint16_t)(15 * linpol[1]), (uint16_t)(15 * linpol[2]));
    H3DTRI_LINPOL3(linpol);
  }
  H3DTRI_SCAN_END(buf->width);
}
```

ALWAYS make sure to call the linpol macro at the end of your pixel shader,
otherwise you'll get very confusing results.

---

If you don't understand, don't worry, there's examples of perspective
correct texture mapping and more in the [tests](tests) directory. In
particular, you may want to look at 
[tests/speedtests/simple3dtest.c](tests/speedtests/simple3dtest.c)
for many such triangle rendering functions. Feel free to copy any of
them you want. Building and running the code will show you the output

## What is the "v1" folder?

It's the first version of the library that is now replaced. 
Since nobody is using haloo3d, I get to make huge breaking changes
whenever I want, and I'm the only one affected. I have some 
projects that depend on the old version, since it's entirely
incompatible with the latest version, so I keep it around
for ease of building.

You probably shouldn't use that one, but feel free to if
you want.


