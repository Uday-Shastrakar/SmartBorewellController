#include <EEPROM.h>

// ======================= Pin Mapping =======================
const int redLED = A1;
const int buzzerPin = A2;
const int yellowLED = A0;
const int blueLED = 12;
const int whiteLED = 13;
const int relayGF = 8;
const int relayFF = 7;
const int button15 = 4;
const int button30 = 5;
const int button60 = 6;
const int timerLED15 = 9;
const int timerLED30 = 10;
const int timerLED60 = 11;
const int optoGF = 3;
const int optoFF = 2;
const int optoMotor = A3;

const int EEPROM_ADDR = 0;

// ======================= Global Variables =======================
bool emergencyStopActive = false;
unsigned long button15PressStart = 0;

unsigned long timerDuration = 15;     // minutes
unsigned long timerStart = 0;         // millis start
bool timerRunning = false;

bool pumpRunning = false;
bool pumpPreviouslyRunning = false;
bool pumpStoppedByTimer = false;      // flag for pump stopped by timer

int whiteLEDBrightness = 0;
int whiteLEDFadeAmount = 5;
unsigned long lastWhiteLEDFade = 0;
const unsigned long whiteLEDFadeInterval = 30;

unsigned long lastSerialUpdate = 0;
const unsigned long serialInterval = 1000;

unsigned long powerDetectedTime = 0;
const unsigned long powerConfirmDelay = 5000; // 5s confirmation
bool powerPreviouslyDetected = false;
bool powerConfirmed = false;

unsigned long relayOnStartTime = 0;
const unsigned long relayHoldTime = 20000; // keep relays for 20s "hold" window if used
bool relayIsOn = false;

unsigned long relayOffStartTime = 0;
const unsigned long relayOffDelay = 20000; // 20s retry delay
bool relayOffCountdown = false;

bool motorCheckInProgress = false;
unsigned long motorCheckStart = 0;

unsigned long lastAlertToggle = 0;
bool alertState = false;

// ======================= Flags =======================
bool alertBuzzerTriggered = false;

// ======================= Buzzer/Melody =======================
volatile bool melodyPlaying = false;
int melodyIndex = 0;
unsigned long noteStartTime = 0;
const int noteDurationMs = 300;
const unsigned long notePauseMs = 50;
const int* currentMelody = nullptr;
int currentMelodyLength = 0;

const int melodyGF[] = {440, 554, 659};
const int melodyFF[] = {523, 659, 784};
const int melodyDefault[] = {392, 392, 330, 330};

struct BuzzerTask {
  bool active = false;
  unsigned long startTime = 0;
  int count = 0;
  int duration = 0;
  int freq = 1000;
  int beepsDone = 0;
} buzzer;

// ======================= Function Prototypes =======================
bool readOptoAC(int pin);
void handleBuzzer();
void startBuzzer(int durationMs, int beepCount, int frequency = 1000);
void activateEmergencyStop();
void handleButtons();
void setTimerEEPROM(int minutes);
void confirmPower(unsigned long now, bool acGF, bool acFF);
void runPumpTimer();
void resetSystem(const char* msg);
void alertBuzzer();
void clearTimerLEDs();
void updateTimerLEDs(int minutes);
void handleWhiteLEDPulse();
void noPulseWhiteLED();
void handleMotorSafety(unsigned long now, bool acGF, bool acFF, bool motorOptoState);
void printStatus(unsigned long now, bool acGF, bool acFF, bool motorOptoState, bool pumpRunning);
void playNextMelodyNote();
void handleMelody();
void startRelayGFMelody();
void startRelayFFMelody();
void startDefaultMelody();

