#include <LiquidCrystal.h>
#include <buttons.h>
#include <EEPROM.h>

/**
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

class MyButton {
public:
  MyButton(int pin);
  int check();

  enum ButtonState {
    BUTTON_ON,
    BUTTON_OFF,
    BUTTON_HOLD
  };
  
private:
  int pin;
};

AttenuationUnit unit1(ATTENUATOR_DATA, ATTENUATOR_CLOCK, ATTENUATOR_LE_1);
AttenuationUnit unit2(ATTENUATOR_DATA, ATTENUATOR_CLOCK, ATTENUATOR_LE_2);
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Button btn1;
Button btn2;
Button btn3;
Button btnUp;
Button btnDown;

int attenuationDistributionMode = 0; // 0 => fully use unit1 and then unit2, 
                                     // 1 => attempt to distribute evenly

void setTotalAttenuation(int attenuationLevelInDb) {
  int firstUnitAttenuation;
  if (attenuationDistributionMode == 0) {
    firstUnitAttenuation = min(attenuationLevelInDb, 31);
  } else {
    firstUnitAttenuation = attenuationLevelInDb / 2;
  }
  
  int secondUnitAttenuation = attenuationLevelInDb - firstUnitAttenuation;

  unit1.setAttenuation(firstUnitAttenuation);
  unit2.setAttenuation(secondUnitAttenuation);
}

int attenuator[3];
int currentActiveAttenuator = -1;

int getCurrentAttenuation() {
  return currentActiveAttenuator == -1 ? 0 : attenuator[currentActiveAttenuator];
}

char lcdLine[17];
void updateDisplay() {
  lcd.setCursor(0, 0);
  sprintf(lcdLine, "   ATTEN %2ddB", getCurrentAttenuation());
  lcd.write(lcdLine);

  memset(lcdLine, ' ', 16);

  char buffer[7];
  lcdLine[0] = 0;
  for (int i = 0; i < 3; i++) {
    if (currentActiveAttenuator == i) {
      sprintf(buffer, "%c%ddB ", 126, attenuator[i]);
    } else {
      sprintf(buffer, "%ddB ", attenuator[i]);
    }
    strcat(lcdLine, buffer);
  }
  strcat(lcdLine, " ");
  lcd.setCursor(0, 1);
  lcd.write(lcdLine);
}

void updateAttenuatorsWithCurrentSettings() {
  setTotalAttenuation(getCurrentAttenuation());
  updateDisplay();
  saveSettingsToEeprom();
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

void saveSettingsToEeprom() {
  EEPROM.write(0, 1);
  for (int i = 0; i < 3; i++) {
    EEPROM.write(i+1, attenuator[i]);
  }
  EEPROM.write(4, currentActiveAttenuator+1);
}

void loadSettingsFromEeprom() {
  byte eepromInitialized = EEPROM.read(0);
  if (eepromInitialized > 0) {
    for (int i = 0; i < 3; i++) {
      attenuator[i] = EEPROM.read(i+1);
    }
    currentActiveAttenuator = EEPROM.read(4) - 1;
  } else {
    attenuator[0] = 10;
    attenuator[1] = 20;
    attenuator[2] = 30;
    currentActiveAttenuator = -1;
    saveSettingsToEeprom();
  }
}

void setup() {
  loadSettingsFromEeprom();

  pinMode(ATTENUATOR_CLOCK, OUTPUT);
  pinMode(ATTENUATOR_DATA, OUTPUT);
  pinMode(ATTENUATOR_LE_1, OUTPUT);
  pinMode(ATTENUATOR_LE_2, OUTPUT);

  btn1.assign(BUTTON_1_PIN);
  btn2.assign(BUTTON_2_PIN);
  btn3.assign(BUTTON_3_PIN);
  btnUp.assign(BUTTON_UP_PIN);
  btnDown.assign(BUTTON_DOWN_PIN);

  btn1.turnOnPullUp();
  btn2.turnOnPullUp();
  btn3.turnOnPullUp();
  btnUp.turnOnPullUp();
  btnDown.turnOnPullUp();

  btn1.setMode(OneShot);
  btn2.setMode(OneShot);
  btn3.setMode(OneShot);

  btnUp.setMode(OneShotTimer);
  btnUp.setTimer(1800);
  btnUp.setRefresh(200);

  btnDown.setMode(OneShotTimer);
  btnDown.setTimer(1800);
  btnDown.setRefresh(200);

//  btn1.begin();
//  btn2.begin();
//  btn3.begin();
//  btnUp.begin();
//  btnDown.begin();
  
  lcd.begin(16, 2);
  lcd.noCursor();
  lcd.write("helloes");
  updateAttenuatorsWithCurrentSettings();
}

void loop() {
  if (btn1.check() == ON) {
    onButton1Press();
  }
  if (btn2.check() == ON) {
    onButton2Press();
  }
  if (btn3.check() == ON) {
    onButton3Press();
  }

  switch (btnUp.check()) {
    case ON:
      onButtonUpPress();
    break;
    case Hold:
      onButtonUpPress();
    break;
    default:
    break;
  }

  switch (btnDown.check()) {
    case ON:
      onButtonDownPress();
    break;
    case Hold:
      onButtonDownPress();
    break;
    default:
    break;
  }

  
//  if (btn1.toggled() && btn1.read() == Button::PRESSED) {
//    onButton1Press();
//  }
//  
//  if (btn2.toggled() && btn2.read() == Button::PRESSED) {
//    onButton2Press();
//  }
//  
//  if (btn3.toggled() && btn3.read() == Button::PRESSED) {
//    onButton3Press();
//  }
//  
//  if (btnUp.toggled() && btnUp.read() == Button::PRESSED) {
//    onButtonUpPress();
//  }
//  
//  if (btnDown.toggled() && btnDown.read() == Button::PRESSED) {
//    onButtonDownPress();
//  }
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

