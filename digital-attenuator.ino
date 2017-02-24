#include <LiquidCrystal.h>
#include <eBtn.h>


/**
 * Adrian Scripca <benishor@gmail.com>, YO6SSW
 * Using PE4306 serial command code from Jeff Tranter <tranter@pobox.com>
 *  
 * Pins
 * 
 * 
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * 10K resistor:
 *    ends to +5V and ground
 *    wiper to LCD VO pin (pin 3)
 * 
 * 
 * D6 -> attenuator clock (both units)
 * D7 -> attenuator data (both units)
 * D8 -> attenuator latch enable (unit 1)
 * D9 -> attenuator latch enable (unit 2)
 * 
 * 
 * A0 -> atten 1
 * A1 -> atten 2
 * A2 -> atten 3
 * A3 -> atten up
 * A4 -> atten down
 * 
 *  
 */

#define ATTENUATOR_CLOCK 6
#define ATTENUATOR_DATA 7
#define ATTENUATOR_LE_1 8
#define ATTENUATOR_LE_2 9

#define BUTTON_1_PIN A0
#define BUTTON_2_PIN A1
#define BUTTON_3_PIN A2
#define BUTTON_UP_PIN A3
#define BUTTON_DOWN_PIN A4

class AttenuationUnit {
public:
  AttenuationUnit(int data, int clock, int latchEnable)
    : dataPin(data), clockPin(clock), latchEnablePin(latchEnable) {
        pinMode(dataPin, OUTPUT);
        pinMode(clockPin, OUTPUT);
        pinMode(latchEnablePin, OUTPUT);
      }

  void setAttenuation(int attenuationLevelInDb);

private:
  int dataPin;
  int clockPin;
  int latchEnablePin;
};

AttenuationUnit unit1(ATTENUATOR_DATA, ATTENUATOR_CLOCK, ATTENUATOR_LE_1);
AttenuationUnit unit2(ATTENUATOR_DATA, ATTENUATOR_CLOCK, ATTENUATOR_LE_2);
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
eBtn btn1 = eBtn(BUTTON_1_PIN);
eBtn btn2 = eBtn(BUTTON_2_PIN);
eBtn btn3 = eBtn(BUTTON_3_PIN);
eBtn btnUp = eBtn(BUTTON_UP_PIN);
eBtn btnDown = eBtn(BUTTON_DOWN_PIN);


void setTotalAttenuation(int attenuationLevelInDb) {
  int firstUnitAttenuation = attenuationLevelInDb / 2;
  int secondUnitAttenuation = attenuationLevelInDb - firstUnitAttenuation;

  unit1.setAttenuation(firstUnitAttenuation);
  unit2.setAttenuation(secondUnitAttenuation);
}

void buttonIsr() {
  btn1.handle();
  btn2.handle();
  btn3.handle();
  btnUp.handle();
  btnDown.handle();
}

int attenuator[3];
int currentActiveAttenuator = -1;

char lcdLine[17];
void updateDisplay() {
  lcd.setCursor(0, 0);
  sprintf(lcdLine, "   ATTEN: %2ddB  ");
  
  lcd.write(lcdLine);
  
  lcd.setCursor(0, 1);
  for (int i = 0; i < 3; i++) {
    if (currentActiveAttenuator == i) {
      sprintf(lcdLine[i*5], " [%2d]", attenuator[i]);
    } else {
      sprintf(lcdLine[i*5], "  %2d ", attenuator[i]);
    }
  }
  lcd.write(lcdLine);
}

void updateAttenuatorsWithCurrentSettings() {
  setTotalAttenuation(currentActiveAttenuator == -1 ? 0 : attenuator[currentActiveAttenuator]);
  updateDisplay();
}

void onButton1Press() {
  currentActiveAttenuator = currentActiveAttenuator == 0 ? -1 : 0;
  updateAttenuatorsWithCurrentSettings();
}

void onButton2Press() {
  currentActiveAttenuator = currentActiveAttenuator == 1 ? -1 : 1;
  updateAttenuatorsWithCurrentSettings();
}

void onButton3Press() {
  currentActiveAttenuator = currentActiveAttenuator == 2 ? -1 : 2;
  updateAttenuatorsWithCurrentSettings();
}

void onButtonUpPress() {
  if (currentActiveAttenuator != -1) {
    if (attenuator[currentActiveAttenuator] < 62) {
      attenuator[currentActiveAttenuator]++;
      updateAttenuatorsWithCurrentSettings();
    }
  }
}

void onButtonDownPress() {
  if (currentActiveAttenuator != -1) {
    if (attenuator[currentActiveAttenuator] > 1) {
      attenuator[currentActiveAttenuator]--;
      updateAttenuatorsWithCurrentSettings();
    }
  }
}


void setup() {

  attenuator[0] = 3;
  attenuator[1] = 6;
  attenuator[2] = 10;
  currentActiveAttenuator = -1;
  
  lcd.begin(16, 2);
  lcd.noCursor();

  btn1.on("press", onButton1Press);
  btn2.on("press", onButton2Press);
  btn3.on("press", onButton3Press);
  btnUp.on("press", onButtonUpPress);
  btnDown.on("press", onButtonDownPress);
  
  attachInterrupt(BUTTON_1_PIN, buttonIsr, CHANGE);
  attachInterrupt(BUTTON_2_PIN, buttonIsr, CHANGE);
  attachInterrupt(BUTTON_3_PIN, buttonIsr, CHANGE);
  attachInterrupt(BUTTON_UP_PIN, buttonIsr, CHANGE);
  attachInterrupt(BUTTON_DOWN_PIN, buttonIsr, CHANGE);
}

void loop() {
}

void AttenuationUnit::setAttenuation(int attenuationLevelInDb) {
  // Check for value range of input.
  if (attenuationLevelInDb < 0 || attenuationLevelInDb > 31)
    return;

  // Initial state
  digitalWrite(latchEnablePin, LOW);
  digitalWrite(clockPin, LOW);
  
  int level;
  for (int bit = 5; bit >= 0; bit--) {
    if (bit == 0) {
      level = 0; // LSB is always zero
    } else {
      level = ((attenuationLevelInDb << 1) >> bit) & 0x01; // Level is value of bit
    }

    digitalWrite(dataPin, level); // Write data value
    digitalWrite(clockPin, HIGH); // Toggle clock high and then low
    digitalWrite(clockPin, LOW);
  }

  digitalWrite(latchEnablePin, HIGH); // Toggle LE high to enable latch
  digitalWrite(latchEnablePin, LOW);  // and then low again to hold it.
}


