#include <EEPROM.h>
#include <math.h>

const int PIN_LED = 17;
const int PIN_BTN = 3;
const int PIN_RED;
const int PIN_GREEN;
const int PIN_BLUE;

const int LOOP_INTERVAL = 10;

int EEPROM_CURRENT_ADDRESS = 0;

const int EEPROM_ON_ADDRESS = EEPROM_CURRENT_ADDRESS++;
const int EEPROM_MODE_ADDRESS = EEPROM_CURRENT_ADDRESS++;
const int EEPROM_HUE_ADDRESS = EEPROM_CURRENT_ADDRESS++;
const int EEPROM_VALUE_ADDRESS = EEPROM_CURRENT_ADDRESS++;

const float BRIGHTNESS_STEP_PER_S = 0.33;
const float HUE_STEP_PER_S = 0.2;

// State
bool on;
//  mode 0: static light
//  mode 1: hue cycle
//  mode 2: brightness cycle
int mode;

// LED state
float hue;
float saturation = 1;
float value;

// Button state
int btnPrevious = 1;
unsigned long tDown = 0;
boolean pressHandled = true;

// Reusable rgb color
const float rgbColor[3];

// Time of last loop
unsigned long tLastLoop = 0;

// Brightness breath effect, going up or doin
bool brightnessGoingUp = true;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);

  Serial.begin(9600);
  Serial.println("Initialize Serial Monitor");

  on = EEPROM.read(EEPROM_ON_ADDRESS);
  mode = EEPROM.read(EEPROM_MODE_ADDRESS);
  hue = EEPROM.read(EEPROM_HUE_ADDRESS);
  value = EEPROM.read(EEPROM_VALUE_ADDRESS);
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

  // Testing until I get actual LED
  analogWrite(PIN_LED, on ? (int)((1.0 - value) * 255) : HIGH);

  if (on) {
    unsigned long msSinceLastLoop = millis() - tLastLoop;
    // Serial.println(msSinceLastLoop);

    switch (mode) {
      case 0:
        // Static color, no need to change
        break;
      case 1:
        // Cycle trough hue, wrapping around at 1.0
        hue = fmod(hue + (HUE_STEP_PER_S / 1000) * msSinceLastLoop, 1.0);
        break;
      case 2:
        // Brightness breath effect
        float newValue = value + (BRIGHTNESS_STEP_PER_S / 1000) * msSinceLastLoop * (brightnessGoingUp ? 1 : -1);
        if (newValue > 1) brightnessGoingUp = false;
        if (newValue < 0) brightnessGoingUp = true;
        value = max(0, min(newValue, 1.0));
        break;
    }

    Serial.print("Hue: ");
    Serial.print(hue);
    Serial.print(", saturation: ");
    Serial.print(saturation);
    Serial.print(", value: ");
    Serial.println(value);

    setLEDColor(hsv2rgb(hue, saturation, value, rgbColor));
  }

  tLastLoop = millis();

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
  EEPROM.write(EEPROM_MODE_ADDRESS, mode);
}

void toggleOn() {
  on = !on;
  EEPROM.write(EEPROM_ON_ADDRESS, on);
}

// Color stuff (https://gist.github.com/postspectacular/2a4a8db092011c6743a7)
// HSV->RGB conversion based on GLSL version
// expects hsv channels defined in 0.0 .. 1.0 interval
float fract(float x) { return x - int(x); }
float mix(float a, float b, float t) { return a + (b - a) * t; }

float* hsv2rgb(float h, float s, float b, float* rgb) {
  rgb[0] = b * mix(1.0, constrain(abs(fract(h + 1.0) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  rgb[1] = b * mix(1.0, constrain(abs(fract(h + 0.6666666) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  rgb[2] = b * mix(1.0, constrain(abs(fract(h + 0.3333333) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  return rgb;
}

void setLEDColor(float *rgb) {
  analogWrite(PIN_RED, (int)((1.0 - rgb[0]) * 255));
  analogWrite(PIN_GREEN, (int)((1.0 - rgb[1]) * 255));
  analogWrite(PIN_BLUE, (int)((1.0 - rgb[2]) * 255));  

  // Serial.print("Set LED color: ");
  // Serial.print(rgb[0]);
  // Serial.print(", ");
  // Serial.print(rgb[1]);
  // Serial.print(", ");
  // Serial.println(rgb[2]);
}
