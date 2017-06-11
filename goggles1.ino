#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

#define NUMLEDS 32
#define PIN 1

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMLEDS, PIN, NEO_GRB + NEO_KHZ800);

#define NUMMODES 5
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
    case 4: loop_5(); break;
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

void loop_5() {

  int phase=0;
  uint32_t black = strip.Color(0,0,0); 
  uint32_t intercol = strip.Color(32,0,0);

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
    strip.setPixelColor((8+rot)%16, intercol);
    strip.setPixelColor(31 - (8+rot)%16, intercol);

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
      strip.setPixelColor((pixel + rot) % 16, thisamber);
      strip.setPixelColor(31 - (pixel + rot) % 16, thisamber);
    }
    strip.setPixelColor((15 + rot)%16, intercol);
    strip.setPixelColor(31-(15 + rot)%16, intercol);

    // render and advance
    phase = (phase + 1) % 1024;
    strip.show();
    delay(10);
  }
}

int intpow(int e, int n) {
  int v = 1;
  for(;n>0;n--) {
    v *= e;
  }
  return v; 
}

