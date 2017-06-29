#include <Arduino.h>
#include <stdint.h>

void initRandom();

byte nextRNGBit();

byte nextRNGByte();

byte nextRNGBits(byte c);

byte get3BitRandom();

byte get3BitRandomLessThan(byte m);

