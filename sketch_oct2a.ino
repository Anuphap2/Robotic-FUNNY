#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#include "Config.h"
#include "RuntimeConfig.h"
#include "WebApi.h"
#include "ManualDriveGuard.h"
#include <esp_random.h>
#include <esp_heap_caps.h>
#include <esp_system.h>

RobotSettings robotSettings;
ConfigStore configStore;
const char* const FIRMWARE_VERSION = "2.3.3";
uint32_t maxLoopUs = 0;
uint32_t driveBootId = 0;
ManualDriveGuard manualDriveGuard;
#include "SongSystem.h"
#include "FaceSystem.h"
#include "MotionSystem.h"
#include "MotorController.h"
#include "SensorManager.h"
#include "RecreationEngine.h"
#include "WebUI.h"

// ============================================================================
// SYSTEM INSTANCES (Computer Science Object-Oriented Architecture)
// ============================================================================
RobotWebServer server(80);

FaceEngine        faceEngine;
SongPlayer        songPlayer(PIN_BUZZER);
MotionPlayer      motionPlayer;
MotorController   motorController;
SensorManager     sensorManager;
RecreationEngine  recreationEngine(faceEngine, songPlayer, motionPlayer, motorController, sensorManager);

// ============================================================================
// WEB SERVER API HANDLERS
// ============================================================================

// Serve Modern Single Page Application
void handleRoot() {
  server.send_P(200, "text/html", DASHBOARD_HTML);
}

void sendError(int status, const char* code) {
  server.sendHeader("Cache-Control", "no-store");
  char json[224];
  snprintf(json, sizeof(json), "{\"ok\":false,\"error\":\"%s\",\"uptimeMs\":%lu,\"driveBootId\":%lu}", code, (unsigned long)millis(), (unsigned long)driveBootId);
  server.send(status, "application/json", json);
}

bool allowMutation() {
  if (server.method() == HTTP_POST && (!server.bodyReady() ||
      !server.header("Content-Type").startsWith("application/x-www-form-urlencoded"))) {
    sendError(415, "form_encoding_required"); return false;
  }
  // Custom header cannot be sent by a cross-origin HTML form. No CORS is enabled.
  if (server.header("X-Robot-Request") != "1") { sendError(403, "robot_header_required"); return false; }
  return true;
}

void stopAll() {
  manualDriveGuard.cancelAll(millis());
  motorController.stop(); songPlayer.stop(); motionPlayer.freeze();
  recreationEngine.setMode(MODE_MANUAL, false);
}

