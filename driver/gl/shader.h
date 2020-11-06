#pragma once

#include "impl.h"

typedef struct {
  GLuint id;
} Shader;

Shader *gbShaderNew(const char *vertexShaderSource,
                    const char *fragmentShaderSource,
                    const char *geometryShaderSource);
void gbShaderFree(Shader *shader);

void gbShaderUse(Shader *shader);

void gbShaderSetInt(Shader *shader, const char *name, int value);
void gbShaderSetFloat(Shader *shader, const char *name, float value);