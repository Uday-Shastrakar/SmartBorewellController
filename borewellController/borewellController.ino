// Pin definitions
const int buzzerPin = A1;
const int redLED = A2;       // Alert LED
const int yellowLED = A0;    // Ground Floor AC LED
const int blueLED = 12;      // First Floor AC LED
const int whiteLED = 13;     // Pump ON LED

const int relayGF = 8;       // Ground Floor relay control
const int relayFF = 7;       // First Floor relay control

const int button15 = 6;      // Timer selection 15 min (active LOW)
const int button30 = 5;      // Timer selection 30 min (active LOW)
const int button60 = 4;      // Timer selection 60 min (active LOW)

const int timerLED15 = 11;   // Timer status LEDs green
const int timerLED30 = 10;
const int timerLED60 = 9;

const int optoGF = 3;        // Optocoupler GF AC detect input (active LOW means AC present)
const int optoFF = 2;        // Optocoupler FF AC detect input (active LOW means AC present)

// Timer variables
unsigned long timerDuration = 0;      // in minutes
unsigned long timerStart = 0;
bool timerRunning = false;

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

  // Initial states
  digitalWrite(relayGF, LOW);
  digitalWrite(relayFF, LOW);
  digitalWrite(whiteLED, LOW);
  digitalWrite(redLED, LOW);

  digitalWrite(timerLED15, LOW);
  digitalWrite(timerLED30, LOW);
  digitalWrite(timerLED60, LOW);

  digitalWrite(yellowLED, HIGH);  // LED off (assuming active LOW)
  digitalWrite(blueLED, HIGH);    // LED off

  noTone(buzzerPin);  // Buzzer off
}

void loop() {
  // Read AC presence from optocouplers (LOW means AC present)
  bool acGF = digitalRead(optoGF) == LOW;
  bool acFF = digitalRead(optoFF) == LOW;

  // Update AC status LEDs (active LOW LED)
  digitalWrite(yellowLED, acGF ? LOW : HIGH);
  digitalWrite(blueLED, acFF ? LOW : HIGH);

  // Check for AC loss alert
  if (!acGF || !acFF) {
    digitalWrite(redLED, HIGH);       // Alert LED ON
    tone(buzzerPin, 1000);            // Buzzer ON
    stopPump();
    timerRunning = false;             // Stop timer on fault
  } else {
    digitalWrite(redLED, LOW);        // Alert LED OFF
    noTone(buzzerPin);                // Buzzer OFF
  }

  // Read timer selection buttons (active LOW)
  if (digitalRead(button15) == LOW) {
    startTimer(15);
    delay(300); // debounce
  } else if (digitalRead(button30) == LOW) {
    startTimer(30);
    delay(300); // debounce
  } else if (digitalRead(button60) == LOW) {
    startTimer(60);
    delay(300); // debounce
  }

  // Timer countdown logic
  if (timerRunning) {
    unsigned long elapsed = (millis() - timerStart) / 60000; // elapsed minutes
    if (elapsed >= timerDuration) {
      stopPump();
      timerRunning = false;
      clearTimerLEDs();
    } else {
      updateTimerLEDs(timerDuration);
      startPump(acGF, acFF);
    }
  } else {
    stopPump();
    clearTimerLEDs();
  }
}

// Start pump relays and ON LED when AC is present
void startPump(bool acGF, bool acFF) {
  digitalWrite(relayGF, acGF ? HIGH : LOW);
  digitalWrite(relayFF, acFF ? HIGH : LOW);

  digitalWrite(whiteLED, HIGH);  // Pump ON LED
}

// Stop pumps and turn off pump LED
void stopPump() {
  digitalWrite(relayGF, LOW);
  digitalWrite(relayFF, LOW);

  digitalWrite(whiteLED, LOW);
}

// Start timer with duration in minutes
void startTimer(int minutes) {
  timerDuration = minutes;
  timerStart = millis();
  timerRunning = true;
}

// Turn off all timer LEDs
void clearTimerLEDs() {
  digitalWrite(timerLED15, LOW);
  digitalWrite(timerLED30, LOW);
  digitalWrite(timerLED60, LOW);
}

// Light the timer LED corresponding to selected duration
void updateTimerLEDs(int minutes) {
  clearTimerLEDs();
  if (minutes == 15) digitalWrite(timerLED15, HIGH);
  else if (minutes == 30) digitalWrite(timerLED30, HIGH);
  else if (minutes == 60) digitalWrite(timerLED60, HIGH);
}
