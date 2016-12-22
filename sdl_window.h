#ifndef SDL_WINDOW_H_
#define SDL_WINDOW_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

class Window
{
public:

  Window(ssize_t _wwidth, ssize_t _wheight, ssize_t _swidth, ssize_t _sheight);

  ~Window();

  void render();

  void poll_events();

  GLuint get_texture() {return texture;}

  bool is_running() {return running;}

private:

  SDL_Window * sdl_window;

  SDL_GLContext gl_context;

  GLuint texture;

  size_t wwidth, wheight, sim_width, sim_height;

  volatile bool running;
};

#endif /* end of include guard: SDL_WINDOW_H_ */
