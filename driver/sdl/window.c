#include "window.h"

#include "common.h"

Window *gbWindowNew(int width, int height) {
  Window *window = malloc(sizeof(Window));

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    gbSetError("<<SDL_Init>> %s\n", SDL_GetError());
    return NULL;
  }

  SDL_Window *win = SDL_CreateWindow(
      "GB", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
      SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  if (win == NULL) {
    gbSetError("<<SDL_CreateWindow>> %s\n", SDL_GetError());
    return NULL;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

  SDL_GLContext context = SDL_GL_CreateContext(win);
  if (context == NULL) {
    gbSetError("<<SDL_GL_CreateContext>> %s\n", SDL_GetError());
    return NULL;
  }

  window->raw = win;
  window->context = context;
  return window;
}

void gbWindowFree(Window *window) {
  SDL_GL_DeleteContext(window->context);
  SDL_DestroyWindow(window->raw);
  free(window);
}