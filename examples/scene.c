#include "../haloo3d.h"
#include "../haloo3dex_gen.h"
#include "../haloo3dex_img.h"
#include "../haloo3dex_obj.h"
#include "../haloo3dex_print.h"
#include "flower.h"
#include "mathc.h"
#include <stdlib.h>
#include <time.h>

#define DOLIGHTING

#define WIDTH 640
#define HEIGHT 480
#define ASPECT ((float)WIDTH / HEIGHT)
#define FOV 90.0
#define NEARCLIP 0.01
#define FARCLIP 100.0
#define LIGHTANG -MPI / 4.0
#define MINLIGHT 0.25
#define SKYSCALE 30
#define AVGWEIGHT 0.85

// this is the number of DYNAMIC objects..
#define NUMOBJECTS 4
#define NUMFLOWERS 300
#define FLOWERIND (NUMOBJECTS - 1)
#define NUMINSTANCES (NUMOBJECTS - 1 + NUMFLOWERS)
#define MAXCAM 1200

#define CALCTIME(thistime, start, end, sum)                                    \
  float thistime = 1000.0 * (float)(end - start) / CLOCKS_PER_SEC;             \
  if (sum == 0)                                                                \
    sum = thistime;                                                            \
  sum = AVGWEIGHT * sum + (1 - AVGWEIGHT) * thistime;

typedef struct {
  mfloat_t xofs;
  mfloat_t yofs;
  mfloat_t zofs;
  mfloat_t yaw;
  mfloat_t pitch;
} camset;

uint16_t redflower[64] = H3D_FLOWER(0xFE55, 0xF6C4, 0xFFE0, 0xFD44, 0xF492);

int readcam(camset *set, int max, char *filename) {
  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    dieerr("Can't open %s for reading cam\n", filename);
  }
  int num = 0;
  while (5 == fscanf(f, "%f %f %f %f %f", &set[num].xofs, &set[num].yofs,
                     &set[num].zofs, &set[num].yaw, &set[num].pitch)) {
    // char linebuf[1024];
    //  fgets(linebuf, sizeof(linebuf), f)) {
    num++;
    if (num >= max) {
      eprintf("Camera file too big! Ignoring rest\n");
      break;
    }
  }
  fclose(f);
  printf("Read %d camlines from %s\n", num, filename);
  return num;
}

