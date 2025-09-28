// ===============================
//   Gas, Vibration & Temperature Alarm System
//   Using DS18B20, MQ2, MQ7, and Vibration Sensor
//   With LCD + Buzzer Output
//   Improved & Refactored Version
// ===============================

// --- Libraries (for temperature sensor, LCD, and OneWire protocol)
#include <DallasTemperature.h>
#include <LiquidCrystal.h>
#include <OneWire.h>

// --- Pin Definitions
const int VIB_PIN   = 10;   // Vibration sensor input pin
const int ONEWIRE_PIN = 11; // OneWire bus pin for DS18B20 temperature sensor
const int BUZZER_PIN = 12;  // Buzzer output pin
const int MQ2_PIN   = A0;   // MQ2 sensor analog input
const int MQ7_PIN   = A4;   // MQ7 sensor analog input

// --- LCD Initialization (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

// --- OneWire protocol object for DS18B20
OneWire oneWire(ONEWIRE_PIN);
DallasTemperature sensors(&oneWire);

// --- Sensor Thresholds & Variables
const int MQ_THRESHOLD = 350;  // Threshold for MQ2 & MQ7 sensors
float currentTemp = 0.0;       // Current temperature
int MQ2Value = 0;              // MQ2 sensor value
int MQ7Value = 0;              // MQ7 sensor value
int vibrationValue = 0;        // Vibration sensor value

// --- Timing Variables
unsigned long lastDataSend = 0;                // Last time data was sent to Serial
const unsigned long dataSendInterval = 1000;   // Interval for sending live data (1s)

// --- Alarm System
enum AlarmType { NONE, VIB_ALARM, MQ2_ALARM, MQ7_ALARM };
AlarmType currentAlarm = NONE;                 // Current active alarm
unsigned long alarmStartTime = 0;              // Start time of alarm
const unsigned long alarmDuration = 5000;      // Alarm duration (5s)

// ===============================
//   Setup Function
// ===============================
void setup() {
  Serial.begin(9600);              // Start serial communication
  sensors.setResolution(11);       // Set DS18B20 resolution (11-bit)

  lcd.begin(16, 2);                // Initialize LCD (16x2)
  lcd.clear();                     // Clear LCD screen

  pinMode(BUZZER_PIN, OUTPUT);     // Buzzer as output
  pinMode(VIB_PIN, INPUT);         // Vibration sensor as input
}

// ===============================
//   Main Loop
// ===============================
void loop() {
  unsigned long currentMillis = millis();

  // --- Read Sensor Values
  MQ2Value = analogRead(MQ2_PIN);                // Read MQ2
  MQ7Value = analogRead(MQ7_PIN);                // Read MQ7
  vibrationValue = digitalRead(VIB_PIN);         // Read vibration
  sensors.requestTemperatures();                 // Request temperature
  currentTemp = sensors.getTempCByIndex(0);      // Get temperature in Celsius

  // --- Check DS18B20 errors
  if (currentTemp == DEVICE_DISCONNECTED_C) {
    Serial.println("ERROR: Temp sensor disconnected");
    currentTemp = -127; // invalid value
  }

  // --- Send Live Data over Serial (every 1 second, JSON format)
  if (currentMillis - lastDataSend >= dataSendInterval) {
    lastDataSend = currentMillis;
    Serial.print("{\"TEMP\":");
    Serial.print(currentTemp, 1);
    Serial.print(",\"MQ2\":");
    Serial.print(MQ2Value);
    Serial.print(",\"MQ7\":");
    Serial.print(MQ7Value);
    Serial.print(",\"VIB\":");
    Serial.print(vibrationValue);
    Serial.println("}");
  }

  // --- Alarm Check
  if (currentAlarm == NONE) {
    if (vibrationValue == HIGH) {
      startAlarm(VIB_ALARM, "EARTHQUAKE!", "!!DANGER!!");
    } else if (MQ2Value >= MQ_THRESHOLD) {
      startAlarm(MQ2_ALARM, "!!!FUME & GAS!!!", "!!DANGER!!");
    } else if (MQ7Value >= MQ_THRESHOLD) {
      startAlarm(MQ7_ALARM, "!MONOXIDE GAS!", "!!DANGER!!");
    }
  } else {
    // If alarm active → manage buzzer blinking & check timeout
    if (currentMillis - alarmStartTime < alarmDuration) {
      // Blink buzzer every 500ms
      if ((currentMillis / 500) % 2 == 0) digitalWrite(BUZZER_PIN, HIGH);
      else digitalWrite(BUZZER_PIN, LOW);
    } else {
      // End alarm
      digitalWrite(BUZZER_PIN, LOW);
      lcd.clear();
      currentAlarm = NONE;
    }
  }

  // --- Display Values on LCD when no alarm
  if (currentAlarm == NONE) {
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(currentTemp, 1);
    lcd.print(" C");

    lcd.setCursor(0, 1);
    lcd.print("M2:");
    lcd.print(MQ2Value);
    lcd.print(" M7:");
    lcd.print(MQ7Value);
  }
}

// ===============================
//   Function to Start Alarm
// ===============================
void startAlarm(AlarmType type, const char* line1, const char* line2) {
  currentAlarm = type;
  alarmStartTime = millis();
  Serial.print("ALARM:");
  Serial.println(type);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}
