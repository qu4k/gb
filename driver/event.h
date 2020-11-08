#pragma once

typedef enum {
  GB_DRIVER_QUIT,
  GB_DRIVER_RESIZE,
  GB_DRIVER_NATIVE,
} GBDriverEventType;

typedef struct {
  GBDriverEventType type;
  union {
    struct {
      int width;
      int height;
    };
    void *_nothing;
  };
} GBDriverEvent;
