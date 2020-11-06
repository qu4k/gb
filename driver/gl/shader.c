#include "shader.h"

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

Shader *gbShaderNew(const char *vertexShaderSource,
                    const char *fragmentShaderSource,
                    const char *geometryShaderSource) {
  Shader *s = malloc(sizeof(Shader));
  s->id = glCreateProgram();

  GLuint vertexShader;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);

  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  GLint success;
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

  if (!success) {
    GLchar infoLog[ERR_MAX_STRLEN];
    glGetShaderInfoLog(vertexShader, ERR_MAX_STRLEN, NULL, infoLog);
    gbSetError("<<vertex>> %s", infoLog);
    return NULL;
  }

  glAttachShader(s->id, vertexShader);
  glDeleteShader(vertexShader);

  GLuint fragmentShader;
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

  if (!success) {
    GLchar infoLog[ERR_MAX_STRLEN];
    glGetShaderInfoLog(fragmentShader, ERR_MAX_STRLEN, NULL, infoLog);
    gbSetError("<<fragment>> %s", infoLog);
    return NULL;
  }
  glAttachShader(s->id, fragmentShader);
  glDeleteShader(fragmentShader);

  glLinkProgram(s->id);

  glGetProgramiv(s->id, GL_LINK_STATUS, &success);

  if (!success) {
    GLchar infoLog[ERR_MAX_STRLEN];
    glGetProgramInfoLog(vertexShader, ERR_MAX_STRLEN, NULL, infoLog);
    gbSetError("<<program>> %s", infoLog);
    return NULL;
  }

  return s;
}

void gbShaderFree(Shader *shader) {
  glDeleteProgram(shader->id);
  free(shader);
}

void gbShaderUse(Shader *shader) { glUseProgram(shader->id); }

void gbShaderSetInt(Shader *shader, const char *name, int value) {
  glUniform1i(glGetUniformLocation(shader->id, name), value);
}
void gbShaderSetFloat(Shader *shader, const char *name, float value) {
  glUniform1f(glGetUniformLocation(shader->id, name), value);
}
