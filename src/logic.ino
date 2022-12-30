#include <microLED.h>
#include "libraries/FastLED-3.2.9/FastLED.h"
// pins
#define BEGIN_STAIRS_PIN 3 // a start of stairs 
#define FINISH_STAIRS_PIN 2 // end of stairs 
#define STRIPE_PIN 12   // pin for stripe 
#define PHOTO_RES_PIN A0 // pin fot photores
#define NUM_OF_STEPS 8     // загальна кількість схожинок 
#define NUM_OF_CHIPS_PER_STAIR 17    // number of chips per one stair 
#define BRIGHNESS_DEFAULT 0     // brightness default
#define BRIGHTNESS_CUSTOMIZABLE 40  // brightness that can be customized 
#define SPEED 500
#define BEG_EFFECT RAINBOW    // the effect from which the effects changing starts 
#define EFFECTS_CHANGE 0      // automatic change of the effects 
#define TIMEOUT 15            // if last detector hadn`t worked - use this timeout 
#define SEUENCE_BGR       // the possible sequence of the colors 
#define DEPTH_OF_A_COLOR 2   // the depth of the color
#define NUM_OF_DIODS NUM_OF_STEPS * NUM_OF_CHIPS_PER_STAIR // number of needed diods 

#define EACH_MILI(mm) \
  static uint32_t y;\
  bool boo = millis() - y >= (mm);\
  if (boo) y = millis();\
  if (boo)
#define FOR_i(from, to) for(int i = (from); i < (to); i++)
#define FOR_j(from, to) for(int j = (from); j < (to); j++)
#define FOR_k(from, to) for(int k = (from); k < (to); k++)

enum {S_IDLE, S_WORK} state_of_a_system = S_IDLE;
enum {COLOR, RAINBOW, FIRE} effect_at_the_moment = BEG_EFFECT;
#define NUM_OF_EFFECTS 3
byte count_for_effects;
LEDdata buf_leds[NUM_OF_DIODS];  // bufer for stripes 
microLED strip(buf_leds, STRIPE_PIN, NUM_OF_CHIPS_PER_STAIR, NUM_OF_STEPS, ZIGZAG, LEFT_BOTTOM, DIR_RIGHT);  // matrix
int speed_of_effects;
int8_t direction_effects;
byte brightness_at_moment = BRIGHTNESS_CUSTOMIZABLE;
int counter = 0;
CRGBPalette16 paletFire;

void setup() {
  Serial.begin(9600);
  strip.setBrightness(brightness_at_moment);    
  strip.clear();
  strip.show();
  paletFire = CRGBPalette16(
                  color_of_fire_getter(0 * 16),
                  color_of_fire_getter(1 * 16),
                  color_of_fire_getter(2 * 16),
                  color_of_fire_getter(3 * 16),
                  color_of_fire_getter(4 * 16),
                  color_of_fire_getter(5 * 16),
                  color_of_fire_getter(6 * 16),
                  color_of_fire_getter(7 * 16),
                  color_of_fire_getter(8 * 16),
                  color_of_fire_getter(9 * 16),
                  color_of_fire_getter(10 * 16),
                  color_of_fire_getter(11 * 16),
                  color_of_fire_getter(12 * 16),
                  color_of_fire_getter(13 * 16),
                  color_of_fire_getter(14 * 16),
                  color_of_fire_getter(15 * 16)
                );
  delay(100);
  strip.clear();
  strip.show();
}

void cycle() {
  brightness_getter();
  analyze_sensors();
  changing_of_effects();
}

void brightness_getter() {
#if (BRIGHNESS_DEFAULT == 1)
  if (state_of_a_system == S_IDLE) {  
    EACH_MILI(3000) {          
      Serial.println(analogRead(PHOTO_RES_PIN));
      brightness_at_moment = map(analogRead(PHOTO_RES_PIN), 30, 800, 10, 200);
      strip.setBrightness(brightness_at_moment);
    }
  }
#endif
}

// getting info from sensors 
void analyze_sensors() {
  static bool boo1 = false;
  static bool boo2 = false;
  static uint32_t count_time;

  if (state_of_a_system == S_WORK && millis() - count_time >= (TIMEOUT * 1000L)) {
    state_of_a_system = S_IDLE;
    int newBrightness = brightness_at_moment;
    while (1) {
      EACH_MILI(50) {
        newBrightness -= 5;
        if (newBrightness < 0) break;
        strip.setBrightness(newBrightness);
        strip.show();
      }
    }
    strip.clear();
    strip.setBrightness(brightness_at_moment);
    strip.show();
  }

  EACH_MILI(50) {
    // end sensor 
    if (digitalRead(FINISH_STAIRS_PIN)) {
      if (!boo2) {
        boo2 = true;
        count_time = millis();
        if (state_of_a_system == S_IDLE) {
          direction_effects = -1;
          if (EFFECTS_CHANGE) {
            if (++count_for_effects >= NUM_OF_EFFECTS){ 
              count_for_effects = 0;}
            effect_at_the_moment = count_for_effects;
          }
        }
        switch (state_of_a_system) {
          case S_IDLE: effederStep(1, 0); state_of_a_system = S_WORK; break;
          case S_WORK:
            if (direction_effects == 1) {
              effederStep(0, 1); 
              state_of_a_system = S_IDLE;
              strip.clear(); 
              strip.show(); 
              return;
            } break;
        }
      }
    } else {
      if (boo2) {boo2 = false;}
    }

    // beginning sensor
    if (digitalRead(BEGIN_STAIRS_PIN)) {
      if (!boo1) {
        boo1 = true;
        count_time = millis();
        if (state_of_a_system == S_IDLE) {
          direction_effects = 1;
          if (EFFECTS_CHANGE) {
            if (++count_for_effects >= NUM_OF_EFFECTS){count_for_effects = 0;}
            effect_at_the_moment = count_for_effects;
          }
        }
        switch (state_of_a_system) {
          case S_IDLE: effederStep(0, 0); state_of_a_system = S_WORK; break;
          case S_WORK:
            if (direction_effects == -1) {
              effederStep(1, 1); 
              state_of_a_system = S_IDLE;
              strip.clear(); 
              strip.show(); 
              return;
            } break;
        }
      }
    } else {
      if (boo1) boo1 = false;
    }
  }
}

// changing the effects during the process of working 
void changing_of_effects() {
  if (state_of_a_system == S_WORK) {
    static uint32_t tt;
    if (millis() - tt >= speed_of_effects) {
      tt = millis();
      switch (effect_at_the_moment) {
        case COLOR: usualStaticColor(direction_effects, 0, NUM_OF_STEPS); break;
        case RAINBOW: effectRainbow(-direction_effects, 0, NUM_OF_STEPS); break; 
        case FIRE: stairsFire(direction_effects, 0, 0); break;
      }
      strip.show();
    }
  }
}

