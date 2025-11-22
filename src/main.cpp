#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// Macros
#define IR_SENSOR_1 4
#define IR_SENSOR_2 3
#define SENSORS_DISTANCE_APART_CM 9 // 9 cm
#define ADDRESS 0x27
#define COLS 16
#define ROWS 2

// Object Declaration
LiquidCrystal_I2C lcd(ADDRESS, COLS, ROWS);

// =================== Variables ==========================================
unsigned long previous_time = 0;
unsigned long time_now = 0;
bool waitingForSecond = false;
uint8_t previous_state_sensor_1 = HIGH;
uint8_t previouse_state_sensor_2 = HIGH;

// ===================== Function declarations: ============================
uint8_t readSensor(uint8_t IR_sensor);
void lcdSetup(void);
void displaySensorReadings(float speed_cm_per_s, unsigned long change_in_time);
float calculateSpeed(unsigned long change_in_time);

// ============================= Program ===================================
void setup(){
  Serial.begin(115200);
  pinMode(IR_SENSOR_1, INPUT);
  pinMode(IR_SENSOR_2, INPUT);
  lcdSetup();

  // initialize previous states
  previous_state_sensor_1 = digitalRead(IR_SENSOR_1);
  previouse_state_sensor_2 = digitalRead(IR_SENSOR_2);
}

void loop(){
  // Read current states
  uint8_t curret_state_sensor_1 = readSensor(IR_SENSOR_1);
  uint8_t current_state_sensor_2 = readSensor(IR_SENSOR_2);

  // Detect falling edge on sensor 1 (HIGH -> LOW)
  if (previous_state_sensor_1 == HIGH && curret_state_sensor_1 == LOW){
    previous_time = micros();
    waitingForSecond = true;
    Serial.println("Sensor1 triggered");
  }

  // Wait for sensor2, and detect its falling edge
  if (waitingForSecond && previouse_state_sensor_2 == HIGH && current_state_sensor_2 == LOW){
    time_now = micros();
    unsigned long change_in_time = time_now - previous_time;
    float speed_cm_per_s = calculateSpeed(change_in_time);
    displaySensorReadings(speed_cm_per_s, change_in_time);
    waitingForSecond = false;
  }

  // Update previous states for next loop
  previous_state_sensor_1 = curret_state_sensor_1;
  previouse_state_sensor_2 = current_state_sensor_2;
}

// ============================== Function definitions: ==========================
void lcdSetup(void){
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Speed Monitor");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Detecting Speed");
  lcd.setCursor(0,1);
  lcd.print(">>>>>>>>>");
}

uint8_t readSensor(uint8_t IR_sensor){
  uint8_t reading = digitalRead(IR_sensor);
  return reading;
}

float calculateSpeed(unsigned long change_in_time){
  if (change_in_time == 0){
    return 0.0f;
  }
    
  float time_in_seconds = (float)change_in_time / 1000000.0f; // convert micros to seconds
  float speed_cm_per_s = (float)SENSORS_DISTANCE_APART_CM / time_in_seconds;
  return speed_cm_per_s;
}

void displaySensorReadings(float speed_cm_per_s, unsigned long change_in_time){
  // Print time and speed to Serial
  Serial.print("Delta (us): ");
  Serial.print(change_in_time);
  Serial.print("  Speed (cm/s): ");
  Serial.println(speed_cm_per_s);

  // Show short result on LCD (convert to m/s for readability)
  float speed_m_s = speed_cm_per_s / 100.0f;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Speed:");
  lcd.setCursor(7, 0);
  lcd.print(speed_m_s, 2);
  lcd.print(" m/s");
  lcd.setCursor(0, 1);
  lcd.print("dt:");
  lcd.setCursor(4, 1);
  lcd.print((float)change_in_time / 1000.0f, 1); // ms
  lcd.print(" ms");

  // reset timing vars if needed (we already handled waiting flag)
  previous_time = time_now = 0;
}