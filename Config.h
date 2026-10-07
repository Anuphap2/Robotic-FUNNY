#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// ⚠️ HARDWARE PIN MAPPING (IMMUTABLE - ห้ามเปลี่ยนเด็ดขาดตามที่ต่อสายไว้)
// ============================================================================

// I2C OLED Display (128x64 SH1106)
#define OLED_I2C_ADDR   0x3C
#define PIN_I2C_SDA     21
#define PIN_I2C_SCL     22
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64

// Sensors & Audio
const int PIN_TRIG      = 5;   // Ultrasonic Trigger
const int PIN_ECHO      = 17;  // Ultrasonic Echo
const int PIN_BUZZER    = 25;  // Active/Passive Piezo Buzzer
const int PIN_MIC_AO    = 34;  // Sound Sensor KY-037 Analog Out

// Servo Actuators
const int PIN_SERVO_HEAD  = 18; // Neck Yaw/Head
const int PIN_SERVO_ARM_L = 19; // Left Arm
const int PIN_SERVO_ARM_R = 23; // Right Arm

// Dual DC Motors (L9110S Driver)
const int PIN_A1A = 26; // Motor Left A
const int PIN_A1B = 27; // Motor Left B
const int PIN_B1A = 14; // Motor Right A
const int PIN_B1B = 12; // Motor Right B

// Motor Speed Trim (0 - 100%)
// ชดเชยความเร็วมอเตอร์ซ้าย-ขวา เพื่อแก้ปัญหารถวิ่งไม่ตรง
// - รถเดินหน้าแล้วเลี้ยวเป๋ไปทางซ้าย = ล้อขวาเร็วกว่าล้อซ้าย ให้ลด MOTOR_RIGHT_TRIM_PCT ลง (เช่น 85, 80, 75)
// - รถเดินหน้าแล้วเลี้ยวเป๋ไปทางขวา = ล้อซ้ายเร็วกว่าล้อขวา ให้ลด MOTOR_LEFT_TRIM_PCT ลง (เช่น 85, 80, 75)
#ifndef MOTOR_LEFT_TRIM_PCT
#define MOTOR_LEFT_TRIM_PCT   100  // กำลังล้อซ้าย (%) ค่าปกติ 100
#endif
#ifndef MOTOR_RIGHT_TRIM_PCT
#define MOTOR_RIGHT_TRIM_PCT   100  // กำลังล้อขวา (%) ค่าปกติ 100
#endif

// Speed Profile for Heavy Chassis Robot (ขับเคลื่อน 100% เท่ากัน ป้องกันล้อติดขัดจากน้ำหนักตัวถัง)
#ifndef MOTOR_MIN_SPEED_PCT
#define MOTOR_MIN_SPEED_PCT    80   // ขีดจำกัดความเร็วต่ำสุด (ห้ามต่ำกว่า 80% เพื่อรักษาแรงบิดขับเคลื่อน)
#endif
#ifndef SPEED_ROAM_FORWARD
#define SPEED_ROAM_FORWARD    100   // เดินหน้าสำรวจเต็มกำลัง 100% (มอเตอร์ 100% เท่ากัน ขับเคลื่อนบอดี้หนัก)
#endif
#ifndef SPEED_ROAM_TURN
#define SPEED_ROAM_TURN       100   // เลี้ยวหลบสิ่งกีดขวาง 100% (Skid-steer หมุนตัวได้อย่างคล่องตัว ไม่ติดขัด)
#endif
#ifndef SPEED_REVERSE_PCT
#define SPEED_REVERSE_PCT      90   // ถอยหลังเว้นระยะเมื่อจ่อกำแพงใกล้มาก
#endif
#ifndef SPEED_FOLLOW_SLOW
#define SPEED_FOLLOW_SLOW     100
#endif
#ifndef SPEED_FOLLOW_MID
#define SPEED_FOLLOW_MID      100
#endif
#ifndef SPEED_FOLLOW_FAST
#define SPEED_FOLLOW_FAST     100
#endif
#ifndef SPEED_TURN_SEARCH
#define SPEED_TURN_SEARCH     100   // หมุนเลี้ยวด้วยแรงบิด 100% เต็มกำลัง
#endif


// ============================================================================
// NETWORK CONFIGURATION
// ============================================================================
const char* const WIFI_SSID     = "Why na";
const char* const WIFI_PASSWORD = "12345678";

// Fallback Access Point (AP) if router not reachable
const char* const AP_SSID       = "Robot-Recreation";
const char* const AP_PASSWORD   = "12345678";

// Servo pulse width range (microseconds)
#define SERVO_MIN_US 500
#define SERVO_MAX_US 2400

#endif // CONFIG_H
