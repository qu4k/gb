#pragma once

#include <stdbool.h>

typedef unsigned char byte;
typedef unsigned short word;
typedef byte small;

byte setBit(byte n, small pos);

byte clearBit(byte n, small pos);

bool testBit(byte n, small pos);

byte getVal(byte n, small pos);

char *fmtByte(byte n);