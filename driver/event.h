#pragma once

typedef enum {
  GB_DRIVER_QUIT,
  GB_DRIVER_RESIZE,
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
