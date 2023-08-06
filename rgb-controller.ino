int PIN_LED = 17;
int PIN_BTN = 3;

int btnPrevious = 1;
long int tDown = 0;
boolean pressHandled = true;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);

  Serial.begin(9600); //This pipes to the serial monitor
  Serial.println("Initialize Serial Monitor");
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


  // Serial.println(digitalRead(PIN_BTN));
  digitalWrite(PIN_LED, digitalRead(PIN_BTN));

  // Loop interval
  delay(50);
}

void onShortClick() {

}

void onLongClick() {

}
