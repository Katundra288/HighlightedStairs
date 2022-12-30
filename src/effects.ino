#define SATURATION_MAX 255
#define SATURATION_MIN 220
#define BRIGHTNESS_MAX 255
#define BRIGHTNESS_MIN 150
#define GAP 45
#define STEP_FOR_FIRE 90
#define BEG_FIRE_COLOR 2

// вмикач для ефектів
void effederStep(bool boo, bool onof) {
  // boo - напрямок 0(назад) 1(вперед)
  // onof - стан 1 (вкл) 0 (вимк)

  byte counter = 0;
  byte c = onof | (boo << 1);

// логіка зміни ефектів під час роботи системи
  while (1) {
          EACH_MILI(SPEED) {
            counter++;
            switch (effect_at_the_moment) {
              case COLOR:
                switch (c) {
                  case 0: usualStaticColor(1, 0, counter); break;
                  case 1: usualStaticColor(1, counter, NUM_OF_STEPS); break;
                  case 2: usualStaticColor(-1, NUM_OF_STEPS - counter, NUM_OF_STEPS); break;
                  case 3: usualStaticColor(-1, 0, NUM_OF_STEPS - counter); break;
                }
                break;
              case RAINBOW:
                switch (c) {
                  case 0: effectRainbow(-1, NUM_OF_STEPS - counter, NUM_OF_STEPS); break;
                  case 1: effectRainbow(-1, 0, NUM_OF_STEPS - counter); break;
                  case 2: effectRainbow(1, NUM_OF_STEPS - counter, NUM_OF_STEPS); break;
                  case 3: effectRainbow(1, 0, NUM_OF_STEPS - counter); break;
                }
            break;
// логіка зміни ефектів під час роботи системи
          case FIRE:
            if (onof) {
              int newBright = brightness_at_moment;

              while(1){
              EACH_MILI(50){

              newBright -=5;
              if(newBright < 0) break;
              strip.setBrightness(newBright);
              stairsFire(0,0,0);
              strip.show();
              }
              }
              strip.clear();
              strip.setBrightness(brightness_at_moment);
              strip.show();
              } else {
              int newBright = 0;
              strip.setBrightness(0);
              while (1) {
                EACH_MILI(50) {
                  newBright += 5;
                  if (newBright > brightness_at_moment) break;
                  strip.setBrightness(newBright);
                  stairsFire(0, 0, 0);
                  strip.show();
                }
                strip.setBrightness(brightness_at_moment);
              }
            }
            return;
            break;
        }
      strip.show();
           if (counter == NUM_OF_STEPS) break;
    }
  }
   if (onof == 1) {
       strip.clear();
       strip.show();
     }
   }

CRGB color_of_fire_getter(int i) {
  return CHSV(
           BEG_FIRE_COLOR + map(i, 0, 255, 0, GAP), // H
           constrain(map(i, 0, 255, SATURATION_MAX, SATURATION_MIN), 0, 255),// S
           constrain(map(i, 0, 255, BRIGHTNESS_MIN, BRIGHTNESS_MAX), 0, 255) // V
                  );
}




}