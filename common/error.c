#include "error.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

GBError *gbGetErrorBuffer(void) {
  static GBError error;
  return &error;
}

const char *gbGetError(void) {
  const GBError *error = gbGetErrorBuffer();
  return error->error ? error->str : "";
}

int gbSetError(const char *fmt, ...) {
  /* Ignore call if invalid format pointer was passed */
  if (fmt != NULL) {
    va_list ap;
    GBError *error = gbGetErrorBuffer();

    error->error = 1; /* mark error as valid */

    va_start(ap, fmt);
    vsnprintf(error->str, ERR_MAX_STRLEN, fmt, ap);
    va_end(ap);
  }

  return -1;
}

void gbClearError(void) { gbGetErrorBuffer()->error = 0; }