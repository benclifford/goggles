#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

#define NUMLEDS 32
#define PIN 1

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMLEDS, PIN, NEO_GRB + NEO_KHZ800);

#define NUMMODES 4
int mode=0;


void setup() {
  // put your setup code here, to run once:
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
//  #if defined (__AVR_ATtiny85__)
//    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
//  #endif

  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off

  mode = (EEPROM.read(0) + 1) % NUMMODES;
  EEPROM.write(0, mode);
}

void loop() {
  switch(mode) {
    case 0: loop_1(); break;
    case 1: loop_2(); break;
    case 2: loop_3(); break;
    case 3: loop_4(); break;
  }
}

void loop_1() {

  uint32_t colour = strip.Color(0,0,15);
  for(byte pix=0; pix<NUMLEDS; pix++) {
    strip.setPixelColor(pix, colour);  
  }

  strip.show();

  delay(1000);
  
  for(byte pix=0; pix<NUMLEDS; pix++) {
    uint32_t colour = strip.Color(255,0,0);
    strip.setPixelColor(pix, colour);
    strip.show();
    delay(1000);
  }
}

void loop_2() {
  uint32_t black = strip.Color(0,0,0);
  uint32_t white = strip.Color(255,255,255);
  for(byte pix=0; pix<NUMLEDS; pix++) {
    strip.setPixelColor(pix, black);  
  }
  strip.show();
  delay(3000);
  for(byte pix=0; pix<NUMLEDS; pix++) {
    strip.setPixelColor(pix, white);  
  }
  strip.show();
  delay(500);
}

void loop_3() {
  uint32_t black = strip.Color(0,0,0);
  
  for(byte start=0; start<16; start++) {
    for(byte pix=0; pix<NUMLEDS; pix++) {
      strip.setPixelColor(pix, black);  
    }
    for(byte off=0; off<7; off++) {
      // TODO: I think pow uses quite a lot of ROM because it is doing floats
      // and actually we need something much less accurate.
      byte amt = intpow(2, off+1);
      uint32_t green = strip.Color(0,amt,0);
      strip.setPixelColor((start + off) % 16, green);
      strip.setPixelColor(NUMLEDS - ((start + off) % 16 + 1), green);
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


int intpow(int e, int n) {
  int v = 1;
  for(;n>0;n--) {
    v *= e;
  }
  return v; 
}

