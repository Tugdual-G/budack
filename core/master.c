#include "master.h"
#include "budack_core.h"
#include "opengl/include/glad/glad.h" // glad should be included before glfw3
#include "opengl/render.h"
#include "tiff_images.h"
#include <math.h>
#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

GLenum glCheckError_(const char *file, int line);
#define glCheckError() glCheckError_(__FILE__, __LINE__)

int master(int world_size, Param param, double a[2], double b[2]) {

  printf("Number of cores : %i \n", world_size);

  double max_memory;

  unsigned int nx = *param.nx, ny = *param.ny, depth = *param.depth,
               maxit = *param.maxit, minit = *param.minit;
  double D = *param.D;
  // in bytes
  max_memory = 3 * nx * ny * (sizeof(uint16_t) + sizeof(uint32_t)) +
               LENGTH_STRT * sizeof(double);
  // In GiB
  max_memory /= (double)1024 * 1024;
  printf("Max memory usage :\x1b[32m %.0f MiB \x1b[0m\n", max_memory);

  printf("nx = %d ; ny = %d ; depth = %u \n", nx, ny, depth);
  printf("maxit = %d ; minit = %d ; Points per pixels %.2f \n", maxit, minit,
         D);
  export_param(param);

  ////////////////////////////////////////////////
  //   Searching for points on the boundary
  ////////////////////////////////////////////////

  // There is no need to parallelise this part

  unsigned int length_brdr = LENGTH_STRT / (world_size - 1);

  ////////////////////////////////////////////////
  //   Cumputing the trajectories
  ////////////////////////////////////////////////

  uint32_t *R_32 = NULL, *G_32 = NULL, *B_32 = NULL;
  R_32 = (uint32_t *)calloc(nx * ny, sizeof(uint32_t));
  G_32 = (uint32_t *)calloc(nx * ny, sizeof(uint32_t));
  B_32 = (uint32_t *)calloc(nx * ny, sizeof(uint32_t));
  if (!B_32 | !R_32 | !G_32) {
    printf("\n Error, no memory allocated for trajectories sum \n");
    exit(1);
  }

  clock_t t_begin = clock();
  border(depth, length_brdr, NULL);
  if (param.cycles_per_update != 0) {
    recieve_and_render(R_32, G_32, B_32, a, b, nx, ny, world_size,
                       param.cycles_per_update);
  } else {
    recieve_and_draw(R_32, G_32, B_32, a, b, nx, ny, world_size);
  }

  printf("\nTime elapsed computing trajectories %f s \n",
         (double)(clock() - t_begin) / (double)CLOCKS_PER_SEC);

  /* uint16_t *R_16 = NULL; */
  /* uint16_t *G_16 = NULL; */
  /* uint16_t *B_16 = NULL; */

  /* // Storing variables on disk */
  /* mirror_traj(ny, nx, R_32); // Make the image symetric */
  /* mirror_traj(ny, nx, G_32); // Make the image symetric */
  /* mirror_traj(ny, nx, B_32); // Make the image symetric */
  /* R_16 = (uint16_t *)calloc(nx * ny, sizeof(uint16_t)); */
  /* G_16 = (uint16_t *)calloc(nx * ny, sizeof(uint16_t)); */
  /* B_16 = (uint16_t *)calloc(nx * ny, sizeof(uint16_t)); */
  /* if (!R_16 | !G_16 | !B_16) { */
  /*   printf("\n Error, no memory allocated for RGB arrays \n"); */
  /*   exit(1); */
  /* } */

  /* save("r.uint32", R_32, sizeof(uint32_t), nx * ny); */
  /* save("g.uint32", G_32, sizeof(uint32_t), nx * ny); */
  /* save("b.uint32", B_32, sizeof(uint32_t), nx * ny); */

  /* normalize_32_to_16bits(R_32, R_16, nx * ny); */
  /* free(R_32); */
  /* normalize_32_to_16bits(G_32, G_16, nx * ny); */
  /* free(G_32); */
  /* normalize_32_to_16bits(B_32, B_16, nx * ny); */
  /* free(B_32); */

  /* unsigned outdir_str_len = strlen(param.output_dir); */
  /* char filename[MAX_PATH_LENGTH + 21] = {'\0'}; */
  /* strncpy(filename, param.output_dir, MAX_PATH_LENGTH); */
  /* if (param.output_dir[outdir_str_len - 1] != '/') { */
  /*   filename[outdir_str_len] = '/'; */
  /* } */
  /* strncat(filename, "image.tiff", 11); */

  /* write_tiff_16bitsRGB(filename, R_16, G_16, B_16, nx, ny); */
  /* free(R_16); */
  /* free(G_16); */
  /* free(B_16); */

  return 0;
}

