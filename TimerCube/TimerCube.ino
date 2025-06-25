#include <Wire.h>
#include <MPU6050.h>

// MPU6050 Variables
MPU6050 mpu;
bool mpuConnected = false;
unsigned long lastMPUCheck = 0;
const unsigned long MPU_CHECK_INTERVAL = 5000; // Check every 5 seconds

// GPIO Definitions
#define TOUCH_PIN 4
#define BUZZER_PIN 5
#define LED_POMODORO 18
#define LED_SHORT 19
#define LED_LONG 23

#define DEBOUNCE_DELAY 50
#define LONG_PRESS_DURATION 1500

// Timer Variables
unsigned long timerBeginMillis = 0;
unsigned long timerDuration = 0;
bool timerRunning = false;
bool timerComplete = false;
bool timerPaused = false;

// Touch State
bool lastTouchState = LOW;
bool touchState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long touchStartTime = 0;
bool touchActive = false;
bool buzzerActive = false;

// Manual mode selection when MPU is not available
int manualModeIndex = 0; // 0=pomodoro, 1=short, 2=long
String modes[] = {"pomodoro", "short", "long"};

// LED Breathing Effect
unsigned long lastBreathTime = 0;
int breathDirection = 1;
int breathBrightness = 0;
const int breathSpeed = 20;

String currentMode = "";

void setup() {
  Serial.begin(115200);
  
  // Initialize GPIO pins first
  pinMode(TOUCH_PIN, INPUT);
  pinMode(LED_POMODORO, OUTPUT);
  pinMode(LED_SHORT, OUTPUT);
  pinMode(LED_LONG, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  setLEDsOff();
  digitalWrite(BUZZER_PIN, LOW);
  
  // Initialize I2C and MPU6050
  initializeMPU();
  
  Serial.println("Device initialized. Pomodoro timer ready!");
}

void loop() {
  // Only check MPU if it's connected, otherwise retry connection periodically
  if (mpuConnected) {
    checkMPUOrientation();
  } else {
    // Try to reconnect periodically
    if (millis() - lastMPUCheck > MPU_CHECK_INTERVAL) {
      initializeMPU();
    }
  }
  
  handleTouchSensor();
  handleTimer();
  handleBuzzer();
  handleLEDs();
  delay(50);
}

void initializeMPU() {
  Wire.begin(21, 22);
  Wire.setClock(100000); // Set I2C clock to 100kHz for better reliability
  
  delay(100); // Give I2C time to initialize
  
  mpu.initialize();
  delay(100);
  
  // Test connection with timeout
  bool connectionTest = false;
  for (int i = 0; i < 3; i++) {
    if (mpu.testConnection()) {
      connectionTest = true;
      break;
    }
    delay(100);
  }
  
  if (connectionTest) {
    mpuConnected = true;
    Serial.println("MPU6050 Connected Successfully!");
  } else {
    mpuConnected = false;
    Serial.println("MPU6050 Connection Failed - Operating in touch-only mode");
  }
  
  lastMPUCheck = millis();
}

void checkMPUOrientation() {
  if (!mpuConnected) return;
  
  int16_t ax, ay, az;
  
  // Try to get acceleration data with error handling
  try {
    mpu.getAcceleration(&ax, &ay, &az);
  } catch (...) {
    // If reading fails, mark MPU as disconnected
    mpuConnected = false;
    Serial.println("MPU6050 read error - switching to touch-only mode");
    return;
  }

  float x = ax / 16384.0;
  float z = az / 16384.0;

  String newMode = "";
  if (z > 0.8) newMode = "pomodoro";
  else if (x > 0.8) newMode = "short";
  else if (x < -0.8) newMode = "long";

  if (newMode != "" && newMode != currentMode) {
    resetTimer();
    currentMode = newMode;
    
    // Set timer duration based on mode
    if (currentMode == "pomodoro") timerDuration = 2 * 60 * 1000UL;
    else if (currentMode == "short") timerDuration = 30 * 1000UL;
    else if (currentMode == "long") timerDuration = 1 * 60 * 1000UL;

    Serial.print("Mode switched to: ");
    Serial.println(currentMode);
    shortBeep();
    startTimer();
  }
}

void handleTouchSensor() {
  int reading = digitalRead(TOUCH_PIN);

  if (reading != lastTouchState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != touchState) {
      touchState = reading;

      if (touchState == HIGH) {
        touchStartTime = millis();
        touchActive = true;
      } else {
        if (touchActive) {
          unsigned long duration = millis() - touchStartTime;
          if (duration < LONG_PRESS_DURATION) {
            // Short press behavior
            if (timerRunning) {
              // Pause/resume timer
              timerPaused = !timerPaused;
              Serial.println(timerPaused ? "Timer Paused" : "Timer Resumed");
              shortBeep();
            } else if (timerComplete) {
              // Acknowledge timer completion
              timerComplete = false;
              buzzerActive = false;
              setLEDsOff();
              Serial.println("Timer Complete Acknowledged");
            } else if (!mpuConnected && !timerRunning) {
              // Manual mode selection when MPU is not available
              cycleModeManually();
            }
          }
        }
        touchActive = false;
      }
    }
  }

  // Long press detection
  if (touchActive && (millis() - touchStartTime >= LONG_PRESS_DURATION)) {
    if (timerRunning || timerPaused) {
      resetTimer();
      longBeep();
    }
    touchActive = false;
  }

  lastTouchState = reading;
}