void handleTelemetry() {
  char json[1536];
  const auto motorSignal = motorController.signalStatus();
  int length = snprintf(json, sizeof(json),
    "{\"vol\":%d,\"peak\":%d,\"dist\":%.1f,\"distanceValid\":%s,\"distanceAgeMs\":%lu,"
    "\"baseline\":%d,\"calibrating\":%s,\"mode\":\"%s\",\"dir\":\"%s\",\"face\":\"%s\","
    "\"timbre\":\"%s\",\"isPlaying\":%s,\"heapFree\":%lu,\"heapMin\":%lu,\"heapLargest\":%lu,"
    "\"uptimeMs\":%lu,\"driveBootId\":%lu,\"maxLoopUs\":%lu,\"oledReady\":%s,\"servoReady\":%s,\"audioReady\":%s,\"motorReady\":%s,"
    "\"rssi\":%d,\"networkMode\":\"%s\",\"ip\":\"%s\",\"version\":\"%s\",\"revision\":%lu,\"pollMs\":%ld,\"interactive\":\"%s\",\"targetClass\":\"%s\",\"decision\":\"%s\",\"behavior\":\"%s\",\"targetLocked\":%s,\"movingVotes\":%u,"
    "\"motorEnabled\":%s,\"servoEnabled\":%s,\"headServoReady\":%s,\"leftServoReady\":%s,\"rightServoReady\":%s,\"motorInitError\":\"%s\",\"resetReason\":%d,"
    "\"motorGPIO\":%u,\"motorPending\":%s,\"motorCommandAgeMs\":%lu,\"motorLeaseMs\":%lu}",
    sensorManager.volume(), sensorManager.peakVolume(), sensorManager.distance(),
    sensorManager.distanceValid() ? "true" : "false", (unsigned long)sensorManager.distanceAge(),
    sensorManager.baseline(), sensorManager.calibrating() ? "true" : "false",
    recreationEngine.modeString(), motorController.directionString(), faceEngine.currentFace().id,
    songPlayer.timbreName(), songPlayer.isPlaying() ? "true" : "false",
    (unsigned long)ESP.getFreeHeap(), (unsigned long)ESP.getMinFreeHeap(),
    (unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
    (unsigned long)millis(), (unsigned long)driveBootId, (unsigned long)maxLoopUs,
    faceEngine.ready() ? "true" : "false", motionPlayer.ready() ? "true" : "false", songPlayer.hardwareOK() ? "true" : "false",
    motorController.ready() ? "true" : "false",
    WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0,
    WiFi.status() == WL_CONNECTED ? "station" : "ap",
    (WiFi.status() == WL_CONNECTED ? WiFi.localIP() : WiFi.softAPIP()).toString().c_str(),
    FIRMWARE_VERSION, (unsigned long)configStore.revision(), (long)robotSettings.get(TELEMETRY_INTERVAL),
    recreationEngine.interactiveString(), recreationEngine.decisionClass(), recreationEngine.decisionReason(),
    recreationEngine.behaviorString(), recreationEngine.targetLocked() ? "true" : "false",
    static_cast<unsigned int>(recreationEngine.movingVotes()),
    robotSettings.get(MOTOR_ENABLED) ? "true" : "false", robotSettings.get(SERVO_ENABLED) ? "true" : "false",
    motionPlayer.headReady() ? "true" : "false", motionPlayer.leftReady() ? "true" : "false", motionPlayer.rightReady() ? "true" : "false",
    motorController.initError(), static_cast<int>(esp_reset_reason()),
    static_cast<unsigned int>(motorSignal.gpioLevels), motorSignal.pending ? "true" : "false",
    (unsigned long)motorSignal.commandAgeMs, (unsigned long)motorSignal.leaseMs);
  if (length < 0 || static_cast<size_t>(length) >= sizeof(json)) { sendError(500, "telemetry_too_large"); return; }
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json);
}

void handleCatalog() {
  JsonResponse out(server);
  out.raw("{\"songs\":[");
  for (size_t i = 0; i < SONG_REGISTRY_COUNT; ++i) {
    if (i) out.raw(",");
    const auto& song = SONG_REGISTRY[i];
    out.raw("{"); out.key("id"); out.quoted(song.id);
    out.raw(","); out.key("title"); out.quoted(song.title);
    out.raw(","); out.key("cat"); out.quoted(song.category);
    out.raw(","); out.key("icon"); out.quoted(song.icon); out.raw("}");
  }
  out.raw("],\"faces\":[");
  for (size_t i = 0; i < FACE_REGISTRY_COUNT; ++i) {
    if (i) out.raw(",");
    const auto& face = FACE_REGISTRY[i];
    out.raw("{"); out.key("id"); out.quoted(face.id);
    out.raw(","); out.key("title"); out.quoted(face.title);
    out.raw(","); out.key("icon"); out.quoted(face.icon); out.raw("}");
  }
  out.raw("],\"motions\":[");
  for (size_t i = 0; i < MOTION_REGISTRY_COUNT; ++i) {
    if (i) out.raw(",");
    const auto& motion = MOTION_REGISTRY[i];
    out.raw("{"); out.key("id"); out.quoted(motion.id);
    out.raw(","); out.key("title"); out.quoted(motion.title);
    out.raw(","); out.key("icon"); out.quoted(motion.icon); out.raw("}");
  }
  out.raw("]}"); out.finish();
}

