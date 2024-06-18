#include "master.h"
#include "budack_core.h"
#include "opengl/include/glad/glad.h" // glad should be included before glfw3
#include "opengl/render.h"
#include "tiff_images.h"
#include <math.h>
#include <mpi.h>
// #include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define N_REQUESTS 8

GLenum glCheckError_(const char *file, int line);
#define glCheckError() glCheckError_(__FILE__, __LINE__)

int master(int world_size, Param param, double a[2], double b[2]) {

  if (PTS_MSG_SIZE * N_REQUESTS != 256) {
    printf("Error : PTS_MSG_SIZE * N_REQUESTS should be equal to 256. \n");
  }

  printf("Number of cores : %i \n", world_size);

  unsigned int nx = *param.nx, ny = *param.ny, depth = *param.depth,
               maxit = *param.maxit, minit = *param.minit;
  double D = *param.D;

  printf("nx = %d ; ny = %d ; depth = %u \n", nx, ny, depth);
  printf("maxit = %d ; minit = %d ; Points per pixels %.2f \n", maxit, minit,
         D);
  export_param(param);

  ////////////////////////////////////////////////
  //   Searching for points on the boundary
  ////////////////////////////////////////////////

  unsigned int length_brdr = LENGTH_STRT / (world_size - 1);
  clock_t t_begin = clock();
  border(depth, length_brdr, NULL);

  ////////////////////////////////////////////////
  //   Cumputing the trajectories
  ////////////////////////////////////////////////

  /* R_32 = (uint32_t *)calloc(nx * ny, sizeof(uint32_t)); */
  /* G_32 = (uint32_t *)calloc(nx * ny, sizeof(uint32_t)); */
  /* B_32 = (uint32_t *)calloc(nx * ny, sizeof(uint32_t)); */
  /* if (!B_32 | !R_32 | !G_32) { */
  /*   printf("\n Error, no memory allocated for trajectories sum \n"); */
  /*   exit(1); */
  /* } */

  recieve_and_render(a, b, nx, ny, world_size, param.cycles_per_update);

  printf("\nTime elapsed computing trajectories %f s \n",
         ((double)clock() - (double)t_begin) / CLOCKS_PER_SEC);

  return 0;
}

void recieve_and_render(double a[2], double b[2], unsigned int nx,
                        unsigned int ny, int world_size,
                        unsigned int cycles_per_update) {

  unsigned int redu_fact = 1;

  write_progress(0);
  MPI_Request requ[N_REQUESTS];
  Pts_msg *recbuff =
      (Pts_msg *)malloc(sizeof(Pts_msg) * PTS_MSG_SIZE * N_REQUESTS);
  if (!recbuff) {
    printf("Error: recbuff not allocated \n");
    exit(1);
  }
  for (unsigned int i = 0; i < N_REQUESTS; ++i) {
    MPI_Recv_init(recbuff + PTS_MSG_SIZE * i, sizeof(Pts_msg) * PTS_MSG_SIZE,
                  MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
                  (requ + i));
  }

  Fargs args = {
      .world_size = world_size,
      .nx = nx,
      .ny = ny,
      .n_it = cycles_per_update,
      .requ = requ,
      .recbuff = recbuff,
      .a = a,
      .b = b,
      .R = NULL,
      .G = NULL,
      .B = NULL,
      .Rmax = 1,
      .Gmax = 1,
      .Bmax = 1,
      .redu_fact = redu_fact,
      .waiting_t = 0,
  };

  Render_object rdr_obj = {
      .width = nx,
      .height = ny,
      .Runit = 0,
      .Gunit = 1,
      .Bunit = 2,
      .recbuff = recbuff,
      .recbuff_length = PTS_MSG_SIZE * N_REQUESTS,
      .recbuff_unit = 0,
      .R = NULL,
      .G = NULL,
      .B = NULL,
      .dx = (a[1] - a[0]) / (double)nx,
  };

  rdr_obj.maxv =
      (uint32_t *)calloc(rdr_obj.recbuff_length * 3, sizeof(uint32_t));

  render_init(&rdr_obj);
  render_loop(&rdr_obj, callback, &args);
  render_finalize(&rdr_obj);
  free(recbuff);
  write_progress(-2);
  printf("\nmaster waiting time : %lf s \n",
         (double)args.waiting_t / CLOCKS_PER_SEC);
}

int callback(Render_object *rdr_obj, void *fargs) {

  glUseProgram(rdr_obj->iterate_program);
  /* glBindBuffer(GL_SHADER_STORAGE_BUFFER, rdr_obj->recbuff_ssbo); */
  static int completion_flag = 0;
  Fargs *args = (Fargs *)fargs;
  Pts_msg *rec = args->recbuff;
  clock_t t0;
  args->n_it = 1;
  unsigned int it = 0, n_requests = N_REQUESTS;
  while ((completion_flag < (args->world_size - 1)) && (it < args->n_it)) {
    ++it;
    for (unsigned int i = 0; i < n_requests; ++i) {
      MPI_Start((args->requ + i));
    }
    t0 = clock();
    for (unsigned int i = 0; i < n_requests; ++i) {
      MPI_Wait((args->requ + i), MPI_STATUS_IGNORE);
      /* printf(" flag = %u\n", rec[i * PTS_MSG_SIZE].color); */

      completion_flag += rec[i * PTS_MSG_SIZE].color != 0 ? 0 : 1;
      if (completion_flag == (args->world_size - 2)) {
        n_requests = 1;
      }
    }
    args->waiting_t += clock() - t0;
    glNamedBufferSubData(rdr_obj->recbuff_ssbo, 0,
                         rdr_obj->recbuff_length * sizeof(Pts_msg), rec);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    /* In case one of the process returns, */
    /* we do not know where it we do not want to leave starting points in the */
    /* buffer. Otherwise the GPU would have to process them multiples times. */
    for (unsigned int k = 0; k < PTS_MSG_SIZE * N_REQUESTS; ++k) {
      rec[k].color = 0;
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
