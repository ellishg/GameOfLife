#ifndef GAMEOFLIFE_H_
#define GAMEOFLIFE_H_

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>

#ifdef __APPLE__
  #include <OpenCL/opencl.h>
  #include <OpenGL/gl.h>
  #include <OpenGL/opengl.h>
#else
  #include <CL/cl.h>
  #include <GL/gl.h>
  #include <GL/glut.h>
#endif

#define check_error(err, str) check_for_error(err, str, __FILE__, __LINE__)

class GameOfLife
{
public:

  GameOfLife(GLuint tex, size_t _width, size_t _height, const char * kernel_filename);

  ~GameOfLife();

  void compute_next_frame();

private:

  const GLuint texture;

  const size_t width, height;

  int cur_state;

  cl_context context;

  cl_command_queue command_queue;

  cl_program program;

  size_t global_size[2];

  cl_kernel next_generation;

  cl_mem game_state[2];
  cl_mem framebuffer;

  cl_int err;

  void check_for_error(cl_int err, const char * str, const char * file, int line_number);
};

#endif /* end of include guard: GAMEOFLIFE_H_ */
