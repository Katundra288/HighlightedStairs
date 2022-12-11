// структура, що описує одну сходинку 
struct OneStep {
  int8_t number_of_leds;
  uint16_t bitmask_for_night_mode;  
};

#define OVERALL_NUMBER_OF_LEDS 256  // загальна кількість неонів для проекту  
#define NUMBER_OF_STEPS 16        // загальна кількість сходинок 

// Опис всіх сходинок з опцією "нічна підсвітка"
 //  0b0100100100100100 - кожний третій чіп буде активовано під час нічного режиму 
 // 0b0000000000000000 - нічний режим не активовано
OneStep steps[NUMBER_OF_STEPS] = { 
{ 16, 0b0100100100100100 },   
{ 16, 0b0000000000000000 },   
{ 16, 0b0100100100100100 },   
{ 16, 0b0000000000000000 },   
{ 16, 0b0100100100100100 },   
{ 16, 0b0000000000000000 },   
{ 16, 0b0100100100100100 },   
{ 16, 0b0000000000000000 },   
{ 16, 0b0100100100100100 },   
{ 16, 0b0000000000000000 },   
{ 16, 0b0100100100100100 },   
{ 16, 0b0000000000000000 },   
{ 16, 0b0100100100100100 },   
{ 16, 0b0000000000000000 },   
{ 16, 0b0100100100100100 },   
{ 16, 0b0100100100100100 }    
};

#define DEFAULT_BRIGHTNESS 1 // початково втсановлена яскравість
#define BRIGHTNESS 100  // яскравість, яку можна встановлювати вручну
#define SPEED_SWITCH 300         // швидкість переключення з одної сходинки на іншу менше = швидше 
#define INITIAL_EFFECT RAINBOW   // режим, з якого починається робота 
#define AUTOMATIC_SWITCH_OF_EFFECTS 1      // зміна ефектів 
#define TIMEOUT 15            // виклчення сходинок через 15 секунд, як рух було задокументовано 
#define COLOR_AT_NIGHT mCOLOR(WHITE)  // колір сходинок вночі 
#define BRIGHTNESS_LIGHT_AT_NIGHT 50  // яскравість підсвітки у діапазоні 0 -255
#define MAX_OF_RESISTOR_AT_NIGHT 500   // максимальне значення при якому підсвітка відключається вночі 
#define HIGHLIGHT_OF_RAILS 0      // флажок для підствінки перил
#define HIGHLIGHT_OF_RAILS_LED_AMOUNT 75    // кількість чипів на перилах
#define SWITCH_BUTTON  0      // кнопка для переключення ефектів 

// значення пінів 
#define MOTION_DETECTOR_START 3   
#define MOTION_DETECTOR_END 2     
#define STAIRS_PIN 12     
#define HIGHLIGHT_OF_RAILS_PIN 11   
#define PHOTORES_PIN A0     
#define SWITCH_BUTTON_PIN 6     // пін для кнопки переключення ефектів 
#define ORDER_BGR       // порядок кольорів 
#define DEBTH_OF_A_COLOR 2   // глибина кольору 

#include <microLED.h>
#include <FastLED.h> 

#if (SWITCH_BUTTON == 1) // якщо маємо кнопку для зміни ефектів, то підключаємо додаткову бібліотеку 
#include <GyverSWITCH_BUTTON.h>
#endif

// макроси 
#define FOR_i(from, to) for(int i = (from); i < (to); i++)
#define FOR_j(from, to) for(int j = (from); j < (to); j++)
#define FOR_k(from, to) for(int k = (from); k < (to); k++)
#define TIMER(x) \
  static uint32_t tmr;\
  bool B = millis() - tmr >= (x);\
  if (B) tmr = millis();\
  if (B) 


int HIGHLIGHT_OF_RAILSSegmentLength = HIGHLIGHT_OF_RAILS_LED_AMOUNT / NUMBER_OF_STEPS;   // кількість чипів на сегмент стрічки на перилах

LEDdata stripeeLed[OVERALL_NUMBER_OF_LEDS];  // буфер стрічки сходинок
microLED stripe(stripeeLed, OVERALL_NUMBER_OF_LEDS, STAIRS_PIN);  // об.єкт стрічка

#if (HIGHLIGHT_OF_RAILS == 1)
LEDdata HIGHLIGHT_OF_RAILSLEDs[HIGHLIGHT_OF_RAILS_LED_AMOUNT];  // буфер стрічки перил
microLED HIGHLIGHT_OF_RAILS(HIGHLIGHT_OF_RAILSLEDs, HIGHLIGHT_OF_RAILS_LED_AMOUNT, HIGHLIGHT_OF_RAILS_PIN);  // стрічка
#endif

int speed_of_effects;
int8_t direction_of_effects;
byte actual_brightness = BRIGHTNESS;
enum {COLOR, RAINBOW, FIRE, EFFECTS_AMOUNT} actual_effect = INITIAL_EFFECT;
byte counter_of_effects;
uint32_t timeoutCounter;
bool isturnedOnSystem;
bool isTurnedOffSystem;
int first_step[NUMBER_OF_STEPS];

struct Motion_detector { // структура, що описує датчик руху
  int8_t direction_of_effects;
  int8_t pin;
  bool lastState;
};

Motion_detector startMotion_detector = { 1, MOTION_DETECTOR_START, false};
Motion_detector endMotion_detector = { -1, MOTION_DETECTOR_END, false};

CRGBPalette16 ColorsFire;
int8_t minimalLength_OfStep = steps[0].led_amount;

#if (SWITCH_BUTTON == 1)
GSWITCH_BUTTON SWITCH_BUTTON(SWITCH_BUTTON_PIN);
#endif

