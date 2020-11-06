#pragma once

#define ERR_MAX_STRLEN 512

typedef struct {
  int error; /* This is a numeric value corresponding to the current error */
  char str[ERR_MAX_STRLEN];
} GBError;

GBError *gbGetErrorBuffer(void);
const char *gbGetError(void);
int gbSetError(const char *fmt, ...);
void gbClearError(void);