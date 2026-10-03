# 🤖 Smart Recreational Robot (หุ่นยนต์สันทนาการอัจฉริยะ)

สถาปัตยกรรมซอฟต์แวร์แบบ **Computer Science Modular & Non-Blocking State Machine** สำหรับ ESP32 ออกแบบเพื่อการใช้งานจริงในกิจกรรมสันทนาการ ค่าย กิจกรรมกลุ่ม และการศึกษา

---

## 📌 1. ฮาร์ดแวร์และการต่อสายพิน (Immutable Hardware Mapping)
> **หมายเหตุสำคัญ:** พินทั้งหมดถูกล็อคตามการต่อวงจรเดิม 100% ห้ามแก้ไขเด็ดขาด

| อุปกรณ์ | หน้าที่ | พิน ESP32 / รายละเอียด |
|---|---|---|
| **OLED Display** | แสดงสีหน้าอารมณ์ 128x64 | I2C (SDA = 21, SCL = 22), Addr `0x3C` |
| **Buzzer** | ลำโพงเล่นเพลงสันทนาการ | `PIN 25` |
| **Microphone (KY-037)** | ตรวจจับเสียงปรบมือ/จังหวะเพลง | `PIN 34` (Analog ADC) |
| **Ultrasonic (HC-SR04/05)** | วัดระยะทางวัตถุ/คน | `TRIG = 5`, `ECHO = 17` |
| **Head Servo** | หมุนคอ/หันซ้าย-ขวา | `PIN 18` |
| **Left Arm Servo** | แขนซ้าย | `PIN 19` |
| **Right Arm Servo** | แขนขวา | `PIN 23` |
| **Motor Driver (L9110S)** | ขับเคลื่อนล้อซ้าย-ขวา | `PIN 26` (A1A), `PIN 27` (A1B), `PIN 14` (B1A), `PIN 12` (B1B) |

---

## 🏗️ 2. สถาปัตยกรรมระบบ (CS Architecture)

