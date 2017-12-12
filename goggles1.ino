#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include "prng.h"

/* EEPROM usage:
 *   byte 0: last used mode
 *   byte 1: PRNG seed
 *   byte 2: PRNG seed
 */

#define NUMLEDS 32
#define PIN 1

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMLEDS, PIN, NEO_GRB + NEO_KHZ800);

byte mode=0;

extern uint16_t prng_register;

#define black strip.Color(0,0,0)

void setup() {
  // put your setup code here, to run once:
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
//  #if defined (__AVR_ATtiny85__)
//    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
//  #endif

  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off

  initRandom();

  // this will be rolled over by loop() once it passes
  // the last mode
  mode = (EEPROM.read(0));
  EEPROM.write(0, mode+1);

}

void loop() {
  switch(mode) {
    case 0: loop_slow_primary_fill(); break;
    case 1: loop_white_flash(); break;
    case 2: loop_green_swirl(); break;
    case 3: loop_pulse_lr_colours(); break;
    case 4: loop_police(); break;
    case 5: loop_prng(); break;
    case 6: loop_campfire(); break;
    case 7: loop_rainbow(); break;
    case 8: loop_amber_lr_pulse(); break;
    case 9: loop_rainbow_on_off(); break;
    case 10: loop_colour_swizz(); break;
    default: mode=0;
             EEPROM.write(0, 1);
             break;
  }
}

byte pickPrimary(byte oldPrimary, byte otherOldPrimary) {
  byte newPrimary=oldPrimary;
  while(newPrimary == oldPrimary || newPrimary == otherOldPrimary) {
    // using this random number generator is
    // pretty expensive - about 692 bytes.
    // newPrimary = random(0,6);
    newPrimary = get3BitRandomLessThan(6);
  }
  return newPrimary;
}

uint32_t primaryToColour(byte primary) {
  switch(primary) {
    case 0: return strip.Color(  0,  0,255);
    case 1: return strip.Color(  0,255,  0);
    case 2: return strip.Color(  0,128,64);
    case 3: return strip.Color(255,  0,  0);
    case 4: return strip.Color(192,  0,192);
    case 5: return strip.Color(255,128,  0);
  }
}

void loop_slow_primary_fill() {

  uint32_t colour = strip.Color(1,1,1);
  setAllPixels(colour);

  strip.show();

  byte leftPrimary=-1;
  byte rightPrimary=-1;

  while(1 == 1) {
    leftPrimary=pickPrimary(leftPrimary, rightPrimary);
    uint32_t leftColour = primaryToColour(leftPrimary);
    for(byte pix=0; pix<16; pix++) {
      strip.setPixelColor(pix, leftColour);
      strip.show();
      delay(1000);
    }
    
    rightPrimary=pickPrimary(rightPrimary, leftPrimary);
    uint32_t rightColour = primaryToColour(rightPrimary);
    for(byte pix=16; pix<32; pix++) {
      strip.setPixelColor(pix, rightColour);
      strip.show();
      delay(1000);
    }
    
  }
}

void loop_white_flash() {
  uint32_t white = strip.Color(255,255,255);
  setAllPixels(black);
  strip.show();
  delay(3000);
  setAllPixels(white);
  strip.show();
  delay(500);
}


uint32_t green_swirl_colour(byte start, byte pix) {
  byte off = (pix - start) & 0x0F;
  if(off < 7) { 
    byte amt = intpow(2, off+1);
    uint32_t green = strip.Color(0,amt,0);
    return green;
  } else {
    return black;
  }
}

void loop_green_swirl() {
  for(byte start=0; start<16; start++) {
    for(byte off=0; off<16; off++) {
      setPixelMirror(off, green_swirl_colour(start, off));
    }
    strip.show();
    delay(200);
  }
}

void loop_pulse_lr_colours() {

  for(byte level=0;level < 8*8; level++) {
    byte up = twopow(level);
    byte down = twopow(63-level);
    uint32_t left = strip.Color(0,up,down);
    uint32_t right = strip.Color(up,down,0);
    for(byte pix=0; pix<32; pix++) {
      strip.setPixelColor(pix, pix < 16 ? left : right);
    }
    strip.show();
    delay(25);
  }
}