void handleGetConfig() {
  JsonResponse out(server);
  out.raw("{"); out.key("revision"); out.number(configStore.revision());
  out.raw(","); out.key("schemaVersion"); out.number(2);
  out.raw(","); out.key("version"); out.quoted(FIRMWARE_VERSION);
  out.raw(","); out.key("storage"); out.quoted(configStore.status());
  out.raw(",\"values\":{");
  for (size_t i = 0; i < CONFIG_FIELD_COUNT; ++i) {
    if (i) out.raw(",");
    out.key(CONFIG_SCHEMA[i].key); out.number(robotSettings.values[i]);
  }
  out.raw("},\"schema\":[");
  for (size_t i = 0; i < CONFIG_FIELD_COUNT; ++i) {
    if (i) out.raw(",");
    const auto& spec = CONFIG_SCHEMA[i];
    out.raw("{"); out.key("key"); out.quoted(spec.key);
    out.raw(","); out.key("label"); out.quoted(spec.label);
    out.raw(","); out.key("group"); out.quoted(spec.group);
    out.raw(","); out.key("min"); out.number(spec.minimum);
    out.raw(","); out.key("max"); out.number(spec.maximum);
    out.raw(","); out.key("default"); out.number(spec.defaultValue);
    out.raw(","); out.key("unit"); out.quoted(spec.unit);
    out.raw(","); out.key("hint"); out.quoted(spec.hint); out.raw("}");
  }
  const char* names[] = {"OLED SDA", "OLED SCL", "Ultrasonic TRIG", "Ultrasonic ECHO", "Buzzer", "Microphone AO", "Head servo", "Left arm servo", "Right arm servo", "Motor A1A", "Motor A1B", "Motor B1A", "Motor B1B"};
  const int pins[] = {PIN_I2C_SDA, PIN_I2C_SCL, PIN_TRIG, PIN_ECHO, PIN_BUZZER, PIN_MIC_AO, PIN_SERVO_HEAD, PIN_SERVO_ARM_L, PIN_SERVO_ARM_R, PIN_A1A, PIN_A1B, PIN_B1A, PIN_B1B};
  out.raw("],\"pins\":{");
  for (size_t i = 0; i < sizeof(pins)/sizeof(pins[0]); ++i) {
    if (i) out.raw(","); out.key(names[i]); out.number(pins[i]);
  }
  out.raw("},"); out.key("oledAddress"); out.number(OLED_I2C_ADDR);
  out.raw("}"); out.finish();
}

void handleSaveConfig() {
  if (!allowMutation()) return;
  if (server.args() > CONFIG_FIELD_COUNT + 1 || server.header("Content-Length").toInt() > 2048) {
    sendError(413, "config_request_too_large"); return;
  }
  int32_t revision = -1;
  if (!server.hasArg("revision") || !parseConfigInteger(server.arg("revision"), revision) || revision < 0) {
    sendError(400, "revision_required"); return;
  }
  if ((uint32_t)revision != configStore.revision()) { sendError(409, "config_changed_reload"); return; }
  RobotSettings next = robotSettings;
  bool seen[CONFIG_FIELD_COUNT] = {};
  bool revisionSeen = false;
  for (int arg = 0; arg < server.args(); ++arg) {
    String key = server.argName(arg);
    if (key == "revision") {
      if (revisionSeen) { sendError(400, "duplicate_revision"); return; }
      revisionSeen = true; continue;
    }
    size_t index = 0;
    while (index < CONFIG_FIELD_COUNT && key != CONFIG_SCHEMA[index].key) ++index;
    if (index == CONFIG_FIELD_COUNT) { sendError(400, "unknown_config_field"); return; }
    if (seen[index] || !parseConfigInteger(server.arg(arg), next.values[index])) {
      sendError(400, CONFIG_SCHEMA[index].key); return;
    }
    seen[index] = true;
  }
  const char* error = nullptr;
  if (!next.validate(error)) { sendError(422, error); return; }
  bool changed = memcmp(next.values, robotSettings.values, sizeof(next.values)) != 0;
  // Invalid requests have no side effects. Stop movement before committing to NVS.
  if (changed) stopAll();
  if (!configStore.save(next, error)) {
    sendError(strcmp(error, "save_too_soon") == 0 ? 429 : 503, error); return;
  }
  if (changed) {
    motorController.applyConfig();
    sensorManager.applyConfig(); sensorManager.startCalibration();
    motionPlayer.applyConfig(); faceEngine.applyConfig();
  }
  char json[96];
  snprintf(json, sizeof(json), "{\"ok\":true,\"changed\":%s,\"revision\":%lu}", changed ? "true" : "false", (unsigned long)configStore.revision());
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json);
}

