#ifndef RUNTIME_CONFIG_H
#define RUNTIME_CONFIG_H

#include <Arduino.h>
#include <Preferences.h>
#include <stdint.h>
#include <string.h>

// GPIO assignments deliberately live only in Config.h. All editable values are
// bounded integers; one schema drives validation, defaults and the web form.
#define ROBOT_CONFIG_FIELDS(X) \
 X(MOTOR_ENABLED,"motor_enabled","เปิดใช้งานมอเตอร์","motor",0,1,1,"bool","ปิดแล้วทุกคำสั่งขับเคลื่อนจะถูกปฏิเสธ") \
 X(MOTOR_TIMEOUT,"motor_timeout_ms","เวลาหยุดเมื่อขาดคำสั่ง","motor",300,5000,2000,"ms","Watchdog เปิดเสมอ ขณะกดปุ่มเว็บจะส่งคำสั่งซ้ำทุก 150 ms") \
 X(MOTOR_LEFT_REVERSE,"motor_left_reverse","กลับทิศล้อซ้าย","motor",0,1,0,"bool","เปลี่ยนทิศทางทางซอฟต์แวร์โดยไม่เปลี่ยน pin") \
 X(MOTOR_RIGHT_REVERSE,"motor_right_reverse","กลับทิศล้อขวา","motor",0,1,0,"bool","ใช้แก้ล้อหมุนกลับด้าน") \
 X(SERVO_ENABLED,"servo_enabled","เปิดใช้งานเซอร์โว","servo",0,1,1,"bool","ปิดแล้วปล่อย PWM ทั้งสามตัว เซอร์โวจะไม่ยึดตำแหน่ง") \
 X(SERVO_MIN_US_CFG,"servo_min_us","พัลส์ต่ำสุด","servo",500,1500,500,"us","เปลี่ยนแล้วต่อ PWM ใหม่ ใช้ค่าที่เซอร์โวรองรับ") \
 X(SERVO_MAX_US_CFG,"servo_max_us","พัลส์สูงสุด","servo",1501,2500,2400,"us","ต้องมากกว่าพัลส์ต่ำสุด") \
 X(MOTION_SPEED,"motion_speed_pct","ความเร็วท่าทาง","servo",25,200,100,"%","100 คือเวลาเดิม ปรับเวลาของ keyframe") \
 X(HEAD_MIN,"head_min","มุมต่ำสุด","head",0,179,0,"deg","ขอบเขตมุมจริงหลังกลับทิศและชดเชย") \
 X(HEAD_MAX,"head_max","มุมสูงสุด","head",1,180,180,"deg","ทุกท่าถูกจำกัดให้อยู่ในช่วงนี้") \
 X(HEAD_CENTER,"head_center","มุมพัก","head",0,180,90,"deg","มุมคำสั่งก่อนกลับทิศและชดเชย") \
 X(HEAD_TRIM,"head_trim","ชดเชยมุม","head",-30,30,0,"deg","บวกเพิ่มมุมจริงหลังกลับทิศ") \
 X(HEAD_REVERSE,"head_reverse","กลับทิศ","head",0,1,0,"bool","มุมจริง = 180 - มุมคำสั่ง แล้วบวกชดเชย") \
 X(LEFT_MIN,"left_min","มุมต่ำสุด","arm_left",0,179,0,"deg","ขอบเขตมุมจริงของแขนซ้าย") \
 X(LEFT_MAX,"left_max","มุมสูงสุด","arm_left",1,180,180,"deg","จำกัดทุกท่า รวมการเต้นตามเสียง") \
 X(LEFT_CENTER,"left_center","มุมพัก","arm_left",0,180,30,"deg","มุมคำสั่งก่อนกลับทิศและชดเชย") \
 X(LEFT_TRIM,"left_trim","ชดเชยมุม","arm_left",-30,30,0,"deg","ชดเชยตำแหน่งแขนซ้าย") \
 X(LEFT_REVERSE,"left_reverse","กลับทิศ","arm_left",0,1,0,"bool","ใช้กับเซอร์โวที่ติดตั้งกลับด้าน") \
 X(RIGHT_MIN,"right_min","มุมต่ำสุด","arm_right",0,179,0,"deg","ขอบเขตมุมจริงของแขนขวา") \
 X(RIGHT_MAX,"right_max","มุมสูงสุด","arm_right",1,180,180,"deg","จำกัดทุกท่า รวมการเต้นตามเสียง") \
 X(RIGHT_CENTER,"right_center","มุมพัก","arm_right",0,180,150,"deg","มุมคำสั่งก่อนกลับทิศและชดเชย") \
 X(RIGHT_TRIM,"right_trim","ชดเชยมุม","arm_right",-30,30,0,"deg","ชดเชยตำแหน่งแขนขวา") \
 X(RIGHT_REVERSE,"right_reverse","กลับทิศ","arm_right",0,1,0,"bool","ใช้กับเซอร์โวที่ติดตั้งกลับด้าน") \
 X(MIC_ENABLED,"mic_enabled","เปิดไมโครโฟน","mic",0,1,1,"bool","ปิดแล้วไม่ตรวจจับ beat หรือ clap") \
 X(MIC_INTERVAL,"mic_interval_ms","รอบอ่านไมค์","mic",5,100,20,"ms","แยกจากรอบวัดระยะ ไม่รอ ultrasonic") \
 X(CLAP_THRESHOLD,"clap_threshold","เกณฑ์ปรบมือ","mic",10,4095,140,"ADC","เทียบกับระดับเสียงที่หัก baseline และกรองแล้ว") \
 X(BEAT_COOLDOWN,"beat_cooldown_ms","ระยะห่างระหว่าง beat","mic",100,2000,280,"ms","ป้องกันตรวจจังหวะเดียวซ้ำหลายครั้ง") \
 X(MIC_SMOOTH,"mic_smooth_pct","น้ำหนักข้อมูลใหม่","mic",1,100,63,"%","ค่าน้อยกรองนุ่ม ค่ามากตอบสนองเร็ว") \
 X(SONAR_ENABLED,"sonar_enabled","เปิด ultrasonic","sonar",0,1,1,"bool","ปิดแล้วระยะเป็นไม่มีข้อมูล") \
 X(SONAR_INTERVAL,"sonar_interval_ms","รอบวัดระยะ","sonar",60,1000,60,"ms","Interactive ควรใช้ 60–200 ms เพื่อเดินต่อเนื่อง ค่าสูงอาจขาดข้อมูลจน watchdog หยุด") \
 X(SONAR_TIMEOUT,"sonar_timeout_us","เวลารอ echo สูงสุด","sonar",2000,30000,22000,"us","จับขอบสัญญาณด้วย interrupt โดยไม่ใช้ pulseIn") \
 X(DISTANCE_OFFSET,"distance_offset_mm","ชดเชยระยะ","sonar",-200,200,0,"mm","ชดเชยตำแหน่งติดตั้งเซนเซอร์") \
 X(AUDIO_ENABLED,"audio_enabled","เปิดเสียง buzzer","audio",0,1,1,"bool","ปิดแล้วหยุดเพลงและสั่งขาเสียง LOW") \
 X(AUDIO_VOLUME,"audio_volume_pct","ระดับ PWM เสียง","audio",0,100,100,"%","สัดส่วน duty ไม่ใช่ระดับ dB; 0 คือเงียบ") \
 X(AUDIO_TEMPO,"audio_tempo_pct","ความเร็วเพลง","audio",50,200,100,"%","ใช้กับโน้ตถัดไป 100 คือ BPM เดิม") \
 X(OLED_ENABLED,"oled_enabled","เปิดจอ OLED","oled",0,1,1,"bool","ปิดจอได้โดยไม่เปลี่ยน I2C หรือ address") \
 X(OLED_CONTRAST,"oled_contrast","ความเข้มจอ","oled",0,255,127,"level","ส่งคำสั่ง contrast ไปยัง SH1106") \
 X(OLED_ANIM,"oled_anim_ms","รอบแอนิเมชัน","oled",100,1000,300,"ms","ค่าสูงลดภาระการส่งข้อมูล I2C") \
 X(PROX_CLOSE,"proximity_close_cm","ระยะใกล้เกินไป","behavior",5,100,18,"cm","โหมด Interactive แสดงตกใจในช่วงนี้") \
 X(PROX_GREETING,"proximity_greeting_cm","ระยะทักทายสูงสุด","behavior",10,200,45,"cm","ต้องมากกว่าระยะใกล้เกินไป") \
 X(PROX_COOLDOWN,"proximity_cooldown_ms","พักระหว่างการทักทาย","behavior",500,10000,1500,"ms","ทักทายครั้งเดียวต่อการพบ ต้องออกห่างต่อเนื่อง 3 วินาทีและเว้นอย่างน้อย 10 วินาทีก่อนทักทายใหม่") \
 X(TELEMETRY_INTERVAL,"telemetry_interval_ms","รอบข้อมูลหน้าเว็บ","system",250,5000,500,"ms","เรียกครั้งถัดไปเมื่อครั้งก่อนเสร็จ หยุดเมื่อซ่อนแท็บ") \
 X(AUTO_STOP,"auto_stop_cm","ระยะหยุดอัตโนมัติ","autonomy",15,100,25,"cm","หยุดทันทีเมื่อใกล้กว่าหรือเท่าค่านี้ ห้ามถอยอัตโนมัติ") \
 X(AUTO_CLEAR,"auto_clear_cm","ระยะโล่งสำหรับเดิน","autonomy",30,250,60,"cm","เริ่มที่ค่านี้หรือระยะหยุดบวก 13 cm แล้วแต่ว่าค่าใดมากกว่า หยุดเดินหน้าเมื่อระยะลดลงอีก 8 cm") \
 X(FOLLOW_DISTANCE,"follow_distance_cm","ระยะตามเป้าหมาย","autonomy",30,150,65,"cm","เริ่มตามเมื่อไกลกว่านี้อีก 5 cm หยุดที่ระยะนี้ ใช้มัธยฐาน 3 ค่าเพื่อลดการสั่งเดิน–หยุดสลับ") \
 X(FOLLOW_MAX,"follow_max_cm","ระยะติดตามสูงสุด","autonomy",60,300,180,"cm","เกินระยะนี้จะหยุด ปลดเป้าหมาย แล้วกลับสำรวจอัตโนมัติ") \
 X(TARGET_DELTA,"target_delta_cm","การเปลี่ยนระยะขั้นต่ำ","autonomy",3,40,5,"cm","ระยะเปลี่ยนขณะหุ่นอยู่นิ่ง = ผู้สมัครเป้าหมายเคลื่อนที่ ไม่ยืนยันว่าเป็นคน") \
 X(AUTO_PULSE,"auto_pulse_ms","เวลาคุ้มครองการเดินขั้นต่ำ","autonomy",60,300,120,"ms","เดินต่อด้วย echo ใหม่เท่านั้น ระยะ watchdog ปรับตามรอบวัดเป็น 150–600 ms และไม่เกิน timeout มอเตอร์") \
 X(AUTO_OBSERVE,"auto_observe_ms","จังหวะสังเกตและพักหลบ","autonomy",300,3000,400,"ms","สำรวจต่อเนื่องอย่างน้อย 3 วินาทีหรือ 4 เท่าค่านี้ก่อนพักสังเกต ใช้ค่านี้พักก่อนลองหลบอีกด้าน") \
 X(AUTO_TURN,"auto_turn_ms","เวลาเลี้ยวสแกนแต่ละช่วง","autonomy",60,250,140,"ms","เลี้ยวเป็นช่วงแล้ววัดใหม่ หากไม่พบทางจะลองสลับด้าน ไม่มีเซนเซอร์ด้านข้าง") \
 X(AUTO_TURN_LIMIT,"auto_turn_limit","จำนวนเลี้ยวต่อชุด","autonomy",1,8,4,"times","ครบชุดแล้วยังไม่พบทางจะพักตามจังหวะสังเกต ก่อนลองอีกด้านเอง ไม่ต้องเริ่มโหมดใหม่")