#define POLICE_BLUE strip.Color(0,0,255)
#define POLICE_AMBER strip.Color(255,64,0)
#define POLICE_INTERCOL strip.Color(0,0,32)

byte rotate_offset(uint16_t phase) {
  // needs to be a multiple of 6 so that
  // rotations do not happen out of phase
  // with the amber dots
  return phase/6;
}

uint32_t lbluex(uint16_t phase) {
  if((phase / 8) % 2 == 0) {
    return POLICE_BLUE;
  } else {
    return black;
  }
}

uint32_t rbluex(uint16_t phase) {
  if((phase / 8) % 2 == 0) {
    return black;
  } else {
    return POLICE_BLUE;
  }
}

uint32_t unrotated_police_colour(uint32_t (*bluecol)(uint16_t), uint16_t phase, byte pixel) {
  if(pixel >=0 && pixel < 8) return bluecol(phase); //pixel = 0 .. 7
  if(pixel == 8) return POLICE_INTERCOL;

  if(pixel > 8 && pixel <= 15) {
    if((5 - phase % 6) == (pixel - 9)) {
      return POLICE_AMBER;
    } else {
      return black;
    }
  }

  // if this if is up by the other POLICE_INTERCOL,
  // the code uses 4 bytes extra (at time of writing)
  if(pixel == 15) return POLICE_INTERCOL;
}

byte rotate_pixel(uint32_t phase, byte pixel) {
  return (pixel - rotate_offset(phase)) & 0x0F;
}

uint32_t x_police_colour(uint32_t (*bluecol)(uint16_t), uint16_t phase, byte pixel) {
  return unrotated_police_colour(bluecol, phase, rotate_pixel(phase, pixel));
}

uint32_t left_police_colour(uint16_t phase, byte pixel) {
  return x_police_colour(lbluex, phase, pixel);
}

uint32_t right_police_colour(uint16_t phase, byte pixel) {
  return x_police_colour(rbluex, phase, 31-pixel);
}

uint32_t police_colour(uint16_t phase, byte pixel) {
  if(pixel < 16) return left_police_colour(phase, pixel);
  if(pixel >= 16) return right_police_colour(phase, pixel);
}

void loop_police() {

  uint16_t phase=0;
  while(1 == 1){

    for(byte pixel = 0; pixel < 32; pixel++) {
      strip.setPixelColor(pixel, police_colour(phase, pixel));
    }

    // render and advance
    // this needs to be a common multiple of all relevant phases
    // otherwise we get jumps at rollover.
    phase = (phase + 1) % (256 * 6);
    strip.show();
    delay(40);
  }
}

// show the PRNG shift register on one side - this should
// sort-of rotate but with feedback happening.
// and on the other side, set each LED in turn on or off
// based on the result of generating one random bit
// every time period. This one bit generation is also
// what will drive the first side to rotate.

#define PRNG_RED strip.Color(255,0,0)
#define PRNG_GREEN strip.Color(0,255,0)

void loop_prng() {
  
  setAllPixels(black);
  strip.show();

  while(1==1) {
    for(byte pix=0; pix<16; pix++) {
      byte b = nextRNGBit();
      if(b == 1) {
        strip.setPixelColor(pix, PRNG_RED);
      } else {
        strip.setPixelColor(pix, black);
      }
      for(byte lsfr_pix=0; lsfr_pix<16; lsfr_pix++) {
        byte lsfr_b = (prng_register >> lsfr_pix) & 1;
        if(lsfr_b == 1) {
          strip.setPixelColor(lsfr_pix + 16, PRNG_GREEN);
        } else {
          strip.setPixelColor(lsfr_pix + 16, black);
        }
      }
      strip.show();
      delay(100);
    }    
  }
}

void loop_campfire() {
  int pixs[32];
  for(byte b=0; b<32; b++) {
    pixs[b] = 7;
  }
  while(1==1) {
    // TODO: adjust
    byte adj_pix = nextRNGBits(5);
    byte updown = nextRNGBit() ;
    if(updown == 1 && pixs[adj_pix] < 15) {
      pixs[adj_pix]++;
    } else if(pixs[adj_pix]>0) {
      pixs[adj_pix]--;
    }
    // now update the LEDs
    for(byte pix = 0; pix < 32; pix++) {
      byte heat = pixs[pix];
      uint32_t colour;
      if(heat == 0) { // off
        colour = strip.Color(0,0,0);
      } else if(heat < 8) { // red 
        colour = strip.Color(intpow(2,heat - 1), 0, 0);
      } else if(heat < 16) { // yellow
        colour = strip.Color(255,intpow(2, heat - 9), 0);
      } // ... otherwise something's awry
      strip.setPixelColor(pix, colour);
    }
    strip.show();
    delay(50);
  }
}