bool parseDriveUint(const char* key, uint32_t& value) {
  if (!server.hasArg(key)) return false;
  String text = server.arg(key);
  if (!text.length() || text.length() > 10) return false;
  uint64_t parsed = 0;
  for (size_t i = 0; i < text.length(); ++i) {
    if (text[i] < '0' || text[i] > '9') return false;
    parsed = parsed * 10 + (text[i] - '0');
    if (parsed > UINT32_MAX) return false;
  }
  value = static_cast<uint32_t>(parsed); return true;
}

bool handleWebMotor(const String& cmd) {
  if (cmd != "stop" && cmd != "forward" && cmd != "backward" && cmd != "left" && cmd != "right" && cmd != "spin") return false;
  uint32_t boot, client, sequence, issued;
  uint32_t now = millis();
  // Unsequenced stop remains available to old clients. Movement requires the
  // current boot token, a per-page sequence and a fresh device-clock timestamp.
  if (cmd == "stop" && !server.hasArg("seq")) {
    manualDriveGuard.cancelAll(now); motorController.stop();
    if (recreationEngine.mode() != MODE_MANUAL) recreationEngine.setMode(MODE_MANUAL, false);
    return true;
  }
  if (!parseDriveUint("boot", boot) || boot != driveBootId ||
      !parseDriveUint("client", client) || !parseDriveUint("seq", sequence) ||
      !parseDriveUint("issued", issued) || !manualDriveGuard.accept(now, client, sequence, issued, cmd == "stop")) return false;
  if (cmd == "stop") manualDriveGuard.cancelAll(now);
  if (recreationEngine.mode() != MODE_MANUAL) recreationEngine.setMode(MODE_MANUAL, false);
  return motorController.handleCommand(cmd, cmd == "stop" ? 0 : manualDriveRemaining(now, issued));
}

