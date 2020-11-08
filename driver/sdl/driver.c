#include "driver.h"

int gbDriverInit(void) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    gbSetError("<<SDL_Init>> %s\n", SDL_GetError());
    return 1;
  }
  return 0;
}

void gbDriverQuit(void) { SDL_Quit(); }

uint32_t gbDriverGetTicks(void) { return SDL_GetTicks(); }

GBDriver *gbDriverNew(int width, int height) {
  GBDriver *driver = malloc(sizeof(GBDriver));

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

  SDL_GL_SetSwapInterval(1);

  driver->raw = win;
  driver->context = context;
  return driver;
}

void gbDriverFree(GBDriver *driver) {
  SDL_GL_DeleteContext(driver->context);
  SDL_DestroyWindow(driver->raw);
  free(driver);
}

void gbDriverDraw(GBDriver *driver) { SDL_GL_SwapWindow(driver->raw); }

static bool (*DriverCallback)(const SDL_Event *) = NULL;

void gbDriverSetEventCallback(bool (*Callback)(const SDL_Event *)) {
  DriverCallback = Callback;
}

int gbDriverPollEvent(GBDriverEvent *event) {
  SDL_Event e;
  bool found = false;
  while (SDL_PollEvent(&e) != 0) {
    found = false;
    if (e.type == SDL_QUIT) {
      event->type = GB_DRIVER_QUIT;
      found = true;
    }
    if (e.type == SDL_WINDOWEVENT) {
      if (e.window.event == SDL_WINDOWEVENT_RESIZED ||
          e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        event->type = GB_DRIVER_RESIZE;
        event->width = e.window.data1;
        event->height = e.window.data2;
        found = true;
      }
      if (e.window.event == SDL_WINDOWEVENT_CLOSE) {
        event->type = GB_DRIVER_QUIT;
        found = true;
      }
    }
    if (DriverCallback != NULL)
      DriverCallback(&e);
    if (found)
      return true;
  }
  return 0;
}