ระบบถูกแยกเป็นโมดูลตามหลัก **Single Responsibility Principle (SRP)** และ **Open-Closed Principle (OCP)**:
- [Config.h](file:///c:/Users/KPG_G/OneDrive/Documents/Arduino/sketch_oct2a/Config.h) : เก็บการตั้งค่าพินฮาร์ดแวร์และ Wi-Fi
- [SongSystem.h](file:///c:/Users/KPG_G/OneDrive/Documents/Arduino/sketch_oct2a/SongSystem.h) : ตู้เพลงสันทนาการ (Jukebox) พร้อม **Non-blocking Melodic Sequencer** เก็บโน้ตไว้ใน PROGMEM flash
- [FaceSystem.h](file:///c:/Users/KPG_G/OneDrive/Documents/Arduino/sketch_oct2a/FaceSystem.h) : เอนจินสีหน้า OLED แสดงผลแบบ Vector พร้อมระบบกระพริบตาอัตโนมัติ (Auto Natural Blink)
- [MotionSystem.h](file:///c:/Users/KPG_G/OneDrive/Documents/Arduino/sketch_oct2a/MotionSystem.h) : คอนโทรลเลอร์ควบคุมเซอร์โว 3 ตัวแบบ Keyframe โดยไม่บล็อกลูป
- [MotorController.h](file:///c:/Users/KPG_G/OneDrive/Documents/Arduino/sketch_oct2a/MotorController.h) : ไดรเวอร์มอเตอร์พร้อมระบบความปลอดภัย Auto-stop Watchdog เมื่อสัญญาณขาดหาย
- [SensorManager.h](file:///c:/Users/KPG_G/OneDrive/Documents/Arduino/sketch_oct2a/SensorManager.h) : ตัวประมวลผลเซนเซอร์ระยะทาง และไมค์พร้อม Clap/Beat Detector จับจังหวะสันทนาการ
- [RecreationEngine.h](file:///c:/Users/KPG_G/OneDrive/Documents/Arduino/sketch_oct2a/RecreationEngine.h) : สมองประสานงานโหมดสันทนาการ (Manual, Party เต้นตามเสียง, Interactive ทักทายคน, Showcase เต้นประกอบเพลง)
- [WebUI.h](file:///c:/Users/KPG_G/OneDrive/Documents/Arduino/sketch_oct2a/WebUI.h) : Cyber-Recreation Web Dashboard ที่โหลดแคตตาล็อกแบบ **Data-Driven (Zero Hardcoding)**
- [sketch_oct2a.ino](file:///c:/Users/KPG_G/OneDrive/Documents/Arduino/sketch_oct2a/sketch_oct2a.ino) : Main Controller & HTTP API Dispatcher

---

## 🎶 3. วิธีเพิ่มเพลงใหม่ (Add New Song)
ไม่ต้องแก้ HTML หรือเขียน `if-else` เพิ่มในโค้ดหลักเลย! เพียงทำ 2 ขั้นตอนใน [SongSystem.h](file:///c:/Users/KPG_G/OneDrive/Documents/Arduino/sketch_oct2a/SongSystem.h):

### 1) กำหนดตัวโน้ตเพลงด้วยสัญกรณ์ดนตรีสากล (Beats & Dynamics)
```cpp
const Note SONG_MY_SONG[] PROGMEM = {
  {NOTE_C5, N_QUARTER, DYN_MF}, // ตัวดำ ปานกลาง
  {NOTE_E5, N_QUARTER, DYN_MP}, // ตัวดำ นุ่มนวล
  {NOTE_G5, N_HALF,    DYN_F}   // ตัวขาว ก้องกังวาน (Forte)
};
```
*(หากใส่แค่ 2 ค่า `{NOTE_C5, N_QUARTER}` ระบบจะคำนวณไดนามิกอัตโนมัติให้ทันที)*

### 2) ลงทะเบียนใน `SONG_REGISTRY` พร้อมระบุ BPM
```cpp
const SongItem SONG_REGISTRY[] = {
  // ... เพลงเดิม ...
  {"mysong", "ชื่อเพลงภาษาไทย", "recreation", "🎵", 125, 0.82f, SONG_MY_SONG, sizeof(SONG_MY_SONG) / sizeof(Note)}
};
```
👉 **เมื่ออัพโหลดโค้ด ปุ่มเพลงจะปรากฏบน Web Dashboard ทันทีโดยอัตโนมัติ! ปรับความเร็วเพลงได้ง่ายๆ แค่แก้ตัวเลข BPM**

### 🎼 4 สไตล์เสียงเครื่องดนตรี (Instrument Timbres & Envelopes):
สามารถเลือกเปลี่ยนสไตล์เสียงบนหน้าเว็บได้ทันที 4 แบบ:
1. 🔔 **Music Box (กล่องดนตรีใสกังวาน):** คลื่นเสียงสั่นสะเทือนแบบกระดิ่งแก้ว ค่อยๆ คลายตัวนุ่มนวล ไพเราะที่สุด
2. 🎻 **Vibrato (เสียงร้องมีลูกคอ):** โน้ตที่ลากยาวจะมีลูกคอพริ้วไหวแบบเสียงร้องมนุษย์ (LFO 5.5 Hz)
3. 🎹 **Acoustic (เปียโน/คาลิมบา):** หัวโน้ตเคาะชัดเจน แล้วมีหางเสียง sustain อบอุ่น
4. 👾 **Chiptune (เกมบอย 8-บิต):** เสียงแนวเกมเรโทรคลาสสิก คมชัดและหนักแน่นด้วยไดนามิกของแต่ละจังหวะ

---

## 😊 4. วิธีเพิ่มหน้าตาใหม่ (Add New Face)
ทำ 2 ขั้นตอนใน [FaceSystem.h](file:///c:/Users/KPG_G/OneDrive/Documents/Arduino/sketch_oct2a/FaceSystem.h):

### 1) เขียนฟังก์ชันวาดกราฟิก OLED
```cpp
inline void drawFaceCustom(Adafruit_SH1106G& d, int phase) {
  // วาดตาซ้าย-ขวาบนจอ 128x64
  d.fillCircle(36, 32, 14, SH110X_WHITE);
  d.fillCircle(92, 32, 14, SH110X_WHITE);
}
```

### 2) ลงทะเบียนใน `FACE_REGISTRY`
```cpp
const FaceItem FACE_REGISTRY[] = {
  // ... หน้าเดิม ...
  {"custom", "ตาแบบใหม่", "🤩", drawFaceCustom}
};
```
👉 **หน้าตานี้จะโผล่บนแดชบอร์ดให้กดเล่นได้ทันที!**

---

## 💃 5. วิธีเพิ่มท่าทางเซอร์โวใหม่ (Add New Motion)
ทำใน [MotionSystem.h](file:///c:/Users/KPG_G/OneDrive/Documents/Arduino/sketch_oct2a/MotionSystem.h):
```cpp
const ServoKeyframe FRAMES_MY_MOVE[] = {
  {90, 150, 30, 250}, // Head, ArmL, ArmR, HoldMs
  {90, 30, 150, 250}
};

// ลงทะเบียนใน MOTION_REGISTRY
{"mymove", "ท่าเต้นใหม่", "🕺", FRAMES_MY_MOVE, sizeof(FRAMES_MY_MOVE)/sizeof(ServoKeyframe), false}
```

### 🌟 ท่าพิเศษไวรัล: ท่า SIX-SEVEN (6-7)
- **สไตล์ท่าทาง:** มือสองข้างหงายขึ้นแล้วขยับสลับขึ้นลงคล้ายการชั่งน้ำหนัก (Weighing Scale) พร้อมส่ายหัวเอียงคอสุดกวน เข้าจังหวะ Drill บีท
- **ระบบประสานงาน:** เมื่อกด Showcase "ท่าไวรัล SIX-SEVEN" ระบบจะเล่นเพลงตีม 6-7 พร้อมแสดงสีหน้ามีมตัวเลข 6 และ 7 สลับเด้งบนจอ OLED และเต้นวนลูปจนจบเพลง!

---

## ⏪ 6. ระบบ Version Control ภายในเครื่อง (Local Git)
เก็บประวัติไว้ในโฟลเดอร์นี้เรียบร้อยแล้ว ไม่มีการ push ขึ้นภายนอก สามารถย้อนกลับได้เสมอ:

- **ดูประวัติ commit:**
  ```powershell
  git log --oneline
  ```
- **ดูโค้ดเวอร์ชันแรกเริ่ม (Baseline):**
  ```powershell
  git checkout 8c53f9b
  ```
- **กลับมาเวอร์ชันล่าสุด:**
  ```powershell
  git checkout master
  ```
- **เซฟเวอร์ชันใหม่ของคุณเมื่อเพิ่มเพลง/หน้าตา:**
  ```powershell
  git add .
  git commit -m "feat: added new song and face"
  ```