void handleAction() {
  if (!allowMutation()) return;
  if (!server.hasArg("cmd")) { sendError(400, "cmd_required"); return; }
  String type = server.arg("type"), cmd = server.arg("cmd");
  if (type.length() > 20 || cmd.length() > 64) { sendError(400, "invalid_command"); return; }
  bool ok = false;
  // Dispatch typed commands first: song stop must never be mistaken for motor stop.
  if (type == "system" && cmd == "stop_all") { stopAll(); ok = true; }
  else if (type == "system" && cmd == "calibrate_mic") {
    if (!robotSettings.get(MIC_ENABLED)) { sendError(409, "mic_disabled"); return; }
    stopAll(); sensorManager.startCalibration(); ok = true;
  } else if (type == "system" && cmd == "retry_hardware") {
    stopAll(); motorController.begin(); motionPlayer.applyConfig();
    if (robotSettings.get(MOTOR_ENABLED) && !motorController.ready()) { sendError(503, "motor_watchdog_unavailable"); return; }
    if (robotSettings.get(SERVO_ENABLED) && !motionPlayer.ready()) { sendError(503, "servo_pwm_unavailable"); return; }
    ok = true;
  } else if (type == "motor") {
    if (cmd != "stop" && !robotSettings.get(MOTOR_ENABLED)) { sendError(409, "motor_disabled"); return; }
    if (cmd != "stop" && !motorController.ready()) { sendError(503, "motor_watchdog_unavailable"); return; }
    ok = handleWebMotor(cmd);
    if (!ok) { sendError(409, "drive_rejected_refresh"); return; }
  }
  else if (type == "mode") {
    if (cmd == "manual") { recreationEngine.setMode(MODE_MANUAL); ok = true; }
    else if (cmd == "party" && robotSettings.get(MIC_ENABLED) && robotSettings.get(SERVO_ENABLED)) { recreationEngine.setMode(MODE_PARTY_BEAT); ok = true; }
    else if (cmd == "interactive" || cmd == "tracking" || cmd == "follow" || cmd == "follow_me") ok = recreationEngine.startInteractive();
  } else if (type == "tracking" || type == "interactive") {
    if (cmd == "start" || cmd == "follow" || cmd == "on" || cmd == "greet" || cmd == "roam") ok = recreationEngine.startInteractive();
    else if (cmd == "stop" || cmd == "off") { stopAll(); ok = true; }
  } else if (type == "showcase") {
    String motion = server.hasArg("motion") ? server.arg("motion") : "dance";
    if (motion.length() <= 64) ok = recreationEngine.startShowcase(cmd.c_str(), motion.c_str());
  } else if (type == "song") {
    if (cmd == "stop") { songPlayer.stop(); ok = true; }
    else ok = songPlayer.playById(cmd.c_str());
  } else if (type == "timbre") {
    ok = true;
    if (cmd == "music_box") songPlayer.setTimbre(TIMBRE_MUSIC_BOX);
    else if (cmd == "vibrato") songPlayer.setTimbre(TIMBRE_VIBRATO);
    else if (cmd == "acoustic") songPlayer.setTimbre(TIMBRE_ACOUSTIC);
    else if (cmd == "chiptune") songPlayer.setTimbre(TIMBRE_CHIPTUNE);
    else ok = false;
  } else if (type == "face") ok = faceEngine.setFace(cmd.c_str());
  else if (type == "motion") {
    if (!robotSettings.get(SERVO_ENABLED)) { sendError(409, "servo_disabled"); return; }
    if (!motionPlayer.ready()) { sendError(503, "servo_pwm_unavailable"); return; }
    if (recreationEngine.mode() != MODE_MANUAL) recreationEngine.setMode(MODE_MANUAL, false);
    ok = motionPlayer.playById(cmd.c_str());
    if (ok && cmd == "six_seven") faceEngine.setTemporaryFace("six_seven", 3000);
  } else if (type == "servo") {
    if (!robotSettings.get(SERVO_ENABLED)) { sendError(409, "servo_disabled"); return; }
    if (cmd == "axis") {
      String axis = server.arg("axis"); int32_t angle;
      if ((axis != "head" && axis != "left" && axis != "right") || !parseConfigInteger(server.arg("angle"), angle) || angle < 0 || angle > 180) {
        sendError(422, "invalid_servo_angle"); return;
      }
      stopAll();
      if (!motionPlayer.setManualAxis(axis, angle)) { sendError(503, "servo_pwm_unavailable"); return; }
      ok = true;
    } else {
      int32_t angles[3]; const char* keys[] = {"head", "left", "right"};
      for (int i = 0; i < 3; ++i) {
        if (!server.hasArg(keys[i]) || !parseConfigInteger(server.arg(keys[i]), angles[i]) || angles[i] < 0 || angles[i] > 180) { sendError(422, "invalid_servo_angle"); return; }
      }
      if (!motionPlayer.ready()) { sendError(503, "servo_pwm_unavailable"); return; }
      stopAll(); motionPlayer.setManualAngles(angles[0], angles[1], angles[2]); ok = true;
    }
  } else if (type.length() == 0) {
    if (cmd == "stop_all") { stopAll(); ok = true; }
    else if (cmd == "stop" || cmd == "forward" || cmd == "backward" || cmd == "left" || cmd == "right" || cmd == "spin") {
      ok = handleWebMotor(cmd);
      if (!ok) { sendError(409, "drive_rejected_refresh"); return; }
    }
    else {
      if (recreationEngine.mode() != MODE_MANUAL) recreationEngine.setMode(MODE_MANUAL, false);
      ok = faceEngine.setFace(cmd.c_str()) || songPlayer.playById(cmd.c_str()) || motionPlayer.playById(cmd.c_str());
    }
  }
  if (!ok) { sendError(422, "unknown_command_or_hardware_disabled"); return; }
  if (type == "mode" || type == "interactive" || type == "tracking" || type == "showcase" || type == "motion" || type == "servo") {
    manualDriveGuard.cancelAll(millis());
  }
  server.sendHeader("Cache-Control", "no-store");
  char response[256];
  const auto motorSignal = motorController.signalStatus();
  snprintf(response, sizeof(response), "{\"ok\":true,\"uptimeMs\":%lu,\"driveBootId\":%lu,\"motorGPIO\":%u,\"motorPending\":%s,\"motorCommandAgeMs\":%lu,\"motorLeaseMs\":%lu}",
    (unsigned long)millis(), (unsigned long)driveBootId, static_cast<unsigned int>(motorSignal.gpioLevels),
    motorSignal.pending ? "true" : "false", (unsigned long)motorSignal.commandAgeMs, (unsigned long)motorSignal.leaseMs);
  server.send(200, "application/json", response);
}

