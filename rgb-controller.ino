#include <EEPROM.h>
#include <math.h>

const int PIN_LED = 17;
const int PIN_BTN = 3;
const int PIN_RED = 5;
const int PIN_GREEN = 6;
const int PIN_BLUE = 9;

const int LOOP_INTERVAL = 10;

int EEPROM_CURRENT_ADDRESS = 0;

const int EEPROM_ON_ADDRESS = EEPROM_CURRENT_ADDRESS++;
const int EEPROM_MODE_ADDRESS = EEPROM_CURRENT_ADDRESS++;
const int EEPROM_HUE_ADDRESS = EEPROM_CURRENT_ADDRESS++;
const int EEPROM_VALUE_ADDRESS = EEPROM_CURRENT_ADDRESS++;

const float BRIGHTNESS_STEP_PER_S = 0.33;
const float HUE_STEP_PER_S = 0.025;
const float OFF_FADE_OUT_S = 1;

// Variables will change:
int buttonState;            // the current reading from the input pin
int lastButtonState = LOW;  // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 40;    // the debounce time; increase if the output flickers


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

// Time since off (for fading out)
unsigned long tOff = 0;

// Brightness breath effect, going up or doin
bool brightnessGoingUp = true;

void setup() {
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);

  // Turn off internal LED
  pinMode(LED_BUILTIN_TX,INPUT);
  pinMode(LED_BUILTIN_RX,INPUT);

  Serial.begin(9600);
  Serial.println("Initialize Serial Monitor");

  on = EEPROM.read(EEPROM_ON_ADDRESS);
  mode = EEPROM.read(EEPROM_MODE_ADDRESS);
  hue = EEPROM.read(EEPROM_HUE_ADDRESS);
  value = EEPROM.read(EEPROM_VALUE_ADDRESS);

   if (value < 0.1) value = 1;
}

void loop() {
  // Click handling
  int reading = digitalRead(PIN_BTN);

  // Serial.print("Reading: ");
  // Serial.print(reading);
  // Serial.print(", buttonState: ");
  // Serial.println(buttonState);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {

      buttonState = reading;

      if (buttonState == LOW) {
        tDown = millis();
        pressHandled = false;
      } else {
        if (!pressHandled) {
          onShortClick();
          pressHandled = true;
        }
      }
    }
  }
  // Serial.print("Press handled: ");
  // Serial.println(pressHandled);
  if (pressHandled != true && (millis() - tDown) > 1000) {
    pressHandled = true;
    onLongClick();
  }

  lastButtonState = reading;

  // Light handling
  if (on) {
    unsigned long msSinceLastLoop = millis() - tLastLoop;

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

    // Serial.print("Hue: ");
    // Serial.print(hue);
    // Serial.print(", saturation: ");
    // Serial.print(saturation);
    // Serial.print(", value: ");
    // Serial.println(value);

    setLEDColor(hsv2rgb(hue, saturation, value, rgbColor));
  } else {

    setLEDColor(new float[3] {0.0, 0.0, 0.0});
  }

  tLastLoop = millis();

  // Loop interval
  delay(LOOP_INTERVAL);
}

void onShortClick() {
  Serial.println("Short click");
  if (!on) {
    toggleOn();
  } else {
    cycleMode();
  }
}

void onLongClick() {
  Serial.println("Long click");
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
  if (on) {
    if (value < 0.1) value = 1;
  } else {
    tOff = millis();
  }

  // int ms = OFF_FADE_OUT_S * 1000;
  // float tempHue = hue;
  // for(int i = 0; i < ms; i += LOOP_INTERVAL) {
  //   Serial.print("hue=");
  //   Serial.println(hue);
  //   float percent = ((float)i / ms);
  //   setLEDColor(hsv2rgb(((int)(hue * 100)) / 100.0, saturation, value * (on ? percent : 1 - percent), rgbColor));
  //   delay(LOOP_INTERVAL);
  // }
  // tLastLoop = millis();
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

  analogWrite(PIN_RED, 255 - (int)((rgb[0]) * 255 * value));
  analogWrite(PIN_GREEN, 255 - (int)((rgb[1]) * 255 * value));
  analogWrite(PIN_BLUE, 255 - (int)((rgb[2]) * 255 * value));

  // Serial.print("Set LED color: ");
  // Serial.print(rgb[0]);
  // Serial.print(", ");
  // Serial.print(rgb[1]);
  // Serial.print(", ");
  // Serial.println(rgb[2]);
}