void recieve_and_draw(uint32_t *R, uint32_t *G, uint32_t *B, double a[2],
                      double b[2], unsigned int nx, unsigned int ny,
                      int world_size) {
  clock_t t0, t = 0;
  write_progress(0);
  MPI_Request requ;
  int completion_flag = 0;
  Pts_msg *recbuff = (Pts_msg *)malloc(sizeof(Pts_msg) * PTS_MSG_SIZE);
  if (!recbuff) {
    printf("Error: recbuff not allocated \n");
    exit(1);
  }
  MPI_Recv_init(recbuff, sizeof(Pts_msg) * PTS_MSG_SIZE, MPI_BYTE,
                MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &requ);

  while (completion_flag < (world_size - 1)) {

    t0 = clock();
    MPI_Start(&requ);
    MPI_Wait(&requ, MPI_STATUS_IGNORE);
    /* MPI_Recv(recbuff, sizeof(Pts_msg) * PTS_MSG_SIZE, MPI_BYTE, */
    /*          MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status); */
    t += clock() - t0;

    if (recbuff[0].color) {
      for (unsigned int i = 0; i < PTS_MSG_SIZE; ++i) {
        switch (recbuff[i].color) {
        case 'r':
          draw_trajectories(R, recbuff[i].x, recbuff[i].y, recbuff[i].nit, a, b,
                            nx, ny);
          break;
        case 'g':
          draw_trajectories(G, recbuff[i].x, recbuff[i].y, recbuff[i].nit, a, b,
                            nx, ny);
          break;
        case 'b':
          draw_trajectories(B, recbuff[i].x, recbuff[i].y, recbuff[i].nit, a, b,
                            nx, ny);
          break;
        }
      }

    } else {
      ++completion_flag;
    }
  }
  free(recbuff);
  write_progress(-2);
  printf("\nmaster waiting time : %lf s \n", (double)t / CLOCKS_PER_SEC);
}

void recieve_and_render(uint32_t *R, uint32_t *G, uint32_t *B, double a[2],
                        double b[2], unsigned int nx, unsigned int ny,
                        int world_size, unsigned int cycles_per_update) {

  unsigned int redu_fact = 1;

  /* clock_t t0, t = 0; */
  write_progress(0);
  MPI_Request requ;
  Pts_msg *recbuff = (Pts_msg *)malloc(sizeof(Pts_msg) * PTS_MSG_SIZE);
  if (!recbuff) {
    printf("Error: recbuff not allocated \n");
    exit(1);
  }
  MPI_Recv_init(recbuff, sizeof(Pts_msg) * PTS_MSG_SIZE, MPI_BYTE,
                MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &requ);

  Fargs args = {
      .world_size = world_size,
      .nx = nx,
      .ny = ny,
      .n_it = cycles_per_update,
      .requ = &requ,
      .recbuff = recbuff,
      .a = a,
      .b = b,
      .R = R,
      .G = G,
      .B = B,
      .redu_fact = redu_fact,
  };

  Render_object rdr_obj = {
      .width = nx,
      .height = ny,
      .Runit = 0,
      .Gunit = 1,
      .Bunit = 2,
      .recbuff = recbuff,
      .recbuff_length = PTS_MSG_SIZE,
      .recbuff_unit = 0,
      .R = R,
      .G = G,
      .B = B,
      .dx = (a[1] - a[0]) / (double)nx,
  };

  render_init(&rdr_obj);
  render_loop(&rdr_obj, callback, &args);
  render_finalize(&rdr_obj);
  free(recbuff);
  write_progress(-2);
  /* printf("\n Rmax = %u , Gmax = %u, Bmax = %u \n", *args.Rmax, *args.Gmax, */
  /*        *args.Bmax); */
  printf("\nmaster waiting time : %lf s \n",
         (double)args.waiting_t / CLOCKS_PER_SEC);
}

int callback(Render_object *rdr_obj, void *fargs) {

  glUseProgram(rdr_obj->compute_program);
  /* glBindBuffer(GL_SHADER_STORAGE_BUFFER, rdr_obj->recbuff_ssbo); */
  static int completion_flag = 0;
  Fargs *args = (Fargs *)fargs;
  Pts_msg *rec = args->recbuff;
  clock_t t0;
  args->n_it = 1;
  unsigned int it = 0;
  while ((completion_flag < (args->world_size - 1)) && (it < args->n_it)) {
    ++it;
    MPI_Start(args->requ);
    t0 = clock();
    MPI_Wait(args->requ, MPI_STATUS_IGNORE);
    args->waiting_t += clock() - t0;
    if (rec[0].color) {

      glNamedBufferSubData(rdr_obj->recbuff_ssbo, 0,
                           rdr_obj->recbuff_length * sizeof(Pts_msg), rec);

      glDispatchCompute((unsigned int)rdr_obj->recbuff_length, 1, 1);

      glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    } else {
      ++completion_flag;
    }
  }

  return (completion_flag != (args->world_size - 1));
}

GLenum glCheckError_(const char *file, int line) {
  GLenum errorCode;
  while ((errorCode = glGetError()) != GL_NO_ERROR) {
    switch (errorCode) {
    case GL_INVALID_ENUM: {
      printf(" ERROR : INVALID_ENUM , file: %s  ,line: %i \n", file, line);
      break;
    }
    case GL_INVALID_VALUE: {
      printf(" ERROR : INVALID_VALUE , file: %s  ,line: %i \n", file, line);
      break;
    }
    case GL_INVALID_OPERATION: {
      printf(" ERROR : INVALID_OPERATION , file: %s  ,line: %i \n", file, line);
      break;
    }
    case GL_STACK_OVERFLOW: {
      printf(" ERROR : STACK_OVERFLOW , file: %s  ,line: %i \n", file, line);
      break;
    }
    case GL_STACK_UNDERFLOW: {
      printf(" ERROR : STACK_UNDERFLOW , file: %s ,line: %i \n", file, line);
      break;
    }
    case GL_OUT_OF_MEMORY: {
      printf(" ERROR : OUT_OF_MEMORY , file: %s ,line: %i \n", file, line);
      break;
    }
    case GL_INVALID_FRAMEBUFFER_OPERATION: {
      printf(" ERROR : INVALID_FRAMEBUFFER_OPERATION , file: %s ,line: %i \n",
             file, line);
      break;
    }
    }
  }
  return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)