void cycleModeManually() {
  manualModeIndex = (manualModeIndex + 1) % 3;
  currentMode = modes[manualModeIndex];
  
  // Set timer duration based on mode
  if (currentMode == "pomodoro") timerDuration = 2 * 60 * 1000UL;
  else if (currentMode == "short") timerDuration = 30 * 1000UL;
  else if (currentMode == "long") timerDuration = 1 * 60 * 1000UL;

  Serial.print("Manual mode selected: ");
  Serial.println(currentMode);
  shortBeep();
  startTimer();
}

void startTimer() {
  timerRunning = true;
  timerComplete = false;
  timerPaused = false;
  timerBeginMillis = millis();
  
  // Reset breathing effect variables
  breathBrightness = 0;
  breathDirection = 1;
  lastBreathTime = millis();
  
  shortBeep();
}

void resetTimer() {
  timerRunning = false;
  timerPaused = false;
  timerComplete = false;
  buzzerActive = false;
  
  // Turn off all LEDs immediately
  setLEDsOff();
  digitalWrite(BUZZER_PIN, LOW);
  
  // Reset breathing effect variables
  breathBrightness = 0;
  breathDirection = 1;
  
  Serial.println("Timer Reset");
}

void handleTimer() {
  if (timerRunning && !timerPaused && currentMode != "") {
    if (millis() - timerBeginMillis >= timerDuration) {
      timerRunning = false;
      timerComplete = true;
      buzzerActive = true;
      Serial.println("TIMER COMPLETE!");
    }
  }
}

void handleBuzzer() {
  if (timerComplete && buzzerActive) {
    // Buzzer beeping pattern when timer is complete
    digitalWrite(BUZZER_PIN, (millis() % 1000 < 500) ? HIGH : LOW);
  } else {
    // Make sure buzzer is off when not needed
    digitalWrite(BUZZER_PIN, LOW);
  }
}

void handleLEDs() {
  if (timerComplete) {
    // Timer complete - LED solid (buzzer handled separately)
    setLEDStatic(currentMode);
    return;
  }

  if (timerRunning && !timerPaused) {
    // Timer running - breathing effect
    if (millis() - lastBreathTime > breathSpeed) {
      lastBreathTime = millis();
      breathBrightness += breathDirection * 5;
      if (breathBrightness >= 255) {
        breathBrightness = 255;
        breathDirection = -1;
      } else if (breathBrightness <= 0) {
        breathBrightness = 0;
        breathDirection = 1;
      }
      setLEDBrightness(currentMode, breathBrightness);
    }
  } else if (timerPaused) {
    // Timer paused - LED solid
    setLEDStatic(currentMode);
  } else {
    // Timer not running - all LEDs off
    setLEDsOff();
  }
}

void setLEDStatic(String mode) {
  // Turn off all LEDs first (use analogWrite to override any PWM)
  analogWrite(LED_POMODORO, 0);
  analogWrite(LED_SHORT, 0);
  analogWrite(LED_LONG, 0);
  
  // Turn on the appropriate LED at full brightness
  if (mode == "pomodoro") {
    analogWrite(LED_POMODORO, 255);
  } else if (mode == "short") {
    analogWrite(LED_SHORT, 255);
  } else if (mode == "long") {
    analogWrite(LED_LONG, 255);
  }
}

void setLEDBrightness(String mode, int brightness) {
  // Turn off all LEDs first (use analogWrite to override any PWM)
  analogWrite(LED_POMODORO, 0);
  analogWrite(LED_SHORT, 0);
  analogWrite(LED_LONG, 0);
  
  // Set brightness for the appropriate LED
  if (mode == "pomodoro") {
    analogWrite(LED_POMODORO, brightness);
  } else if (mode == "short") {
    analogWrite(LED_SHORT, brightness);
  } else if (mode == "long") {
    analogWrite(LED_LONG, brightness);
  }
}

void setLEDsOff() {
  analogWrite(LED_POMODORO, 0);
  analogWrite(LED_SHORT, 0);
  analogWrite(LED_LONG, 0);
}

void shortBeep() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
}

void longBeep() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(800);
  digitalWrite(BUZZER_PIN, LOW);
}