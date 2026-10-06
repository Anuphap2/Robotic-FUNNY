#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <ESP32Servo.h>

#include "Config.h"
#include "RuntimeConfig.h"
#include "WebApi.h"
#include <esp_heap_caps.h>

RobotSettings robotSettings;
ConfigStore configStore;
const char* const FIRMWARE_VERSION = "2.2.0";
uint32_t maxLoopUs = 0;
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
  char json[160];
  snprintf(json, sizeof(json), "{\"ok\":false,\"error\":\"%s\"}", code);
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
  motorController.stop(); songPlayer.stop(); motionPlayer.freeze();
  recreationEngine.setMode(MODE_MANUAL, false);
}

void handleTelemetry() {
  char json[1280];
  int length = snprintf(json, sizeof(json),
    "{\"vol\":%d,\"peak\":%d,\"dist\":%.1f,\"distanceValid\":%s,\"distanceAgeMs\":%lu,"
    "\"baseline\":%d,\"calibrating\":%s,\"mode\":\"%s\",\"dir\":\"%s\",\"face\":\"%s\","
    "\"timbre\":\"%s\",\"isPlaying\":%s,\"heapFree\":%lu,\"heapMin\":%lu,\"heapLargest\":%lu,"
    "\"uptimeMs\":%lu,\"maxLoopUs\":%lu,\"oledReady\":%s,\"servoReady\":%s,\"audioReady\":%s,\"motorReady\":%s,"
    "\"rssi\":%d,\"networkMode\":\"%s\",\"ip\":\"%s\",\"version\":\"%s\",\"revision\":%lu,\"pollMs\":%ld,\"interactive\":\"%s\",\"targetClass\":\"%s\",\"decision\":\"%s\",\"behavior\":\"%s\",\"targetLocked\":%s,\"movingVotes\":%u}",
    sensorManager.volume(), sensorManager.peakVolume(), sensorManager.distance(),
    sensorManager.distanceValid() ? "true" : "false", (unsigned long)sensorManager.distanceAge(),
    sensorManager.baseline(), sensorManager.calibrating() ? "true" : "false",
    recreationEngine.modeString(), motorController.directionString(), faceEngine.currentFace().id,
    songPlayer.timbreName(), songPlayer.isPlaying() ? "true" : "false",
    (unsigned long)ESP.getFreeHeap(), (unsigned long)ESP.getMinFreeHeap(),
    (unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
    (unsigned long)millis(), (unsigned long)maxLoopUs,
    faceEngine.ready() ? "true" : "false", motionPlayer.ready() ? "true" : "false", songPlayer.hardwareOK() ? "true" : "false",
    motorController.ready() ? "true" : "false",
    WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0,
    WiFi.status() == WL_CONNECTED ? "station" : "ap",
    (WiFi.status() == WL_CONNECTED ? WiFi.localIP() : WiFi.softAPIP()).toString().c_str(),
    FIRMWARE_VERSION, (unsigned long)configStore.revision(), (long)robotSettings.get(TELEMETRY_INTERVAL),
    recreationEngine.interactiveString(), recreationEngine.decisionClass(), recreationEngine.decisionReason(),
    recreationEngine.behaviorString(), recreationEngine.targetLocked() ? "true" : "false",
    static_cast<unsigned int>(recreationEngine.movingVotes()));
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
  } else if (type == "motor") {
    if (recreationEngine.mode() != MODE_MANUAL) recreationEngine.setMode(MODE_MANUAL, false);
    ok = motorController.handleCommand(cmd);
  }
  else if (type == "mode") {
    if (cmd == "manual") { recreationEngine.setMode(MODE_MANUAL); ok = true; }
    else if (cmd == "party" && robotSettings.get(MIC_ENABLED) && robotSettings.get(SERVO_ENABLED)) { recreationEngine.setMode(MODE_PARTY_BEAT); ok = true; }
    else if (cmd == "interactive") ok = recreationEngine.startInteractive();
  } else if (type == "interactive") {
    // Old start commands are aliases for the unified automatic mode.
    if (cmd == "start" || cmd == "greet" || cmd == "roam" || cmd == "follow") ok = recreationEngine.startInteractive();
    else if (cmd == "stop") { stopAll(); ok = true; }
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
    if (recreationEngine.mode() != MODE_MANUAL) recreationEngine.setMode(MODE_MANUAL, false);
    ok = motionPlayer.playById(cmd.c_str());
    if (ok && cmd == "six_seven") faceEngine.setTemporaryFace("six_seven", 3000);
  } else if (type == "servo") {
    int32_t angles[3]; const char* keys[] = {"head", "left", "right"};
    for (int i = 0; i < 3; ++i) {
      if (!server.hasArg(keys[i]) || !parseConfigInteger(server.arg(keys[i]), angles[i]) || angles[i] < 0 || angles[i] > 180) { sendError(422, "invalid_servo_angle"); return; }
    }
    if (robotSettings.get(SERVO_ENABLED) && motionPlayer.ready()) {
      stopAll(); motionPlayer.setManualAngles(angles[0], angles[1], angles[2]); ok = true;
    }
  } else if (type.length() == 0) {
    if (cmd == "stop_all") { stopAll(); ok = true; }
    else {
      if (recreationEngine.mode() != MODE_MANUAL) recreationEngine.setMode(MODE_MANUAL, false);
      ok = motorController.handleCommand(cmd) || faceEngine.setFace(cmd.c_str()) || songPlayer.playById(cmd.c_str()) || motionPlayer.playById(cmd.c_str());
    }
  }
  if (!ok) { sendError(422, "unknown_command_or_hardware_disabled"); return; }
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", "{\"ok\":true}");
}

// ============================================================================
// SYSTEM SETUP
// ============================================================================
void setup() {
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
