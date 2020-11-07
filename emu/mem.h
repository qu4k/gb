#pragma once

#include "common.h"

const char gbBootRom[0x100];

#define GB_MEM_ROM_SIZE sizeof(gbBootRom)
#define GB_MEM_RAM_SIZE 0x10000

typedef unsigned short addr;

typedef struct {
  byte* rom;
  byte* ram;
} GBMemory;

GBMemory* gbMemNew();
void gbMemFree(GBMemory* mem);

bool gbMemWrite(GBMemory* mem, addr address, byte value);
byte* gbMemRead(GBMemory* mem, addr address);