#include <EEPROM.h>

// Pin definitions
const int buzzerPin = A1;
const int redLED = A2;       // Alert LED when both powers present
const int yellowLED = A0;    // GF power indicator LED
const int blueLED = 12;      // FF power indicator LED
const int whiteLED = 13;     // Pump running indicator (pulsing)

const int relayGF = 8;       // Ground Floor relay control
const int relayFF = 7;       // First Floor relay control

const int button15 = 6;      // Timer 15 min button (active LOW)
const int button30 = 5;      // Timer 30 min button (active LOW)
const int button60 = 4;      // Timer 60 min button (active LOW)

const int timerLED15 = 11;   // Timer LEDs (green)
const int timerLED30 = 10;
const int timerLED60 = 9;

const int optoGF = 3;        // Optocoupler GF AC detect (active LOW means AC present)
const int optoFF = 2;        // Optocoupler FF AC detect

// Timer variables and EEPROM address
unsigned long timerDuration = 0;          // in minutes
unsigned long timerStart = 0;
bool timerRunning = false;
const int EEPROM_ADDR = 0;

// Variables for buzzer pulsing alert
unsigned long buzzerPulseStart = 0;
bool buzzerState = false;
const unsigned long buzzerOnDuration = 300;   // 300 ms beep
const unsigned long buzzerOffDuration = 700;  // 700 ms silent

// Variables for white LED pulsing
int whiteLEDBrightness = 0;
int whiteLEDFadeAmount = 5;  // change per cycle
unsigned long lastWhiteLEDFade = 0;
const unsigned long whiteLEDFadeInterval = 30;  // ms

// To track if pump was ON to do double beep once at start
bool pumpPreviouslyOn = false;

