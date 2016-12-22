#include "GameOfLife.h"

GameOfLife::GameOfLife(GLuint tex, size_t _width, size_t _height, const char * kernel_filename) : texture(tex), width(_width), height(_height), cur_state(0)
{
  int kernel_fd;
  if ((kernel_fd = open(kernel_filename, O_RDONLY)) == -1)
  {
    perror("Could not open kernel file");
    exit(1);
  }

  cl_uint num_available_platforms = -1;
  cl_uint num_available_devices = -1;

  err = clGetPlatformIDs(1, NULL, &num_available_platforms);
  check_error(err, "Unable to get the number of platform IDs");
  if (true)
  {
    fprintf(stdout, "%d platform(s) available\n", num_available_platforms);
  }

  cl_platform_id platforms[num_available_platforms];

  err = clGetPlatformIDs(num_available_platforms, platforms, NULL);
  check_error(err, "Unable to get platform IDs");

  char name[128];
  if (true)
  {
    for (int i = 0; i < num_available_platforms; i++)
    {
      clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 128, name, NULL);
      fprintf(stdout, "\tplatform[%i] name: %s\n", i, name);
    }
  }

  cl_platform_id my_platform = platforms[0];

  err = clGetDeviceIDs(my_platform, CL_DEVICE_TYPE_ALL, 0, NULL, &num_available_devices);
  check_error(err, "Unable to get the number of device IDs");
  if (true)
  {
    fprintf(stdout, "%d device(s) available\n", num_available_devices);
  }

  cl_device_id devices[num_available_devices];

  err = clGetDeviceIDs(my_platform, CL_DEVICE_TYPE_ALL, num_available_devices, devices, NULL);
  check_error(err, "Unable to get device IDs");

  cl_device_id my_device = NULL;
  cl_device_type device_type;

  for (int i = 0; i < num_available_devices; i++)
  {
    clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 128, name, NULL);
    clGetDeviceInfo(devices[i], CL_DEVICE_TYPE, sizeof(cl_device_type), &device_type, NULL);
    if (true)
    {
      fprintf(stdout, "\tdevice[%i] is %s\n", i, name);
    }
    switch (device_type)
    {
      case CL_DEVICE_TYPE_GPU:
        if (true)
        {
          my_device = devices[i];
        }
        break;
      case CL_DEVICE_TYPE_CPU:
        if (false)
        {
          my_device = devices[i];
        }
        break;
      default:
        break;
    }
  }

  if (my_device == NULL)
  {
    check_error(-1, "Unable to find device");
  }

  if (true)
  {
    clGetDeviceInfo(my_device, CL_DEVICE_NAME, 100, name, NULL);
    fprintf(stdout, "Chosen device: %s\n", name);
  }

#ifdef __APPLE__
  CGLContextObj gl_context = CGLGetCurrentContext();
  CGLShareGroupObj gl_share_group = CGLGetShareGroup(gl_context);

  cl_context_properties properties[] = {
    CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)gl_share_group,
    0
  };
#elif __linux__
  cl_context_properties properties[] = {
    CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
    CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
    CL_CONTEXT_PLATFORM, (cl_context_properties)fluid_platform,
    0
  };
#endif

  context = clCreateContext(properties, 1, &my_device, NULL, NULL, &err);
  check_error(err, "Unable to create cl context");

  command_queue = clCreateCommandQueue(context, my_device, CL_QUEUE_PROFILING_ENABLE, &err);
  check_error(err, "Unable to create command queue");

  struct stat buf;
  if (fstat(kernel_fd, &buf) == -1)
  {
    perror("fstat");
    exit(1);
  }
  const size_t kernel_src_size = buf.st_size;
  char * kernel_src;
  if ((kernel_src = (char *)mmap(NULL, kernel_src_size, PROT_READ, MAP_PRIVATE | MAP_FILE, kernel_fd, 0)) == MAP_FAILED)
  {
    perror("Could not read kernel file");
    exit(1);
  }

  program = clCreateProgramWithSource(context, 1, (const char **)&kernel_src, &kernel_src_size, &err);
  check_error(err, "Unable to create program with source");

  if (munmap(kernel_src, kernel_src_size) == -1)
  {
    perror("Could not release memory");
    exit(1);
  }

  char kernel_definitions[64];
  snprintf(kernel_definitions, 64, "-D WIDTH=%zu -D HEIGHT=%zu", width, height);

  err = clBuildProgram(program, 1, &my_device, kernel_definitions, NULL, NULL);
  const size_t max_log_length = 16384;
  char log[max_log_length];
  clGetProgramBuildInfo(program, my_device, CL_PROGRAM_BUILD_LOG, max_log_length, log, NULL);
  fprintf(stderr, "%s", log);
  check_error(err, "Unable to build program");

  global_size[0] = width;
  global_size[1] = height;

  next_generation = clCreateKernel(program, "next_generation", &err);
  check_error(err, "Unable to create next_generation");

  game_state[0] = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_char) * width * height, NULL, &err);
  game_state[1] = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_char) * width * height, NULL, &err);
  check_error(err, "Unable to create game state buffer");


  srand(time(NULL));
  cl_char initial_state[width * height];
  for (size_t i = 0; i < width * height; i++)
  {
    initial_state[i] = rand() % 2;
  }

  err = clEnqueueWriteBuffer(command_queue, game_state[cur_state ^ 1], CL_TRUE, 0, sizeof(cl_char) * width * height, (void *)initial_state, 0, NULL, NULL);
  check_error(err, "Unable to fill game_state");

  framebuffer = clCreateFromGLTexture2D(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, texture, &err);
  check_error(err, "Unable to create cl/gl texture");

  err = clFinish(command_queue);
  check_error(err, "Unable to finish command queue");
}

GameOfLife::~GameOfLife()
{
  clReleaseMemObject(game_state[0]);
  clReleaseMemObject(game_state[1]);
  clReleaseMemObject(framebuffer);

  clReleaseKernel(next_generation);

  clReleaseProgram(program);
  clReleaseCommandQueue(command_queue);
  clReleaseContext(context);
}

void GameOfLife::compute_next_frame()
{
  glFinish();

  err = clEnqueueAcquireGLObjects(command_queue, 1, &framebuffer, 0, 0, NULL);
  check_error(err, "Unable to acquire texture");

  //__kernel void next_generation(__global char * src, __global char * dest, write_only image2d_t framebuffer)
  err = clSetKernelArg(next_generation, 0, sizeof(cl_mem), &game_state[cur_state ^ 1]);
  err |= clSetKernelArg(next_generation, 1, sizeof(cl_mem), &game_state[cur_state]);
  err |= clSetKernelArg(next_generation, 2, sizeof(cl_mem), &framebuffer);
  check_error(err, "Unable to set args");

  err = clEnqueueNDRangeKernel(command_queue, next_generation, 2, NULL, global_size, NULL, 0, NULL, NULL);
  check_error(err, "Unable to enque next_generation");

  err = clFinish(command_queue);
  check_error(err, "Unable to finish queue");

  err = clEnqueueReleaseGLObjects(command_queue, 1, &framebuffer, 0, 0, NULL);
  check_error(err, "Unable to release texture");

  cur_state ^= 1;
}

void GameOfLife::check_for_error(cl_int err, const char * str, const char * file, int line_number)
{
  if (err != CL_SUCCESS)
  {
    fprintf(stderr, "<%s>:%d %s: %d\n", file, line_number, str, (int)err);
    exit(1);
  }
}
