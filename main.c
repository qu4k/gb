#include "driver/gl/impl.h"

#define NO_STDIO_REDIRECT
#include <SDL.h>
#include <stdlib.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

#include "common.h"
#include "driver/gl/shader.h"
#include "driver/sdl/driver.h"
#include "emu/mem.h"

#include "driver/imgui/memory_view.h"

#define WIDTH 800
#define HEIGHT 600

#define CHANNELS 4

#define IMAGE_SIZE (WIDTH) * (HEIGHT) * (CHANNELS)

#define FPS 59.727500569606

int main(int a, char *b[]) {
  GBMemory *mem = gbMemNew();

  GBMemoryEditor *mem_edit = meditMemoryEditorNew();

  gbDriverInit();
  GBDriver *driver = gbDriverNew(WIDTH, HEIGHT);
  GBDriver *debugger = gbDriverNew(WIDTH, HEIGHT);

  if (driver == NULL || debugger == NULL) {
    printf("gbDriverNew error: %s\n", gbGetError());
    return 1;
  }

  if (gl3wInit()) {
    fprintf(stderr, "Failed to initialize OpenGL loader!\n");
    return 1;
  }

  printf("=== GB by @qu4k ===\n");

  SDL_version linked;
  SDL_GetVersion(&linked);

  printf("Driver: SDL v%d.%d.%d\n", linked.major, linked.minor, linked.patch);
  printf("OpenGL: v%s <<%s>>\n", glGetString(GL_VERSION),
         glGetString(GL_VENDOR));
  printf("GLSL: v%s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

  printf("=== =========== ===\n");

  glViewport(0, 0, WIDTH, HEIGHT);

  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

  igCreateContext(NULL);
  ImGuiIO *io = igGetIO();
  io->ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io->IniFilename = NULL;

  ImGui_ImplSDL2_InitForOpenGL(debugger->raw, debugger->context);
  ImGui_ImplOpenGL3_Init("#version 330");
  gbDriverSetEventCallback(ImGui_ImplSDL2_ProcessEvent);

  igStyleColorsDark(NULL);

  double tickInteval = 1000. / FPS; // frequency in Hz to period in ms
  uint32_t lastUpdateTime = 0;
  uint32_t deltaTime = 0;
  uint32_t accumulator = 0;

  ImVec4 clearColor;
  clearColor.x = 0.45f;
  clearColor.y = 0.55f;
  clearColor.z = 0.60f;
  clearColor.w = 1.00f;

  GBDriverEvent e;
  int quit = 0;
  while (!quit) {
    while (gbDriverPollEvent(&e) != 0) {
      if (e.type == GB_DRIVER_QUIT)
        quit = true;
      if (e.type == GB_DRIVER_RESIZE)
        glViewport(0, 0, e.width, e.height);
    }

    uint32_t currentTime = gbDriverGetTicks();
    deltaTime = currentTime - lastUpdateTime;
    accumulator += deltaTime;

    while (accumulator >= tickInteval) {
      // update

      accumulator -= tickInteval;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(debugger->raw);
    igNewFrame();

    static float f = 0.0f;
    static int counter = 0;

    igShowDemoWindow(1);

    meditDrawWindow(mem_edit, "Memory Editor", mem->rom, GB_MEM_ROM_SIZE,
                    0x0000);

    igRender();

    SDL_GL_MakeCurrent(debugger->raw, debugger->context);

    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());

    // Update screen
    gbDriverDraw(debugger);

    SDL_GL_MakeCurrent(driver->raw, driver->context);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());

    // Update screen
    gbDriverDraw(driver);

    lastUpdateTime = currentTime;
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  igDestroyContext(NULL);

  gbDriverFree(driver);

  gbDriverQuit();

  gbMemFree(mem);

  return 0;
}
