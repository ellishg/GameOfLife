#include "sdl_window.h"

const GLint vertices[] = {-1, 1, 1, 1, 1, -1, -1, -1};
const GLint tex_coords[] = {0, 0, 1, 0, 1, 1, 0, 1};

Window::Window(ssize_t _wwidth, ssize_t _wheight, ssize_t _swidth, ssize_t _sheight) : wwidth(_wwidth), wheight(_wheight), sim_width(_swidth), sim_height(_sheight), running(true)
{
  if (SDL_Init(SDL_INIT_VIDEO))
  {
    fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
    exit(1);
  }

  sdl_window = SDL_CreateWindow("Conway's Game of Life", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, wwidth, wheight, SDL_WINDOW_OPENGL);

  if (!sdl_window)
  {
    fprintf(stderr, "Unable to create window: %s\n", SDL_GetError());
    exit(1);
  }

  gl_context = SDL_GL_CreateContext(sdl_window);

  if (gl_context == 0)
  {
    fprintf(stderr, "Unable to create context: %s\n", SDL_GetError());
    exit(1);
  }

  glEnable(GL_TEXTURE_2D);

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sim_width, sim_height, 0, GL_RGBA, GL_FLOAT, NULL);

  GLenum err = glGetError();
  if (err != GL_NO_ERROR)
  {
    fprintf(stderr, "Window setup failed: %d\n", err);
    exit(1);
  }
}

Window::~Window()
{
  glDeleteTextures(1, &texture);
  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(sdl_window);
}

void Window::render()
{
  glBindTexture(GL_TEXTURE_2D, texture);

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(2, GL_INT, 0, vertices);

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexCoordPointer(2, GL_INT, 0, tex_coords);

  glDrawArrays(GL_QUADS, 0, 4);

  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  SDL_GL_SwapWindow(sdl_window);
}

void Window::poll_events()
{
  SDL_Event event;

  while (SDL_PollEvent(&event))
  {
    if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
    {
      running = false;
    }
  }
}
