#pragma once

#include "common.h"
#include "event.h"
#include <SDL.h>

typedef struct {
  SDL_Window *raw;
  SDL_GLContext *context;
} GBDriver;

int gbDriverInit(void);
void gbDriverQuit(void);

uint32_t gbDriverGetTicks(void);

GBDriver *gbDriverNew(int width, int height);
void gbDriverFree(GBDriver *driver);

void gbDriverDraw(GBDriver *driver);

void gbDriverSetEventCallback(bool (*Callback)(const SDL_Event *));

int gbDriverPollEvent(GBDriverEvent *event);