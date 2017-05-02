#include <stdio.h>

#include "sdl_window.h"
#include "gameoflife.h"

int main(int argc, char ** argv)
{
  size_t width = 1024;
  size_t height = 1024;

  Window window(512, 512, width, height);

  GameOfLife game(window.get_texture(), width, height, "gameoflife.cl");

  while (window.is_running())
  {
    game.compute_next_frame();

    window.render();

    window.poll_events();
  }

  return 0;
}
