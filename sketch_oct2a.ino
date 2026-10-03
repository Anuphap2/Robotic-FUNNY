#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <ESP32Servo.h>

#include "Config.h"
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
WebServer server(80);

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

// Telemetry Endpoint: Real-time sensor and state metrics
void handleTelemetry() {
  String json = "{";
  json += "\"vol\":" + String(sensorManager.volume()) + ",";
  json += "\"peak\":" + String(sensorManager.peakVolume()) + ",";
  json += "\"dist\":" + String(sensorManager.distance(), 1) + ",";
  json += "\"mode\":\"" + String(recreationEngine.modeString()) + "\",";
  json += "\"dir\":\"" + String(motorController.directionString()) + "\",";
  json += "\"face\":\"" + String(faceEngine.currentFace().id) + "\",";
  json += "\"timbre\":\"" + String(songPlayer.timbreName()) + "\",";
  json += "\"isPlaying\":" + String(songPlayer.isPlaying() ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

// Catalog Endpoint: Dynamically exposes all registered songs, faces, and motions
// Adding any new song or face in C++ automatically reflects in the web UI!
void handleCatalog() {
  String json = "{\"songs\":[";
  for (size_t i = 0; i < SONG_REGISTRY_COUNT; i++) {
    json += "{\"id\":\"" + String(SONG_REGISTRY[i].id) + "\",";
    json += "\"title\":\"" + String(SONG_REGISTRY[i].title) + "\",";
    json += "\"cat\":\"" + String(SONG_REGISTRY[i].category) + "\",";
    json += "\"icon\":\"" + String(SONG_REGISTRY[i].icon) + "\"}";
    if (i + 1 < SONG_REGISTRY_COUNT) json += ",";
  }
  json += "],\"faces\":[";
  for (size_t i = 0; i < FACE_REGISTRY_COUNT; i++) {
    json += "{\"id\":\"" + String(FACE_REGISTRY[i].id) + "\",";
    json += "\"title\":\"" + String(FACE_REGISTRY[i].title) + "\",";
    json += "\"icon\":\"" + String(FACE_REGISTRY[i].icon) + "\"}";
    if (i + 1 < FACE_REGISTRY_COUNT) json += ",";
  }
  json += "],\"motions\":[";
  for (size_t i = 0; i < MOTION_REGISTRY_COUNT; i++) {
    json += "{\"id\":\"" + String(MOTION_REGISTRY[i].id) + "\",";
    json += "\"title\":\"" + String(MOTION_REGISTRY[i].title) + "\",";
    json += "\"icon\":\"" + String(MOTION_REGISTRY[i].icon) + "\"}";
    if (i + 1 < MOTION_REGISTRY_COUNT) json += ",";
  }
  json += "]}";

  server.send(200, "application/json", json);
}

// Action Dispatcher: Clean polymorphic command routing
void handleAction() {
  if (!server.hasArg("cmd")) {
    server.send(400, "text/plain", "Missing cmd argument");
    return;
  }

  String type = server.hasArg("type") ? server.arg("type") : "";
  String cmd  = server.arg("cmd");

  // 1. Motor / Direction commands
  if (type == "motor" || cmd == "forward" || cmd == "backward" || cmd == "left" || cmd == "right" || cmd == "stop" || cmd == "spin") {
    motorController.handleCommand(cmd);
    server.send(200, "text/plain", "OK");
    return;
  }

  // 2. Recreation Mode selection
  if (type == "mode") {
    if (cmd == "manual")      recreationEngine.setMode(MODE_MANUAL);
    else if (cmd == "party")  recreationEngine.setMode(MODE_PARTY_BEAT);
    else if (cmd == "interactive") recreationEngine.setMode(MODE_INTERACTIVE);
    server.send(200, "text/plain", "OK");
    return;
  }

  // 3. Synchronized Showcase performance (Song + Dance)
  if (type == "showcase") {
    String motionId = server.hasArg("motion") ? server.arg("motion") : "dance";
    recreationEngine.startShowcase(cmd.c_str(), motionId.c_str());
    server.send(200, "text/plain", "OK");
    return;
  }

  // 4. Music & Sound commands
  if (type == "song") {
    if (cmd == "stop") {
      songPlayer.stop();
    } else {
      songPlayer.playById(cmd.c_str());
    }
    server.send(200, "text/plain", "OK");
    return;
  }

  // 4.1 Timbre / Instrument selection
  if (type == "timbre") {
    if (cmd == "music_box") songPlayer.setTimbre(TIMBRE_MUSIC_BOX);
    else if (cmd == "vibrato") songPlayer.setTimbre(TIMBRE_VIBRATO);
    else if (cmd == "acoustic") songPlayer.setTimbre(TIMBRE_ACOUSTIC);
    else if (cmd == "chiptune") songPlayer.setTimbre(TIMBRE_CHIPTUNE);
    server.send(200, "text/plain", "OK");
    return;
  }

  // 5. Face / OLED Expression commands
  if (type == "face") {
    faceEngine.setFace(cmd.c_str());
    server.send(200, "text/plain", "OK");
    return;
  }

  // 6. Servo Motion commands
  if (type == "motion") {
    motionPlayer.playById(cmd.c_str());
    server.send(200, "text/plain", "OK");
    return;
  }

  // 7. Emergency Stop / Global reset
  if (cmd == "stop_all" || (type == "system" && cmd == "stop_all")) {
    motorController.stop();
    songPlayer.stop();
    motionPlayer.toCenter();
    recreationEngine.setMode(MODE_MANUAL);
    faceEngine.setFace("normal");
    server.send(200, "text/plain", "OK");
    return;
  }

  // Fallback Dispatcher for legacy / untyped queries
  if (faceEngine.setFace(cmd.c_str())) {
    server.send(200, "text/plain", "OK");
    return;
  }
  if (songPlayer.playById(cmd.c_str())) {
    server.send(200, "text/plain", "OK");
    return;
  }
  if (motionPlayer.playById(cmd.c_str())) {
    server.send(200, "text/plain", "OK");
    return;
  }

  server.send(200, "text/plain", "Ignored");
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
  server.on("/", HTTP_GET, handleRoot);
  server.on("/telemetry", HTTP_GET, handleTelemetry);
  server.on("/api/catalog", HTTP_GET, handleCatalog);
  server.on("/action", HTTP_GET, handleAction);
  server.begin();
  Serial.println("[Server] HTTP WebServer started on port 80");

  // Ready State
  recreationEngine.begin();
  delay(1200);
  faceEngine.setFace("normal");
  songPlayer.playById("sfx_happy");

  Serial.println("=============================================");
  Serial.println("  🎉 RECREATIONAL ROBOT READY FOR ACTION!");
  Serial.println("=============================================\n");
}

// ============================================================================
// SYSTEM MAIN LOOP (Cooperative Non-blocking Multitasking)
// ============================================================================
void loop() {
  server.handleClient();
  sensorManager.update();
  songPlayer.update();
  motionPlayer.update();
  motorController.update();
  faceEngine.update();
  recreationEngine.update();
}