void setup() {
  Serial.begin(9600);
  setBrightness(actual_brightness);    
  clear();
  show();  
  
#if (SWITCH_BUTTON == 1)
  SWITCH_BUTTON.setType(HIGH_PULL);
  SWITCH_BUTTON.setDirection(NORM_OPEN);
  SWITCH_BUTTON.setDebounce(100);     // налаштування антимигання
  SWITCH_BUTTON.setTimeout(700);      
  SWITCH_BUTTON.setClickTimeout(600); // таймаут між кліками
#endif

  ColorsFire = CRGBPalette16(
                  getFireColor(0 * 16),
                  getFireColor(1 * 16),
                  getFireColor(2 * 16),
                  getFireColor(3 * 16),
                  getFireColor(4 * 16),
                  getFireColor(5 * 16),
                  getFireColor(6 * 16),
                  getFireColor(7 * 16),
                  getFireColor(8 * 16),
                  getFireColor(9 * 16),
                  getFireColor(10 * 16),
                  getFireColor(11 * 16),
                  getFireColor(12 * 16),
                  getFireColor(13 * 16),
                  getFireColor(14 * 16),
                  getFireColor(15 * 16)
                );
  // вираховування мінімальної ширини для сходинки, щоб ефект вогню працював коректно 
  first_step[0] = 0;
  FOR_i(1, NUMBER_OF_STEPS) {
    if (steps[i].led_amount < minimalLength_OfStep) {
      minimalLength_OfStep = steps[i].led_amount;
    }
    first_step[i] = first_step[i-1] + steps[i-1].led_amount; // рахуємо стартові позиції для кожної сходинки
  }
  delay(100);
  clear();
  show();
}

void cyclicOperations() { // запускання ефектів, якщо стан системи не самий початковий або найостанніший 
  switchButtonOperations();
  MotionDetectorOperations(&startMotion_detector);
  MotionDetectorOperations(&endMotion_detector);
  if (isturnedOnSystem || isTurnedOffSystem) {
    photoResistorOperations();
    handl lightDuringNightOperation();
    delay(50);
  } else {
    effectsOperations();
    TimeoutOperations();
  }
}

void switchButtonOperations() { // обробка кнопки включення/виключення 
#if (SWITCH_BUTTON == 1)
  SWITCH_BUTTON.tick();
  if (SWITCH_BUTTON.isClick() || SWITCH_BUTTON.isHolded())
    actual_effect = ++counter_of_effects % EFFECTS_AMOUNT;
#endif
}

void photoResistorOperations() {// обробка дій фоторезистора   
#if DEFAULT_BRIGHTNESS == 1
  TIMER(3000) {            
    int detector = analogRead(PHOTORES_PIN);
    Serial.print("Photo resistor ");
    Serial.println(detector);
    isTurnedOffSystem = detector > MAX_OF_RESISTOR_AT_NIGHT;
    actual_brightness = isTurnedOffSystem ? 0 : map(detector, 30, 800, 10, 200);
    setBrightness(actual_brightness);
  }
#endif
}

void lightDuringNightOperation() { 
  TIMER(60000) {
   lightDuringNight();
  }
}

void lightDuringNight() { // обробка нічного режиму 
  if (isTurnedOffSystem) {
    Serial.println("System OFF ");
    clear();
    show();
    return;
  }
  TurnOffAnimated(BRIGHTNESS_LIGHT_AT_NIGHT);
  clear();
  FOR_i(0, NUMBER_OF_STEPS) {
    if (steps[i].night_mode_bitmask) {
      steps[i].night_mode_bitmask = (uint16_t) steps[i].night_mode_bitmask >> 1 | steps[i].night_mode_bitmask << 15;
      ApplyBitMaskToSteps(i, COLOR_AT_NIGHT, steps[i].night_mode_bitmask); // накладаємо бітову маску на поточну сходинку 
    }
  }
  TurnOnAnimated(BRIGHTNESS_LIGHT_AT_NIGHT);
}

void TimeoutOperations() { // обробка таймаутів через які вкл/викл система 
  if (millis() - timeoutCounter >= (TIMEOUT * 1000L)) {
    isturnedOnSystem = true;
    if (direction_of_effects == 1) {
      stepFader(0, 1);
    } else {
      stepFader(1, 1);
    }
   lightDuringNight();
  }
}

void MotionDetectorOperations(Motion_detector *det) { // обробка дій, що виконує детектор руху
  if (isTurnedOffSystem) return;

  int stateNext = digitalRead(det->pin);
  if (stateNext && !det->lastState) {
    Serial.print("PIR det ");
    Serial.println(det->pin);
    timeoutCounter = millis(); 
    if (isturnedOnSystem) {
      direction_of_effects = det->direction_of_effects;
      if (AUTOMATIC_SWITCH_OF_EFFECTS) {
        actual_effect = ++counter_of_effects % EFFECTS_AMOUNT;
      }
      stepFader(direction_of_effects == 1 ? 0 : 1,  0);
      isturnedOnSystem = false;
    }
  }
  det->lastState = stateNext;
}

// логіка зміни ефектів під час роботи системи
void effectsOperations() {
  static uint32_t tmr;
  if (millis() - tmr >= speed_of_effects) {
    tmr = millis();
    switch (actual_effect) {
      case COLOR: staticColor(direction_of_effects, 0, NUMBER_OF_STEPS); break;
      case RAINBOW: rainbowstripees(-direction_of_effects, 0, NUMBER_OF_STEPS); break; 
      case FIRE: fireStairs(direction_of_effects, 0, 0); break;
    }
    show();
  }
}
