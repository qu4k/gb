#define NO_STDIO_REDIRECT
#define GL_GLEXT_PROTOTYPES

#include <SDL.h>
#include <SDL_opengl.h>
#include <stdlib.h>

#include "emu/bits.h"

#define WIDTH 800
#define HEIGHT 600

void resize(int width, int height) { glViewport(0, 0, width, height); }

int main(int a, char *b[]) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("SDL_Init Error: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Window *win = SDL_CreateWindow(
      "GB", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT,
      SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  if (win == NULL) {
    printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
    return 1;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

  SDL_GLContext context = SDL_GL_CreateContext(win);
  if (context == NULL) {
    printf("SDL_GL_CreateContext Error: %s\n", SDL_GetError());
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

  float vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f};

  GLuint VBO;
  glGenBuffers(1, &VBO);

  GLuint VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  const char *vertexShaderSource =
      "#version 330 core\n"
      "layout (location = 0) in vec3 aPos;\n"
      "void main() {\n"
      "  gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
      "}\0";

  GLuint vertexShader;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);

  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  GLint success;
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

  if (!success) {
    GLchar infoLog[512];
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    printf("@@ Vertex shader error: %s", infoLog);
    return 1;
  }

  const char *fragmentShaderSource =
      "#version 330 core\n"
      "out vec4 FragColor;\n"
      "void main() {\n"
      "  FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
      "}\n";

  GLuint fragmentShader;
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

  if (!success) {
    GLchar infoLog[512];
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    printf("@@ Fragment shader error: %s", infoLog);
    return 1;
  }

  GLuint shaderProgram;
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  if (!success) {
    GLchar infoLog[512];
    glGetProgramInfoLog(vertexShader, 512, NULL, infoLog);
    printf("@@ Shader program error: %s", infoLog);
    return 1;
  }

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(0);

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
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Update screen
    SDL_GL_SwapWindow(win);
  }

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(win);
  SDL_Quit();

  return 0;
}
