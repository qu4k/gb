#include "bits.h"

#include <string.h>

byte setBit(byte n, small pos) {
  n |= (1 << pos);
  return n;
}

byte clearBit(byte n, small pos) {
  n &= ~(1 << pos);
  return n;
}

bool testBit(byte n, small pos) { return ((n) & (1 << (pos))) > 0; }

byte getVal(byte n, small pos) { return ((n >> pos) & 1); }

char* fmtByte(byte n) {
  static char b[9];
  b[0] = '\0';

  int z;
  for (z = 128; z > 0; z >>= 1) {
    strcat(b, ((n & z) == z) ? "1" : "0");
  }

  return b;
}