enum ConfigField {
#define CONFIG_ENUM(id, key, label, group, lo, hi, def, unit, hint) id,
  ROBOT_CONFIG_FIELDS(CONFIG_ENUM)
#undef CONFIG_ENUM
  CONFIG_FIELD_COUNT
};
struct ConfigSpec {
  const char *key, *label, *group;
  int32_t minimum, maximum, defaultValue;
  const char *unit, *hint;
};
const ConfigSpec CONFIG_SCHEMA[] = {
#define CONFIG_SPEC(id, key, label, group, lo, hi, def, unit, hint) {key,label,group,lo,hi,def,unit,hint},
  ROBOT_CONFIG_FIELDS(CONFIG_SPEC)
#undef CONFIG_SPEC
};
#undef ROBOT_CONFIG_FIELDS

struct RobotSettings {
  int32_t values[CONFIG_FIELD_COUNT];
  RobotSettings() { resetDefaults(); }
  void resetDefaults() {
    for (size_t i = 0; i < CONFIG_FIELD_COUNT; ++i) values[i] = CONFIG_SCHEMA[i].defaultValue;
  }
  int32_t get(ConfigField field) const { return values[field]; }
  bool validate(const char*& error) const {
    for (size_t i = 0; i < CONFIG_FIELD_COUNT; ++i) {
      if (values[i] < CONFIG_SCHEMA[i].minimum || values[i] > CONFIG_SCHEMA[i].maximum) {
        error = CONFIG_SCHEMA[i].key; return false;
      }
    }
    const ConfigField axes[] = {HEAD_MIN, LEFT_MIN, RIGHT_MIN};
    for (ConfigField base : axes) {
      int lo = values[base], hi = values[base + 1];
      int home = values[base + 4] ? 180 - values[base + 2] : values[base + 2];
      home += values[base + 3];
      if (lo >= hi || home < lo || home > hi) { error = CONFIG_SCHEMA[base].key; return false; }
    }
    if (get(SERVO_MIN_US_CFG) >= get(SERVO_MAX_US_CFG)) { error = "servo_min_us"; return false; }
    if (get(PROX_CLOSE) >= get(PROX_GREETING)) { error = "proximity_close_cm"; return false; }
    if (get(AUTO_STOP) >= get(AUTO_CLEAR)) { error = "auto_clear_cm"; return false; }
    if (get(FOLLOW_DISTANCE) <= get(AUTO_STOP) || get(FOLLOW_DISTANCE) + 5 >= get(FOLLOW_MAX)) { error = "follow_distance_cm"; return false; }
    error = nullptr; return true;
  }
  int servoAngle(ConfigField base, int logical) const {
    int angle = values[base + 4] ? 180 - logical : logical;
    angle += values[base + 3];
    return constrain(angle, values[base], values[base + 1]);
  }
};
extern RobotSettings robotSettings;

