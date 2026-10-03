#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <ESP32Servo.h>

#include "webpage.h"

// ================= ตั้งค่า Wi-Fi =================
const char* ssid     = "Why na";
const char* password = "12345678";

WebServer server(80);

// ================= กำหนดขาพินอุปกรณ์ =================
#define i2c_Address 0x3C
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);

const int PIN_TRIG   = 5;
const int PIN_ECHO   = 17;
const int PIN_BUZZER = 25;
const int PIN_MIC_AO = 34;

const int PIN_SERVO_HEAD  = 18;
const int PIN_SERVO_ARM_L = 19;
const int PIN_SERVO_ARM_R = 23;

Servo servoHead;
Servo servoArmL;
Servo servoArmR;

const int PIN_A1A = 26; // มอเตอร์ซ้าย A
const int PIN_A1B = 27; // มอเตอร์ซ้าย B
const int PIN_B1A = 14; // มอเตอร์ขวา A
const int PIN_B1B = 12; // มอเตอร์ขวา B

// *** include robot_content.h ตรงนี้ เพื่อให้มองเห็น PIN_BUZZER และ Servos แน่นอน 100% ***
#include "robot_content.h"

// ================= ตัวแปรและสถานะระบบ =================
int micBaseline = 2000;
int currentVol = 0;
float currentDistance = 0.0;
unsigned long lastSensorRead = 0;

// ================= ฟังก์ชันควบคุมล้อ L9110S =================
void motorStop() {
  digitalWrite(PIN_A1A, LOW); digitalWrite(PIN_A1B, LOW);
  digitalWrite(PIN_B1A, LOW); digitalWrite(PIN_B1B, LOW);
}
void moveForward() {
  digitalWrite(PIN_A1A, HIGH); digitalWrite(PIN_A1B, LOW);
  digitalWrite(PIN_B1A, HIGH); digitalWrite(PIN_B1B, LOW);
}
void moveBackward() {
  digitalWrite(PIN_A1A, LOW); digitalWrite(PIN_A1B, HIGH);
  digitalWrite(PIN_B1A, LOW); digitalWrite(PIN_B1B, HIGH);
}
void turnLeft() {
  digitalWrite(PIN_A1A, LOW); digitalWrite(PIN_A1B, HIGH);
  digitalWrite(PIN_B1A, HIGH); digitalWrite(PIN_B1B, LOW);
}
void turnRight() {
  digitalWrite(PIN_A1A, HIGH); digitalWrite(PIN_A1B, LOW);
  digitalWrite(PIN_B1A, LOW); digitalWrite(PIN_B1B, HIGH);
}

// ================= ฟังก์ชันหน้าตา OLED =================
void drawEyes(const String& expr) {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  if (expr == "happy") {
    display.setTextSize(3);
    display.setCursor(26, 20);
    display.print("^  ^");
  } else if (expr == "surprise") {
    display.drawCircle(36, 28, 18, SH110X_WHITE);
    display.drawCircle(92, 28, 18, SH110X_WHITE);
    display.fillCircle(64, 48, 6, SH110X_WHITE);
  } else if (expr == "blink") {
    display.fillRect(20, 30, 32, 5, SH110X_WHITE);
    display.fillRect(76, 30, 32, 5, SH110X_WHITE);
  } else if (expr == "look_left") {
    display.drawCircle(36, 30, 16, SH110X_WHITE);
    display.drawCircle(92, 30, 16, SH110X_WHITE);
    display.fillCircle(28, 30, 7, SH110X_WHITE);
    display.fillCircle(84, 30, 7, SH110X_WHITE);
  } else if (expr == "look_right") {
    display.drawCircle(36, 30, 16, SH110X_WHITE);
    display.drawCircle(92, 30, 16, SH110X_WHITE);
    display.fillCircle(44, 30, 7, SH110X_WHITE);
    display.fillCircle(100, 30, 7, SH110X_WHITE);
  } else {
    display.fillCircle(36, 30, 14, SH110X_WHITE);
    display.fillCircle(92, 30, 14, SH110X_WHITE);
  }

  display.display();
}

// ================= ฟังก์ชันวัดระยะทาง Ultrasonic =================
float getDistance() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long dur = pulseIn(PIN_ECHO, HIGH, 25000);
  if (dur == 0) return -1.0;
  return (dur * 0.0343) / 2.0;
}

// ================= WEB SERVER HANDLERS =================
void handleRoot() {
  server.send(200, "text/html", MAIN_PAGE);
}

void handleTelemetry() {
  String json = "{\"vol\":" + String(currentVol) + ",\"dist\":" + String(currentDistance, 1) + "}";
  server.send(200, "application/json", json);
}

void handleAction() {
  if (!server.hasArg("cmd")) {
    server.send(400, "text/plain", "Missing cmd");
    return;
  }
  String cmd = server.arg("cmd");

  // 1. D-Pad ล้อรถ
  if (cmd == "forward")       moveForward();
  else if (cmd == "backward") moveBackward();
  else if (cmd == "left")     turnLeft();
  else if (cmd == "right")    turnRight();
  else if (cmd == "stop")     motorStop();

  // 2. หน้าตา OLED
  else if (cmd == "normal" || cmd == "blink" || cmd == "happy" ||
           cmd == "surprise" || cmd == "look_left" || cmd == "look_right") {
    drawEyes(cmd);
  }

  // 3. ท่าทาง Servo Registry
  else if (dispatchMotion(cmd)) {
    // รันท่าทางเสร็จสิ้น
  }

  // 4. เพลง Jukebox Registry
  else if (dispatchSong(cmd)) {
    // เล่นเพลงเสร็จสิ้น
  }

  server.send(200, "text/plain", "OK");
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_MIC_AO, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  noTone(PIN_BUZZER);

  pinMode(PIN_A1A, OUTPUT); pinMode(PIN_A1B, OUTPUT);
  pinMode(PIN_B1A, OUTPUT); pinMode(PIN_B1B, OUTPUT);
  motorStop();

  Wire.begin(21, 22);
  display.begin(i2c_Address, true);
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(15, 24);
  display.println("Connecting WiFi...");
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected! IP: " + WiFi.localIP().toString());

  server.on("/", handleRoot);
  server.on("/telemetry", handleTelemetry);
  server.on("/action", handleAction);
  server.begin();

  long sum = 0;
  for (int i = 0; i < 100; i++) {
    sum += analogRead(PIN_MIC_AO);
    delay(5);
  }
  micBaseline = sum / 100;

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  servoHead.setPeriodHertz(50);
  servoArmL.setPeriodHertz(50);
  servoArmR.setPeriodHertz(50);

  servoHead.attach(PIN_SERVO_HEAD, 500, 2400);
  servoArmL.attach(PIN_SERVO_ARM_L, 500, 2400);
  servoArmR.attach(PIN_SERVO_ARM_R, 500, 2400);

  servoHead.write(90);
  servoArmL.write(30);
  servoArmR.write(150);
  drawEyes("normal");

  tone(PIN_BUZZER, 2500); delay(100); noTone(PIN_BUZZER);
}

// ================= LOOP =================
void loop() {
  server.handleClient();

  if (millis() - lastSensorRead >= 80) {
    lastSensorRead = millis();
    int rawAudio = analogRead(PIN_MIC_AO);
    currentVol = abs(rawAudio - micBaseline);
    currentDistance = getDistance();
  }
}