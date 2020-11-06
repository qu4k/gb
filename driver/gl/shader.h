#pragma once

#include "impl.h"

typedef struct {
  GLuint id;
} Shader;

Shader *shaderNew(const char *vertexShaderSource,
                  const char *fragmentShaderSource,
                  const char *geometryShaderSource);
void shaderFree(Shader *shader);

void shaderUse(Shader *shader);

void shaderSetInt(Shader *shader, const char *name, int value);
void shaderSetFloat(Shader *shader, const char *name, float value);