#pragma once

#include <stdbool.h>

#define byte unsigned char
#define small byte

byte setBit(byte n, small pos);

byte clearBit(byte n, small pos);

bool testBit(byte n, small pos);

byte getVal(byte n, small pos);

char* fmtByte(byte n);