// ======================= Setup =======================
void setup() {
  Serial.begin(9600);
  Serial.println("=== Pump Control System Initialized ===");

  // Pin Modes
  pinMode(redLED, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
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
  pinMode(optoMotor, INPUT_PULLUP);

  // default relays OFF (HIGH for active-low modules)
  digitalWrite(relayGF, HIGH);
  digitalWrite(relayFF, HIGH);
  clearTimerLEDs();
  digitalWrite(redLED, LOW);
  analogWrite(whiteLED, 0);

  byte storedTime = EEPROM.read(EEPROM_ADDR);
  if (storedTime == 0xFF) storedTime = 15;
  timerDuration = storedTime;
  updateTimerLEDs(timerDuration);

  Serial.print("Loaded Timer from EEPROM: ");
  Serial.println(timerDuration);
}

// ======================= Main Loop =======================
void loop() {
  unsigned long now = millis();

  // ================= AC Detection =================
  bool acGF = readOptoAC(optoGF);
  bool acFF = readOptoAC(optoFF);
  bool motorOptoState = readOptoAC(optoMotor);

  handleButtons();
  handleBuzzer();
  handleMelody();

  // indicator LEDs for AC presence
  digitalWrite(yellowLED, acGF ? HIGH : LOW);
  digitalWrite(blueLED, acFF ? HIGH : LOW);

  // Emergency active: only reset when both powers are OFF
  if (emergencyStopActive) {
    if (!acGF && !acFF) {
      Serial.println("Both powers OFF. Emergency reset.");
      emergencyStopActive = false;
      alertBuzzerTriggered = false;
      digitalWrite(redLED, LOW);
    }
    return; // skip rest of loop while emergency
  }

  // No AC power at all -> reset everything
  if (!acGF && !acFF) {
    resetSystem("NO Power from FF & GF - Timer reset");
    return;
  }

  // If both GF and FF present -> critical alert
  if (acGF && acFF) {
    alertBuzzer();
    return;
  } else {
    if (!alertBuzzerTriggered) {
      digitalWrite(redLED, LOW);
      noTone(buzzerPin);
    }
  }

  // Confirm stable power (after powerConfirmDelay)
  confirmPower(now, acGF, acFF);

  // --- Relay Decision ---
  bool desiredRelayGF = HIGH;
  bool desiredRelayFF = HIGH;

  if (powerConfirmed && !relayOffCountdown && !pumpStoppedByTimer) { 
    // only turn on relays if pump not stopped by timer
    if (acGF && !acFF) desiredRelayGF = LOW;
    else if (!acGF && acFF) desiredRelayFF = LOW;
  }

  // Relay hold logic
  if (!relayIsOn && (desiredRelayGF == LOW || desiredRelayFF == LOW)) {
    relayOnStartTime = now;
    relayIsOn = true;
  }

  if (relayIsOn) {
    if (now - relayOnStartTime < relayHoldTime) {
      digitalWrite(relayGF, desiredRelayGF);
      digitalWrite(relayFF, desiredRelayFF);
    } else {
      digitalWrite(relayGF, desiredRelayGF);
      digitalWrite(relayFF, desiredRelayFF);
      if (desiredRelayGF == HIGH && desiredRelayFF == HIGH) relayIsOn = false;
    }
  } else {
    digitalWrite(relayGF, desiredRelayGF);
    digitalWrite(relayFF, desiredRelayFF);
  }

  // Motor safety check
  handleMotorSafety(now, acGF, acFF, motorOptoState);

  // Pump running logic
  pumpRunning = (digitalRead(relayGF) == LOW || digitalRead(relayFF) == LOW) && motorOptoState;

  if (pumpRunning && !pumpPreviouslyRunning) {
    timerStart = now;
    timerRunning = true;
    pumpStoppedByTimer = false; // reset flag
    startBuzzer(200, 1, 1200); // pump start beep
    alertBuzzerTriggered = true;
  }
  pumpPreviouslyRunning = pumpRunning;

  // Run timer logic
  runPumpTimer();

  // White LED pulse
  if (pumpRunning) handleWhiteLEDPulse();
  else noPulseWhiteLED();

  // Timer LED
  updateTimerLEDs(timerDuration);

  // Serial debug/status every 1 sec
  if (now - lastSerialUpdate >= serialInterval) {
    lastSerialUpdate = now;
    printStatus(now, acGF, acFF, motorOptoState, pumpRunning);
  }
}

// ======================= AC Detection =======================
bool readOptoAC(int pin) {
  int sum = 0;
  const int samples = 10;
  for (int i = 0; i < samples; i++) {
    sum += digitalRead(pin); // LOW = active
    delay(1);
  }
  int avg = sum / samples;
  return avg < 0.5;
}

// ======================= Buzzer Handling =======================
void startBuzzer(int durationMs, int beepCount, int frequency) {
  buzzer.active = true;
  buzzer.startTime = millis();
  buzzer.duration = durationMs;
  buzzer.count = beepCount;
  buzzer.freq = frequency;
  buzzer.beepsDone = 0;
  tone(buzzerPin, buzzer.freq, buzzer.duration);
}

void handleBuzzer() {
  if (!buzzer.active) return;
  unsigned long now = millis();
  if (now - buzzer.startTime >= (unsigned long)(buzzer.duration + 50)) {
    buzzer.beepsDone++;
    if (buzzer.beepsDone >= buzzer.count) {
      buzzer.active = false;
      noTone(buzzerPin);
    } else {
      buzzer.startTime = now;
      tone(buzzerPin, buzzer.freq, buzzer.duration);
    }
  }
}

// ======================= Emergency Stop =======================
void activateEmergencyStop() {
  Serial.println("EMERGENCY STOP ACTIVATED");
  emergencyStopActive = true;

  // turn everything OFF
  digitalWrite(relayGF, HIGH);
  digitalWrite(relayFF, HIGH);
  timerRunning = false;
  pumpRunning = false;
  clearTimerLEDs();
  noPulseWhiteLED();
  noTone(buzzerPin);

  // turn red LED ON to indicate emergency
  digitalWrite(redLED, HIGH);
}

// ======================= Button Handling =======================
void handleButtons() {
  static unsigned long lastPress = 0;
  unsigned long now = millis();
  bool button15Pressed = (digitalRead(button15) == LOW);

  // long hold for emergency stop (7 sec)
  if (button15Pressed) {
    if (button15PressStart == 0) button15PressStart = now;
    else if (!emergencyStopActive && now - button15PressStart >= 7000) activateEmergencyStop();
  } else button15PressStart = 0;

  if (emergencyStopActive) return;  // no further actions while emergency

  if (now - lastPress < 200) return; // simple debounce

  // Timer selection buttons
  if (digitalRead(button15) == LOW) {
    setTimerEEPROM(15);
    pumpStoppedByTimer = false; // reset flag
    startBuzzer(100, 1, 1000);
    lastPress = now;
  } else if (digitalRead(button30) == LOW) {
    setTimerEEPROM(30);
    pumpStoppedByTimer = false;
    startBuzzer(100, 1, 1000);
    lastPress = now;
  } else if (digitalRead(button60) == LOW) {
    setTimerEEPROM(60);
    pumpStoppedByTimer = false;
    startBuzzer(100, 1, 1000);
    lastPress = now;
  }
}

void setTimerEEPROM(int minutes) {
  timerDuration = minutes;
  EEPROM.write(EEPROM_ADDR, minutes);
  updateTimerLEDs(minutes);
}

// ======================= Power Confirmation =======================
void confirmPower(unsigned long now, bool acGF, bool acFF) {
  if (acGF || acFF) {
    if (!powerPreviouslyDetected) {
      powerPreviouslyDetected = true;
      powerDetectedTime = now;
      powerConfirmed = false;
    } else if (!powerConfirmed && now - powerDetectedTime >= powerConfirmDelay) {
      powerConfirmed = true;
      Serial.println("Power Confirmed");
    }
  } else {
    powerPreviouslyDetected = false;
    powerConfirmed = false;
  }
}

// ======================= Pump Timer =======================
void runPumpTimer() {
  if (!timerRunning) return;
  unsigned long elapsedMillis = millis() - timerStart;
  if (elapsedMillis >= timerDuration * 60000UL) {
    timerRunning = false;
    pumpRunning = false;
    relayIsOn = false;
    pumpStoppedByTimer = true;  // pump stopped by timer

    // turn relays OFF immediately
    digitalWrite(relayGF, HIGH);
    digitalWrite(relayFF, HIGH);

    clearTimerLEDs();
    noPulseWhiteLED();
    startBuzzer(300, 3, 900); // expiry buzzer
    Serial.println("TIMER EXPIRED - Pump OFF");
  }
}

// ======================= Reset =======================
void resetSystem(const char* msg) {
  static unsigned long lastResetMsg = 0;
  unsigned long now = millis();

  digitalWrite(relayGF, HIGH);
  digitalWrite(relayFF, HIGH);
  timerRunning = false;
  pumpRunning = false;
  clearTimerLEDs();
  noPulseWhiteLED();
  powerPreviouslyDetected = false;
  powerConfirmed = false;
  noTone(buzzerPin);
  pumpPreviouslyRunning = false;
  relayOffCountdown = false;

  if (now - lastResetMsg >= 1000) {
    Serial.println(msg);
    lastResetMsg = now;
  }
}

// ======================= Alert (Both sources present) =======================
void alertBuzzer() {
  digitalWrite(relayGF, HIGH);
  digitalWrite(relayFF, HIGH);
  digitalWrite(redLED, HIGH);
  if (millis() - lastAlertToggle >= 500) {
    lastAlertToggle = millis();
    alertState = !alertState;
    if (alertState) tone(buzzerPin, 2000);
    else noTone(buzzerPin);
  }
  alertBuzzerTriggered = true;
  timerRunning = false;
  pumpRunning = false;
  pumpStoppedByTimer = false;
  noPulseWhiteLED();
  clearTimerLEDs();
  powerPreviouslyDetected = false;
  powerConfirmed = false;
  pumpPreviouslyRunning = false;
  relayOffCountdown = false;
}

// ======================= Timer LEDs =======================
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

// ======================= White LED Pulse =======================
void handleWhiteLEDPulse() {
  unsigned long now = millis();
  if (now - lastWhiteLEDFade >= whiteLEDFadeInterval) {
    lastWhiteLEDFade = now;
    whiteLEDBrightness += whiteLEDFadeAmount;
    if (whiteLEDBrightness <= 0 || whiteLEDBrightness >= 255) whiteLEDFadeAmount = -whiteLEDFadeAmount;
    analogWrite(whiteLED, whiteLEDBrightness);
  }
}
void noPulseWhiteLED() { analogWrite(whiteLED, 0); }

// ======================= Motor Safety =======================
void handleMotorSafety(unsigned long now, bool acGF, bool acFF, bool motorOptoState) {
  bool relayON = (digitalRead(relayGF) == LOW || digitalRead(relayFF) == LOW);
  static bool storedMotorOpto = false;

  if (relayON) {
    if (!motorCheckInProgress) {
      motorCheckInProgress = true;
      motorCheckStart = now;
    }
    if (motorCheckInProgress && now - motorCheckStart >= 5000) {
      storedMotorOpto = motorOptoState;
      if (!storedMotorOpto) {
        relayOffCountdown = true;
        relayOffStartTime = now;
        digitalWrite(relayGF, HIGH);
        digitalWrite(relayFF, HIGH);
        pumpRunning = false;
        timerRunning = false;
        noPulseWhiteLED();
        Serial.println("Motor not detected after 5s - Relay OFF, waiting 20s");
        startBuzzer(200, 3, 1000);
      }
      motorCheckInProgress = false;
    }
  } else {
    storedMotorOpto = false;
    motorCheckInProgress = false;
  }

  if (relayOffCountdown && now - relayOffStartTime >= relayOffDelay) {
    relayOffCountdown = false;
    Serial.println("Retrying relay ON after 20s");
    if (powerConfirmed) {
      if (acGF && !acFF) {
        digitalWrite(relayGF, LOW);
        digitalWrite(relayFF, HIGH);
      } else if (!acGF && acFF) {
        digitalWrite(relayGF, HIGH);
        digitalWrite(relayFF, LOW);
      }
    }
  }
}

// ======================= Serial Status =======================
void printStatus(unsigned long now, bool acGF, bool acFF, bool motorOptoState, bool pumpRunning) {
  unsigned long elapsed = timerRunning ? now - timerStart : 0;
  unsigned long remainingMillis = timerDuration * 60000UL;
  if (elapsed < remainingMillis) remainingMillis -= elapsed;
  unsigned int remainingMinutes = remainingMillis / 60000;
  unsigned int remainingSeconds = (remainingMillis % 60000) / 1000;

  Serial.print("Pump: "); Serial.print(pumpRunning ? "ON" : "OFF");
  Serial.print(" | Timer Running: "); Serial.print(timerRunning ? "YES" : "NO");
  Serial.print(" | Time Remaining: "); Serial.print(remainingMinutes); Serial.print(":");
  if (remainingSeconds < 10) Serial.print("0");
  Serial.print(remainingSeconds);
  Serial.print(" | Selected Timer: "); Serial.print(timerDuration);
  Serial.print(" | GF Relay: "); Serial.print(digitalRead(relayGF) == LOW ? "ON" : "OFF");
  Serial.print(" | FF Relay: "); Serial.print(digitalRead(relayFF) == LOW ? "ON" : "OFF");
  Serial.print(" | optoGF: "); Serial.print(acGF ? "ON" : "OFF");
  Serial.print(" | optoFF: "); Serial.print(acFF ? "ON" : "OFF");
  Serial.print(" | Motor Opto: "); Serial.print(motorOptoState ? "ON" : "OFF");
  Serial.print(" | Emergency: "); Serial.println(emergencyStopActive ? "YES" : "NO");
}



// ======================= Melody =======================
void handleMelody() {
  if (!melodyPlaying) return;
  if (millis() - noteStartTime >= noteDurationMs + notePauseMs) playNextMelodyNote();
}

void playNextMelodyNote() {
  if (melodyIndex >= currentMelodyLength) {
    noTone(buzzerPin);
    melodyPlaying = false;
    melodyIndex = 0;
    currentMelody = nullptr;
    return;
  }
  int freq = currentMelody[melodyIndex];
  tone(buzzerPin, freq, noteDurationMs);
  noteStartTime = millis();
  melodyIndex++;
  melodyPlaying = true;
}

void startRelayGFMelody() {
  currentMelody = melodyGF;
  currentMelodyLength = sizeof(melodyGF) / sizeof(int);
  melodyIndex = 0;
  playNextMelodyNote();
}

void startRelayFFMelody() {
  currentMelody = melodyFF;
  currentMelodyLength = sizeof(melodyFF) / sizeof(int);
  melodyIndex = 0;
  playNextMelodyNote();
}

void startDefaultMelody() {
  currentMelody = melodyDefault;
  currentMelodyLength = sizeof(melodyDefault) / sizeof(int);
  melodyIndex = 0;
  playNextMelodyNote();
}