inline bool parseConfigInteger(const String& text, int32_t& value) {
  if (text.length() == 0 || text.length() > 11) return false;
  size_t i = text[0] == '-' ? 1 : 0;
  if (i == text.length()) return false;
  int64_t number = 0;
  for (; i < text.length(); ++i) {
    if (text[i] < '0' || text[i] > '9') return false;
    number = number * 10 + text[i] - '0';
    if (number > INT32_MAX + (text[0] == '-' ? 1LL : 0LL)) return false;
  }
  value = static_cast<int32_t>(text[0] == '-' ? -number : number);
  return true;
}

class ConfigStore {
public:
  uint32_t revision() const { return _revision; }
  const char* status() const { return _status; }
  void begin() {
    Preferences prefs;
    if (!prefs.begin("robot-config", false)) { _status = "storage_unavailable"; return; }
    if (prefs.getBytesLength("settings") == 0) { prefs.end(); return; }
    Record record = {};
    const size_t length = prefs.getBytesLength("settings");
    const char* error = nullptr;
    bool ok = false;
    if (length == sizeof(record)) {
      ok = prefs.getBytes("settings", &record, sizeof(record)) == sizeof(record) &&
        record.magic == MAGIC && record.version == VERSION &&
        record.checksum == checksum(record) && record.settings.validate(error);
    } else if (length == sizeof(LegacyRecord)) {
      LegacyRecord old = {};
      if (prefs.getBytes("settings", &old, sizeof(old)) == sizeof(old) &&
          old.magic == MAGIC && old.version == 1 &&
          old.checksum == crcBytes(&old, offsetof(LegacyRecord, checksum))) {
        // Append-only schema: preserve all 42 previously saved values.
        memcpy(record.settings.values, old.values, sizeof(old.values));
        record.revision = old.revision;
        ok = record.settings.validate(error);
        _needsMigration = ok;
      }
    }
    prefs.end();
    if (!ok) { _status = "invalid_saved_config"; return; }
    robotSettings = record.settings;
    _revision = record.revision;
    _status = _needsMigration ? "migrated_v1" : "saved";
  }
  bool save(const RobotSettings& next, const char*& error) {
    if (!next.validate(error)) return false;
    // Keep legacy data readable without an extra write on a no-op save. The
    // API stops autonomous movement before any actual settings change/write.
    if (!memcmp(next.values, robotSettings.values, sizeof(next.values))) return true;
    if (_hasWritten && millis() - _lastWrite < 2000) { error = "save_too_soon"; return false; }
    Record record = {};
    record.magic = MAGIC; record.version = VERSION;
    record.revision = _revision + 1; record.settings = next;
    record.checksum = checksum(record);
    Preferences prefs;
    if (!prefs.begin("robot-config", false)) { error = "storage_unavailable"; return false; }
    bool ok = prefs.putBytes("settings", &record, sizeof(record)) == sizeof(record);
    prefs.end();
    if (!ok) { error = "storage_write_failed"; return false; }
    robotSettings = next;
    _revision = record.revision; _status = "saved";
    _needsMigration = false; _hasWritten = true; _lastWrite = millis();
    return true;
  }
private:
  static constexpr uint32_t MAGIC = 0x52424346;
  static constexpr uint32_t VERSION = 2;
  struct LegacyRecord { uint32_t magic, version, revision; int32_t values[42]; uint32_t checksum; };
  static_assert(TELEMETRY_INTERVAL == 41, "Do not reorder persisted v1 fields");
  struct Record {
    uint32_t magic, version, revision;
    RobotSettings settings;
    uint32_t checksum;
  };
  uint32_t _revision = 0, _lastWrite = 0;
  bool _hasWritten = false, _needsMigration = false;
  const char* _status = "defaults";
  static uint32_t checksum(const Record& record) {
    return crcBytes(&record, offsetof(Record, checksum));
  }
  static uint32_t crcBytes(const void* data, size_t length) {
    // CRC32 over a versioned, fixed-width record; no heap allocations.
    uint32_t crc = 0xFFFFFFFF;
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);
    for (size_t i = 0; i < length; ++i) {
      crc ^= bytes[i];
      for (int bit = 0; bit < 8; ++bit) crc = (crc >> 1) ^ (0xEDB88320 & (0U - (crc & 1)));
    }
    return ~crc;
  }
};
#endif
