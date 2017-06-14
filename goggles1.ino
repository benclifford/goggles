#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

/* EEPROM usage:
 *   byte 0: last used mode
 *   byte 1: PRNG seed
 *   byte 2: PRNG seed
 */

#define NUMLEDS 32
#define PIN 1

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMLEDS, PIN, NEO_GRB + NEO_KHZ800);

#define NUMMODES 7
int mode=0;

uint16_t prng_register;

void setup() {
  // put your setup code here, to run once:
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
//  #if defined (__AVR_ATtiny85__)
//    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
//  #endif

  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off

  initRandom();

  mode = (EEPROM.read(0) + 1) % NUMMODES;
  EEPROM.write(0, mode);

}

void loop() {
  switch(mode) {
    case 0: loop_1(); break;
    case 1: loop_2(); break;
    case 2: loop_3(); break;
    case 3: loop_4(); break;
    case 4: loop_5(); break;
    case 5: loop_6(); break;
    case 6: loop_7(); break;
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

void loop_1() {

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

void loop_2() {
  uint32_t black = strip.Color(0,0,0);
  uint32_t white = strip.Color(255,255,255);
  setAllPixels(black);
  strip.show();
  delay(3000);
  setAllPixels(white);
  strip.show();
  delay(500);
}

void loop_3() {
  uint32_t black = strip.Color(0,0,0);
  
  for(byte start=0; start<16; start++) {
    setAllPixels(black);

    for(byte off=0; off<7; off++) {
      // TODO: I think pow uses quite a lot of ROM because it is doing floats
      // and actually we need something much less accurate.
      byte amt = intpow(2, off+1);
      uint32_t green = strip.Color(0,amt,0);
      setPixelMirror((start + off) % 16, green);
    }
    strip.show();
    delay(200);
  }
}

void loop_4() {

  for(int level=0;level < 8; level++) {
    byte up = intpow(2, level);
    byte down = intpow(2, 7-level);
    uint32_t left = strip.Color(0,up,down);
    uint32_t right = strip.Color(up,down,0);
    for(byte pix=0; pix<16; pix++) {
      strip.setPixelColor(pix, left);
    }
    for(byte pix=16; pix<32; pix++) {
      strip.setPixelColor(pix, right);
    }
    strip.show();
    delay(200);
  }
}

void loop_5() {

  int phase=0;
  uint32_t black = strip.Color(0,0,0); 
  uint32_t intercol = strip.Color(0,0,32);

  while(1 == 1){
    byte rot = phase / 32;
    uint32_t blue = strip.Color(0,0,255); // TODO modulate
    uint32_t lblue;
    uint32_t rblue;
    if((phase / 32) % 2 == 0) { lblue = blue; rblue = black; }
       else {lblue = black; rblue = blue; }
    
    for(byte pixel = 0; pixel < 8; pixel++) {
      strip.setPixelColor((pixel + rot) % 16, lblue);
      strip.setPixelColor(31 - (pixel + rot) % 16, rblue);
    }
    setPixelMirror((8+rot) % 16, intercol);

    int amber_intensity;
    /*
    if((phase / 8) % 2 == 0) {
        amber_intensity = 1;
    } else {
      amber_intensity = 0;
    }
    */
    amber_intensity = 1;
    uint32_t amber = strip.Color(255 * amber_intensity, 64 * amber_intensity,0); // TODO modulate
    
    for(byte pixel = 9; pixel < 15; pixel++) {
      uint32_t thisamber;
      if((phase/4) % 6 == (pixel-9)) {
        thisamber = amber; 
      } else {
        thisamber = black;
      }
      setPixelMirror((pixel + rot) % 16, thisamber);
    }
    setPixelMirror((15 + rot) % 16, intercol);


    // render and advance
    phase = (phase + 1) % 1024;
    strip.show();
    delay(10);
  }
}

// show the PRNG shift register on one side - this should
// sort-of rotate but with feedback happening.
// and on the other side, set each LED in turn on or off
// based on the result of generating one random bit
// every time period. This one bit generation is also
// what will drive the first side to rotate.
void loop_6() {

  uint32_t black = strip.Color(0,0,0);
  uint32_t red = strip.Color(255,0,0);
  uint32_t green = strip.Color(0,255,0);
  
  setAllPixels(black);
  strip.show();

  while(1==1) {
    for(byte pix=0; pix<16; pix++) {
      byte b = nextRNGBit();
      if(b == 1) {
        strip.setPixelColor(pix, red);
      } else {
        strip.setPixelColor(pix, black);
      }
      for(byte lsfr_pix=0; lsfr_pix<16; lsfr_pix++) {
        byte lsfr_b = (prng_register >> lsfr_pix) & 1;
        if(lsfr_b == 1) {
          strip.setPixelColor(lsfr_pix + 16, green);
        } else {
          strip.setPixelColor(lsfr_pix + 16, black);
        }
      }
      strip.show();
      delay(100);
    }    
  }
}

void loop_7() {
  int pixs[32];
  for(byte b=0; b<32; b++) {
    pixs[b] = 7;
  }
  while(1==1) {
    // TODO: adjust
    byte adj_pix = nextRNGByte() % 32;
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

int intpow(int e, int n) {
  int v = 1;
  for(;n>0;n--) {
    v *= e;
  }
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
  byte out = 0;
  for(byte b = 0; b <8 ; b++) {
    out = (out << 1) | nextRNGBit();
  }
  return out;
}

byte get3BitRandom() {
  return nextRNGBit() | (nextRNGBit() << 1) | (nextRNGBit() << 2);
}

byte get3BitRandomLessThan(byte m) {
  byte out;
   do {
     out = get3BitRandom();
   } while(out >= m);
}