void setup() {
  // Initialize pins
  pinMode(buzzerPin, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  pinMode(whiteLED, OUTPUT);

  pinMode(relayGF, OUTPUT);
  pinMode(relayFF, OUTPUT);

  pinMode(button15, INPUT_PULLUP);
  pinMode(button30, INPUT_PULLUP);
  pinMode(button60, INPUT_PULLUP);

  pinMode(timerLED15, OUTPUT);
  pinMode(timerLED30, OUTPUT);
  pinMode(timerLED60, OUTPUT);

  pinMode(optoGF, INPUT_PULLUP);
  pinMode(optoFF, INPUT_PULLUP);

  // Buzzer double beep on startup
  for (int i = 0; i < 2; i++) {
    tone(buzzerPin, 1000);
    delay(200);
    noTone(buzzerPin);
    delay(200);
  }

  // Initialize variables
  timerDuration = EEPROM.read(EEPROM_ADDR);
  if (timerDuration != 15 && timerDuration != 30 && timerDuration != 60) {
    timerDuration = 0;  // invalid EEPROM data fallback
  } else {
    // Start timer based on saved duration
    timerStart = millis();
    timerRunning = true;
  }

  // Initial LED and relay states OFF
  digitalWrite(redLED, LOW);
  digitalWrite(yellowLED, HIGH); // LEDs active LOW off state
  digitalWrite(blueLED, HIGH);
  digitalWrite(whiteLED, LOW);

  digitalWrite(relayGF, LOW);
  digitalWrite(relayFF, LOW);

  clearTimerLEDs();
  updateTimerLEDs(timerDuration);
}

void loop() {
  // Read AC presence (active LOW = AC present)
  bool acGF = digitalRead(optoGF) == LOW;
  bool acFF = digitalRead(optoFF) == LOW;

  // Update power detection LEDs (active LOW)
  digitalWrite(yellowLED, acGF ? LOW : HIGH);
  digitalWrite(blueLED, acFF ? LOW : HIGH);

  // Read timer buttons (active LOW)
  if (digitalRead(button15) == LOW) {
    startTimer(15);
    delay(300); // debounce
  } else if (digitalRead(button30) == LOW) {
    startTimer(30);
    delay(300);
  } else if (digitalRead(button60) == LOW) {
    startTimer(60);
    delay(300);
  }

  // Alert and pump logic
  if (acGF && acFF) {
    // Both powers present - ALERT mode, pump OFF
    digitalWrite(redLED, HIGH);
    digitalWrite(relayGF, LOW);
    digitalWrite(relayFF, LOW);
    timerRunning = false;
    clearTimerLEDs();
    pumpPreviouslyOn = false;
    noTone(buzzerPin); // We'll pulse buzzer manually below

    handleBuzzerPulse();

    noPulseWhiteLED();
  } else if (acGF && !acFF) {
    // Only GF power
    digitalWrite(redLED, LOW);
    digitalWrite(relayFF, LOW);
    digitalWrite(relayGF, HIGH);

    if (!pumpPreviouslyOn) {
      pumpDoubleBeep();
      pumpPreviouslyOn = true;
    }

    runPumpTimer();

    handleWhiteLEDPulse();
  } else if (!acGF && acFF) {
    // Only FF power
    digitalWrite(redLED, LOW);
    digitalWrite(relayGF, LOW);
    digitalWrite(relayFF, HIGH);

    if (!pumpPreviouslyOn) {
      pumpDoubleBeep();
      pumpPreviouslyOn = true;
    }

    runPumpTimer();

    handleWhiteLEDPulse();
  } else {
    // No power on GF or FF
    digitalWrite(redLED, LOW);
    digitalWrite(relayGF, LOW);
    digitalWrite(relayFF, LOW);
    timerRunning = false;
    clearTimerLEDs();
    pumpPreviouslyOn = false;

    noTone(buzzerPin);
    noPulseWhiteLED();
  }
}

void startTimer(int minutes) {
  timerDuration = minutes;
  timerStart = millis();
  timerRunning = true;
  EEPROM.write(EEPROM_ADDR, timerDuration);
  updateTimerLEDs(timerDuration);
}

void runPumpTimer() {
  if (timerRunning) {
    unsigned long elapsed = (millis() - timerStart) / 60000; // in minutes
    if (elapsed >= timerDuration) {
      timerRunning = false;
      clearTimerLEDs();
      digitalWrite(relayGF, LOW);
      digitalWrite(relayFF, LOW);
      pumpPreviouslyOn = false;
      noPulseWhiteLED();
    } else {
      updateTimerLEDs(timerDuration);
    }
  }
}

// Buzzer pulse when both powers present
void handleBuzzerPulse() {
  unsigned long currentMillis = millis();
  if (buzzerState) {
    if (currentMillis - buzzerPulseStart >= buzzerOnDuration) {
      noTone(buzzerPin);
      buzzerState = false;
      buzzerPulseStart = currentMillis;
    }
  } else {
    if (currentMillis - buzzerPulseStart >= buzzerOffDuration) {
      tone(buzzerPin, 1000);
      buzzerState = true;
      buzzerPulseStart = currentMillis;
    }
  }
}

void pumpDoubleBeep() {
  for (int i = 0; i < 2; i++) {
    tone(buzzerPin, 1000);
    delay(200);
    noTone(buzzerPin);
    delay(200);
  }
}

// Timer LEDs control
void clearTimerLEDs() {
  digitalWrite(timerLED15, LOW);
  digitalWrite(timerLED30, LOW);
  digitalWrite(timerLED60, LOW);
}

void updateTimerLEDs(int minutes) {
  clearTimerLEDs();
  if (minutes == 15) digitalWrite(timerLED15, HIGH);
  else if (minutes == 30) digitalWrite(timerLED30, HIGH);
  else if (minutes == 60) digitalWrite(timerLED60, HIGH);
}

// White LED pulsing (fade effect)
void handleWhiteLEDPulse() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastWhiteLEDFade >= whiteLEDFadeInterval) {
    lastWhiteLEDFade = currentMillis;
    whiteLEDBrightness += whiteLEDFadeAmount;
    if (whiteLEDBrightness <= 0 || whiteLEDBrightness >= 255) {
      whiteLEDFadeAmount = -whiteLEDFadeAmount;
    }
    analogWrite(whiteLED, whiteLEDBrightness);
  }
}

void noPulseWhiteLED() {
  noTone(buzzerPin);
  analogWrite(whiteLED, 0);
}

// End of code
