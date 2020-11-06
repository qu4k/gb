#pragma once

#include <SDL.h>

typedef struct {
  SDL_Window *raw;
  SDL_GLContext *context;
} Window;

Window *gbWindowNew(int width, int height);
void gbWindowFree(Window *window);