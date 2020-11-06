#define NO_STDIO_REDIRECT
#include <SDL.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "common.h"
#include "driver/gl/impl.h"
#include "driver/gl/shader.h"
#include "driver/sdl/window.h"
#include "emu/bits.h"

#define WIDTH 800
#define HEIGHT 600

#define CHANNELS 4

#define IMAGE_SIZE (WIDTH) * (HEIGHT) * (CHANNELS)

#define FPS 59.727500569606

#define FEATURE_SIZE 24

void updatePixels(GLubyte *dst, int size) {

  static char v = 0;
  v = (v + 1) % 256;

  if (!dst)
    return;

  char *ptr = (char *)dst;

  // copy 4 bytes at once
  for (int i = 0; i < HEIGHT; ++i) {
    for (int j = 0; j < WIDTH; ++j) {
      *ptr = v;
      *(ptr + 1) = v;
      *(ptr + 2) = v;
      *(ptr + 3) = v;
      ptr += 4;
    }
  }
}

int main(int a, char *b[]) {
  Window *window = gbWindowNew(WIDTH, HEIGHT);
  if (window == NULL) {
    printf("gbWindowNew error: %s\n", gbGetError());
    return 1;
  }

  GLubyte *image = malloc(IMAGE_SIZE);
  memset(image, 255, IMAGE_SIZE);

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

  const char *vertexShaderSource =
      "#version 330 core\n"
      "layout (location = 0) in vec3 aPos;\n"
      "layout (location = 1) in vec3 aColor;\n"
      "layout (location = 2) in vec2 aTexCoord;\n"
      "\n"
      "out vec3 ourColor;\n"
      "out vec2 TexCoord;\n"
      "\n"
      "void main() {\n"
      "  gl_Position = vec4(aPos, 1.0);\n"
      "  ourColor = aColor;\n"
      "  TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
      "}\0";

  const char *fragmentShaderSource =
      "#version 330 core\n"
      "out vec4 FragColor;\n"
      "\n"
      "in vec3 ourColor;\n"
      "in vec2 TexCoord;\n"
      "\n"
      "uniform sampler2D texture1;\n"
      "\n"
      "void main() {\n"
      "  FragColor = texture(texture1, TexCoord);\n"
      "}\n";

  Shader *s = gbShaderNew(vertexShaderSource, fragmentShaderSource, NULL);
  if (s == NULL) {
    printf("gbShaderNew error: %s\n", gbGetError());
    return 1;
  }

  float vertices[] = {
      // positions  // colors         // texture coords
      0.9f,  0.9f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
      0.9f,  -0.9f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
      -0.9f, -0.9f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
      -0.9f, 0.9f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
  };
  unsigned int indices[] = {0, 1, 3, 1, 2, 3};

  GLuint VAO, VBO, EBO, PBOs[2];
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  glGenBuffers(2, PBOs);

  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PBOs[0]);
  glBufferData(GL_PIXEL_UNPACK_BUFFER, IMAGE_SIZE, 0, GL_STREAM_DRAW);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PBOs[1]);
  glBufferData(GL_PIXEL_UNPACK_BUFFER, IMAGE_SIZE, 0, GL_STREAM_DRAW);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  // color attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  // texture coord attribute
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  GLuint texture1;

  glGenTextures(1, &texture1);
  glBindTexture(GL_TEXTURE_2D, texture1);
  // set the texture wrapping parameters
  glTexParameteri(
      GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
      GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WIDTH, HEIGHT, 0, GL_BGRA,
               GL_UNSIGNED_BYTE, image);
  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0);

  gbShaderSetInt(s, "texture1", 0);

  static int index = 0;
  int nextIndex = 0;

  double tickInteval = 1000. / FPS; // frequency in Hz to period in ms
  uint32_t lastUpdateTime = 0;
  uint32_t deltaTime = 0;
  uint32_t accumulator = 0;

  SDL_Event e;
  int quit = 0;
  while (!quit) {
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        quit = 1;
      }
      if (e.type == SDL_WINDOWEVENT) {
        if (e.window.event == SDL_WINDOWEVENT_RESIZED ||
            e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
          glViewport(0, 0, e.window.data1, e.window.data2);
        }
      }
    }

    uint32_t currentTime = SDL_GetTicks();
    deltaTime = currentTime - lastUpdateTime;
    accumulator += deltaTime;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);

    //

    while (accumulator >= tickInteval) {
      index = (index + 1) % 2;
      nextIndex = (index + 1) % 2;

      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PBOs[index]);

      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_BGRA,
                      GL_UNSIGNED_BYTE, 0);

      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, PBOs[nextIndex]);
      glBufferData(GL_PIXEL_UNPACK_BUFFER, IMAGE_SIZE, 0, GL_STREAM_DRAW);
      GLubyte *ptr =
          (GLubyte *)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);

      if (ptr) {
        updatePixels(ptr, IMAGE_SIZE);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
      }

      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

      accumulator -= tickInteval;
    }

    glClear(GL_COLOR_BUFFER_BIT);

    gbShaderUse(s);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Update screen
    SDL_GL_SwapWindow(window->raw);

    lastUpdateTime = currentTime;
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glDeleteBuffers(2, PBOs);

  gbShaderFree(s);
  gbWindowFree(window);
  SDL_Quit();

  return 0;
}
