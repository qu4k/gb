#define NO_STDIO_REDIRECT
#include <SDL.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "common/common.h"
#include "driver/gl/impl.h"
#include "driver/gl/shader.h"
#include "emu/bits.h"

#define WIDTH 800
#define HEIGHT 600

void resize(int width, int height) { glViewport(0, 0, width, height); }

int main(int a, char *b[]) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("SDL_Init error: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Window *win = SDL_CreateWindow(
      "GB", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT,
      SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  if (win == NULL) {
    printf("SDL_CreateWindow error: %s\n", SDL_GetError());
    return 1;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

  SDL_GLContext context = SDL_GL_CreateContext(win);
  if (context == NULL) {
    printf("SDL_GL_CreateContext error: %s\n", SDL_GetError());
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

  resize(WIDTH, HEIGHT);


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

  Shader *s = shaderNew(vertexShaderSource, fragmentShaderSource, NULL);
  if (s == NULL) {
    printf("shaderNew error: %s\n", gbGetError());
    return 1;
  }

  float vertices[] = {
      // positions  // colors         // texture coords
      1.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
      1.0f,  -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
      -1.0f, 1.0f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
  };
  unsigned int indices[] = {0, 1, 3, 1, 2, 3};

  GLuint VAO, VBO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

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

  // load image, create texture and generate mipmaps
  int width, height, nrChannels;
  stbi_set_flip_vertically_on_load(true);
  unsigned char *data =
      stbi_load("brickwall.jpg", &width, &height, &nrChannels, 0);
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    printf("Error: failed to load texture\n");
    return 1;
  }
  stbi_image_free(data);

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
          resize(e.window.data1, e.window.data2);
        }
      }
    }
    glClear(GL_COLOR_BUFFER_BIT);

    shaderUse(s);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Update screen
    SDL_GL_SwapWindow(win);
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  shaderFree(s);

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(win);
  SDL_Quit();

  return 0;
}