void loop_rainbow() {
  byte col = 0;
  byte pix = 0;
  while(1==1) {
    setPixelMirror(pix, primaryToColour(col));
    col = (col + 1) % 6;
    pix = (pix + 1) % 16;
    strip.show();
    delay(166); // will give us one swizzle per second with 6 colours
  }
}

void loop_amber_lr_pulse() {
  for(byte offset=0; offset < 32; offset +=16) {
    for(byte level=8;level < 8*8; level++) {
      byte up = twopow(level);
      uint32_t left = strip.Color(up,up/2,0);
      for(byte pix=0; pix<16; pix++) {
        strip.setPixelColor(pix+offset, left);
      }
      strip.show();
      delay(10);
    }
    delay(200);
    for(byte level=8*8-1;level > 8; level--) {
      byte up = twopow(level);
      uint32_t left = strip.Color(up,up/2,0);
      for(byte pix=0; pix<16; pix++) {
        strip.setPixelColor(pix+offset, left);
      }
      strip.show();
      delay(15);
    }
    for(byte pix=0; pix<16; pix++) {
      strip.setPixelColor(pix+offset, black);
    }
    strip.show();
    delay(200);
  }
}

void loop_rainbow_on_off() {

  byte col = 0;
  while(1==1) {
    for(byte offset=0; offset<32; offset+=16) {
      byte rot = nextRNGBits(4);
      byte dir = nextRNGBit();
      for(byte pix=0;pix<16;pix++) {
        byte phys_pix = (pix + rot) % 16;
        if(dir == 0) phys_pix = 15-phys_pix;
        strip.setPixelColor(phys_pix+offset, primaryToColour(col));
        col = (col + 1) % 6;
        strip.show();
        delay(166); // will give us one swizzle per second with 6 colours
      }
      for(byte pix=0;pix<16;pix++) {
      
        byte phys_pix = (pix + rot) % 16;
        if(dir == 0) phys_pix = 15-phys_pix;
        strip.setPixelColor(phys_pix+offset, black);
        strip.show();
        delay(166);
      }
    }
  }
}


uint32_t colour_of_swizz(byte offset, byte destpix) {
  byte pix = (destpix - offset) & 0x0F;
  if(pix >= 0 && pix < 3) return primaryToColour(1);
  if(pix >= 4 && pix < 7) return primaryToColour(0);
  if(pix >= 8 && pix < 11) return primaryToColour(3);
  if(pix >=12 && pix < 15) return primaryToColour(4);
  return black;
}

void loop_colour_swizz()  {
  for(byte offset=0; offset<16; offset++) {
    for(byte pix = 0; pix < 16; pix++) {
      setPixelMirror(pix,colour_of_swizz(offset, pix));
    }
    strip.show();
    delay(100);
  }
}

byte intpow(byte e, byte n) {
  byte v = 1;
  for(;n>0;n--) {
    v *= e;
  }
  return v; 
}

// n is a number between 0 .. 63
// so 6 bits
// use highest 3 bits as exponent
// and lowest 3 bits as linear filler
uint8_t twopow(uint8_t n) {
  uint8_t v = 1;
  uint8_t ex = (n & 0x38) >> 3;
  uint8_t fill = n & 0x07;
  v <<= ex;
  if(ex >= 3) {
    fill <<= ex - 3;
  } else { // ex < 3 
    fill >>= -(ex - 3);
  }
  v = v | fill;
  return v;
}


void setPixelMirror(byte pix, uint32_t colour) {
  strip.setPixelColor(pix, colour);
  strip.setPixelColor(NUMLEDS - pix - 1, colour);
}

void setAllPixels(uint32_t colour) {
  for(byte pix=0; pix<NUMLEDS; pix++) {
    strip.setPixelColor(pix, colour);  
  }
}

