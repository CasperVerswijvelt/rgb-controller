#include <EEPROM.h>

int PIN_LED = 17;
int PIN_BTN = 3;
int LOOP_INTERVAL = 10;
int EEPROM_ON_ADDRESS = 0;
int EEPROM_MODE_ADDRESS = 1;

// State
bool on;
//  mode 0: static light
//  mode 1: hue cycle
//  mode 2: brightness cycle
int mode ;

// Button state
int btnPrevious = 1;
long int tDown = 0;
boolean pressHandled = true;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);

  Serial.begin(9600);
  Serial.println("Initialize Serial Monitor");

  on = EEPROM.read(EEPROM_ON_ADDRESS);
  mode = EEPROM.read(EEPROM_MODE_ADDRESS);

  Serial.print("Read state from EEPROM: ");
  Serial.println(on);
  Serial.print("Read mode from EEPROM: ");
  Serial.println(mode);
}

void loop() {
  // Read button and compare to previous
  int btnNew = digitalRead(PIN_BTN);
  if (btnPrevious != btnNew) {

    if (btnNew == LOW) {
      tDown = millis();
      pressHandled = false;
    } else {
      if (!pressHandled) {
        Serial.println("Short click");
        onShortClick();
        pressHandled = true;
      }
    }
  }
  if (!pressHandled && (millis() - tDown) > 1000) {
    Serial.println("Long click");
    onLongClick();
    pressHandled = true;
  }
  btnPrevious = btnNew;

  digitalWrite(PIN_LED, on ? LOW : HIGH);

  // Loop interval
  delay(LOOP_INTERVAL);
}

void onShortClick() {
  if (!on) {
    toggleOn();
  } else {
    cycleMode();
  }
}

void onLongClick() {
  toggleOn();
}

void cycleMode() {
  mode ++;
  mode = mode % 3;
  saveStateToEEPROM();
}

void toggleOn() {
  on = !on;
  saveStateToEEPROM();
}

void saveStateToEEPROM() {
  Serial.print("On: ");
  Serial.println(on);
  Serial.print("Mode: ");
  Serial.println(mode);

  EEPROM.write(EEPROM_O _ADDRESS, on);
  EEPROM.write(EEPROM_MODE_ADDRESS, mode);
}
