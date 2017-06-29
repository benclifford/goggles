#include <Arduino.h>
#include <stdint.h>
#include <EEPROM.h>
#include "prng.h"

/* EEPROM usage:
 *   byte 0: last used mode
 *   byte 1: PRNG seed
 *   byte 2: PRNG seed
 */

uint16_t prng_register;

#define PRNG_FEEDBACK 0xb400u

void initRandom() {
  uint16_t lsb = EEPROM.read(1);
  uint16_t usb = EEPROM.read(2);
  prng_register = (usb << 8) | lsb;
  
  // If the register is 0, we get a cycle of
  // 0s out, rather than a pseudo-random sequence.
  // If so, replace it with something non-zero.
  // This might end up shortening the sequence of
  // possible outputs by interacting with the
  // pseudorandom sequence used for re-seeding in
  // a particular way? But this will only happen
  // if the pseudorandom sequence contains 16
  // zeroes in a row (so as to make the above
  // write zeroes into the EEPROM) or when the
  // eeprom is freshly initialised to 0 from elsewhere.
  if(prng_register == 0) prng_register=1;

  // now the generator is initialised safely, we can
  // generate a new seed to use next time.

  nextRNGBit();

  EEPROM.write(1, prng_register & 0xFF);
  EEPROM.write(2, (prng_register & 0xFF00) >> 8);

}

byte nextRNGBit() {
  // shift out an output bit
  byte out = prng_register & 1;
  prng_register >>= 1;

  // apply feedback
  if(out == 1) {
    prng_register ^= PRNG_FEEDBACK; // from appropriate polynomial
  }
  
  return out;
}

byte nextRNGByte() {
  return nextRNGBits(8);
}

byte nextRNGBits(byte c) {
  byte out = 0;
  for(byte b = 0; b < c ; b++) {
    out = (out << 1) | nextRNGBit();
  }
  return out;
}

byte get3BitRandom() {
  return nextRNGBits(3);
}

byte get3BitRandomLessThan(byte m) {
  byte out;
   do {
     out = get3BitRandom();
   } while(out >= m);
}