// ============================================================================
// SYSTEM SETUP
// ============================================================================
void setup() {
  driveBootId = (esp_random() & 0x7FFFFFFF) | 1;
  Serial.begin(115200);
  delay(100);
  Serial.println("\n=============================================");
  Serial.println("  🤖 SMART RECREATIONAL ROBOT INITIALIZING");
  Serial.println("=============================================");

  configStore.begin();

  // Initialize Hardware Drivers
  motorController.begin();
  sensorManager.begin();
  songPlayer.begin();
  motionPlayer.begin();

  // Initialize Display
  if (!faceEngine.begin()) {
    Serial.println("⚠️ Warning: SH1106 OLED Display not detected at 0x3C");
  } else {
    faceEngine.showBootMessage("Connecting WiFi...", WIFI_SSID);
  }

  // Connect to Wi-Fi with timeout and fallback to SoftAP
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setSleep(false); // Low-latency control; trades additional radio power for prompt reception.
  Serial.print("Connecting to Wi-Fi: ");
  Serial.print(WIFI_SSID);

  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 7000) {
    delay(250);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] Connected successfully!");
    Serial.print("[WiFi] IP Address: ");
    Serial.println(WiFi.localIP());
    faceEngine.showBootMessage("WiFi Connected!", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\n[WiFi] Connection timed out. Starting Fallback SoftAP mode...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    Serial.print("[WiFi AP] SSID: ");
    Serial.println(AP_SSID);
    Serial.print("[WiFi AP] IP Address: ");
    Serial.println(WiFi.softAPIP());
    faceEngine.showBootMessage("AP Mode: Robot-Recreation", WiFi.softAPIP().toString().c_str());
  }

  // Setup Web Server Endpoints
  const char* headers[] = {"X-Robot-Request", "Content-Length", "Content-Type"};
  server.collectHeaders(headers, 3);
  server.on("/api/config", HTTP_GET, handleGetConfig);
  server.on("/api/config", HTTP_POST, handleSaveConfig, []() { server.receiveBody(); });
  server.on("/action", HTTP_POST, handleAction, []() { server.receiveBody(); });
  server.on("/", HTTP_GET, handleRoot);
  server.on("/telemetry", HTTP_GET, handleTelemetry);
  server.on("/api/catalog", HTTP_GET, handleCatalog);
  server.on("/action", HTTP_GET, handleAction);
  server.onNotFound([]() { sendError(404, "not_found"); });
  server.begin();
  Serial.println("[Server] HTTP WebServer started on port 80");

  // Ready State
  recreationEngine.begin();
  delay(1200);
  faceEngine.setFace("normal");
  // Keep boot quiet while the microphone baseline is measured in loop().

  Serial.println("=============================================");
  Serial.println("  🎉 RECREATIONAL ROBOT READY FOR ACTION!");
  Serial.println("=============================================\n");
}

// ============================================================================
// SYSTEM MAIN LOOP (Cooperative Non-blocking Multitasking)
// ============================================================================
void loop() {
  uint32_t loopStart = micros();
  server.handleClient();
  sensorManager.update();
  songPlayer.update();
  motionPlayer.update();
  motorController.update();
  faceEngine.update();
  recreationEngine.update();
  maxLoopUs = max(maxLoopUs, micros() - loopStart);
  delay(1); // Yield to Wi-Fi and the scheduler.
}