int main(int argc, char **argv) {

  if (argc != 4) {
    eprintf("WARN: THIS PROGRAM GENERATES A LOT OF FILES!\n");
    dieerr("You must pass in the following:\n- obj file .obj\n- texture file "
           ".ppm\n- camera file (xofs yofs zofs yawdeg pitchdeg)\n");
    //- obj xofs\n- obj yofs\n- obj zofs\n- obj rotation deg\n");
  }

  // Load the junk + generate stuff
  haloo3d_obj models[NUMOBJECTS];
  haloo3d_fb textures[NUMOBJECTS];
  haloo3d_obj_loadfile(models, argv[1]);
  haloo3d_img_loadppmfile(textures, argv[2]);
  haloo3d_gen_1pxgradient(textures + 1, 0xF44F, 0xF001, 32);
  haloo3d_gen_skybox(models + 1);
  uint16_t checkcols[2] = {0xF0A0, 0xF270};
  haloo3d_gen_checkerboard(textures + 2, checkcols, 2, 32);
  haloo3d_gen_sloped(models + 2, 61, 1.0, 1.25);
  haloo3d_fb_init_tex(textures + 3, 8, 8);
  memcpy(textures[3].buffer, redflower, sizeof(uint16_t) * 64);
  // memset(textures[3].buffer, 0xFF, sizeof(uint16_t) * 64);
  haloo3d_gen_crossquad(models + 3, textures + 3);

  camset cams[MAXCAM];
  int numcams = readcam(cams, MAXCAM, argv[3]);

  // Create the camera matrix, which DOES change. In this one,
  // we move the camera instead of the model
  haloo3d_camera camera;
  haloo3d_camera_init(&camera);

  // Create the perspective matrix, which doesn't change
  mfloat_t perspective[MAT4_SIZE];
  haloo3d_perspective(perspective, FOV, ASPECT, NEARCLIP, FARCLIP);

  // Lighting. Note that for performance, the lighting is always calculated
  // against the base model, and is thus not realistic if the object rotates in
  // the world. This can be fixed easily, since each object gets its own
  // lighting vector, which can easily be rotated in the opposite direction of
  // the model
  struct vec3 light;
  vec3(light.v, 0, -MCOS(LIGHTANG), MSIN(LIGHTANG));

  int totalfaces = 0;
  int totalverts = 0;
  haloo3d_obj_instance objects[NUMINSTANCES];
  for (int i = 0; i < NUMINSTANCES; i++) {
    if (i < FLOWERIND) {
      haloo3d_objin_init(objects + i, models + i, textures + i);
    } else { // Setup the flowers
      haloo3d_objin_init(objects + i, models + FLOWERIND, textures + FLOWERIND);
      objects[i].cullbackface = 0;
      objects[i].scale = 0.5;
      int rvi = rand() % models[2].numvertices;
      vec3_assign(objects[i].pos.v, models[2].vertices[rvi].v);
      objects[i].pos.y += 0.5;
    }
    totalfaces += objects[i].model->numfaces;
    totalverts += objects[i].model->numvertices;
  }
#ifdef DOLIGHTING
  objects[0].lighting = &light;
  objects[2].lighting = &light;
#endif
  // objects[0].pos.z = 0;
  // objects[0].pos.y = -10;
  // vec3(objects[2].pos.v, 0.5, 0.8, 0.5);
  objects[0].pos.y = 1;
  objects[1].scale = SKYSCALE;

  // Now we create a framebuffer to draw the triangle into
  haloo3d_fb fb;
  haloo3d_fb_init(&fb, WIDTH, HEIGHT);

  // Printing to screen needs tracking
  haloo3d_print_tracker t;
  char printbuf[8192];
  haloo3d_print_initdefault(&t, printbuf, sizeof(printbuf));
  t.fb = &fb;
  t.logprints = 1;

  // Storage stuff
  mfloat_t matrix3d[MAT4_SIZE], matrixcam[MAT4_SIZE], matrixscreen[MAT4_SIZE],
      matrixmodel[MAT4_SIZE];
  haloo3d_facef outfaces[H3D_FACEF_MAXCLIP];
  struct vec3 tmp1;
  haloo3d_facef face, baseface;
  struct vec4 *vert_precalc;
  mallocordie(vert_precalc, sizeof(struct vec4) * H3D_OBJ_MAXVERTICES);
  char fname[1024];
  // float sumverts = 0, sumdraw = 0, sumframe = 0, suminit = 0, sumclip = 0;
  // clock_t begin, end, vertend, drawend, initend, clipend, tempstart, tempend;
  clock_t begin, end;
  float sumframe = 0;
  // float sumframe = 0;
  // clock_t begin, end;
  int totaldrawn = 0;

  eprintf("Scene has %d tris, %d verts\n", totalfaces, totalverts);

  // -----------------------------------
  //     Actual rendering
  // -----------------------------------

  for (int cami = 0; cami < numcams; cami++) {
    begin = clock();
    // vertend = begin;
    // drawend = begin;
    // clipend = begin;
    totaldrawn = 0;

    haloo3d_print_refresh(&t);
    camera.pos.x = cams[cami].xofs;
    camera.pos.y = cams[cami].yofs;
    camera.pos.z = cams[cami].zofs;
    camera.yaw = cams[cami].yaw;
    camera.pitch = cams[cami].pitch + MPI_2;

    // REMEMBER TO CLEAR DEPTH BUFFER
    haloo3d_fb_cleardepth(&fb);

    // Screen matrix calc. We multiply the modelview matrix with this later
    haloo3d_camera_calclook(&camera, matrixcam);
    mat4_inverse(matrixcam, matrixcam);
    mat4_multiply(matrixscreen, perspective, matrixcam);

    // initend = clock();

    // Iterate over objects
    for (int i = 0; i < NUMINSTANCES; i++) {
      // tempstart = clock();
      //  Setup final model matrix and the precalced vertices
      vec3_add(tmp1.v, objects[i].pos.v, objects[i].lookvec.v);
      haloo3d_my_lookat(matrixmodel, objects[i].pos.v, tmp1.v, camera.up.v);
      haloo3d_mat4_scale(matrixmodel, objects[i].scale);
      mat4_multiply(matrix3d, matrixscreen, matrixmodel);
      haloo3d_precalc_verts(objects[i].model, matrix3d, vert_precalc);
      // tempend = clock();
      // vertend += (tempend - tempstart);
      //  Iterate over object faces
      for (int fi = 0; fi < objects[i].model->numfaces; fi++) {
        // Copy face values out of precalc array and clip them
        haloo3d_make_facef(objects[i].model->faces[fi], vert_precalc,
                           objects[i].model->vtexture, face);
        // tempstart = clock();
        int tris = haloo3d_facef_clip(face, outfaces);
        // tempend = clock();
        // clipend += (tempend - tempstart);
        // tempstart = tempend;
        for (int ti = 0; ti < tris; ti++) {
          int backface = !haloo3d_facef_finalize(outfaces[ti]);
          if (objects[i].cullbackface && backface) {
            continue;
          }
          totaldrawn++;
          mfloat_t intensity = 1.0;
          if (objects[i].lighting) {
            haloo3d_obj_facef(objects[i].model, objects[i].model->faces[fi],
                              baseface);
            intensity =
                haloo3d_calc_light(objects[i].lighting->v, MINLIGHT, baseface);
          }
          //   We still have to convert the points into the view
          haloo3d_facef_viewport_into(outfaces[ti], WIDTH, HEIGHT);
          haloo3d_texturedtriangle(&fb, objects[i].texture, intensity,
                                   outfaces[ti]);
        }
        // tempend = clock();
        // drawend += (tempend - tempstart);
      }
    }

    end = clock();

    // CALCTIME(thisinittime, begin, initend, suminit);
    // CALCTIME(thisverttime, begin, vertend, sumverts);
    // CALCTIME(thiscliptime, begin, clipend, sumclip);
    // CALCTIME(thisdrawtime, begin, drawend, sumdraw);
    CALCTIME(thisframetime, begin, end, sumframe);
    // haloo3d_print(&t,
    //               "Init work: %.2f (%.2f)\nVert calc: %.2f (%.2f)\n     Clip:
    //               "
    //               "%.2f (%.2f)\n     Draw: %.2f"
    //               " (%.2f)\n    Frame: %.2f (%.2f)\nTris: %d / %d\nVerts:
    //               %d\n", thisinittime, suminit, thisverttime, sumverts,
    //               thiscliptime, sumclip, thisdrawtime, sumdraw,
    //               thisframetime, sumframe, totaldrawn, totalfaces,
    //               totalverts);
    haloo3d_print(&t, "Frame: %.2f (%.2f)\nTris: %d / %d\nVerts: %d\n",
                  thisframetime, sumframe, totaldrawn, totalfaces, totalverts);

    sprintf(fname, "scene_%04d.ppm", cami);
    haloo3d_img_writeppmfile(&fb, fname);
  }

  for (int i = 0; i < NUMOBJECTS; i++) {
    haloo3d_obj_free(models + i);
    haloo3d_fb_free(textures + i);
  }
}
