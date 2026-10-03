#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <Arduino.h>

const char MAIN_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="th">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
  <meta charset="utf-8">
  <title>Smart Entertainment Robot Dashboard</title>
  <style>
    * { box-sizing: border-box; -webkit-tap-highlight-color: transparent; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
      background: #0f172a;
      color: #f8fafc;
      margin: 0;
      padding: 16px;
      display: flex;
      justify-content: center;
    }
    .container {
      width: 100%;
      max-width: 480px;
      background: #1e293b;
      padding: 20px;
      border-radius: 24px;
      box-shadow: 0 10px 30px rgba(0,0,0,0.5);
      border: 1px solid #334155;
    }
    h2 { margin: 0 0 4px; font-size: 22px; text-align: center; color: #38bdf8; }
    .subtitle { text-align: center; font-size: 13px; color: #94a3b8; margin-bottom: 18px; }
    
    .hud-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-bottom: 16px; }
    .hud-card { background: #090d16; border: 1px solid #334155; border-radius: 16px; padding: 12px; }
    .hud-title { font-size: 12px; font-weight: 600; color: #94a3b8; margin-bottom: 6px; }
    .hud-val { font-size: 20px; font-weight: bold; color: #38bdf8; }
    .bar-bg { background: #334155; height: 10px; border-radius: 5px; overflow: hidden; margin-top: 8px; }
    .bar-fill { background: #10b981; height: 100%; width: 0%; transition: width 0.1s linear, background-color 0.2s; }

    .sec-label { font-size: 12px; font-weight: 700; letter-spacing: 0.8px; text-transform: uppercase; color: #94a3b8; margin: 16px 0 10px; }

    /* D-PAD Controller */
    .dpad-container { display: grid; grid-template-columns: repeat(3, 1fr); gap: 8px; max-width: 260px; margin: 0 auto 12px; }
    .dpad-btn {
      height: 56px; border: none; border-radius: 16px; background: #334155; color: white;
      font-size: 18px; font-weight: bold; cursor: pointer; display: flex; align-items: center; justify-content: center;
      transition: 0.1s;
    }
    .dpad-btn:active { transform: scale(0.92); background: #38bdf8; }
    .dpad-stop { background: #ef4444; }

    /* Button Grids */
    .grid-2 { display: grid; grid-template-columns: 1fr 1fr; gap: 8px; }
    .grid-3 { display: grid; grid-template-columns: repeat(3, 1fr); gap: 8px; }
    .btn {
      border: none; border-radius: 12px; padding: 12px 6px; font-size: 13px; font-weight: 600;
      cursor: pointer; color: white; transition: transform 0.08s;
    }
    .btn:active { transform: scale(0.95); }
    .btn-motion { background: #6366f1; }
    .btn-face   { background: #0284c7; }
    .btn-song   { background: #ea580c; }
    .btn-sound  { background: #8b5cf6; }
    .btn-full   { grid-column: span 2; }
  </style>
</head>
<body>

<div class="container">
  <h2>🤖 ROBOT CONTROLLER</h2>
  <div class="subtitle">Companion & Recreational Control</div>

  <!-- Telemetry HUD -->
  <div class="hud-grid">
    <div class="hud-card">
      <div class="hud-title">MIC LEVEL (KY-037)</div>
      <div class="hud-val" id="volText">0</div>
      <div class="bar-bg"><div id="barSound" class="bar-fill"></div></div>
    </div>
    <div class="hud-card">
      <div class="hud-title">DISTANCE (HC-SR05)</div>
      <div class="hud-val"><span id="distText">--</span> <small style="font-size:12px; color:#94a3b8;">cm</small></div>
      <div class="bar-bg"><div id="barDist" class="bar-fill" style="background:#0284c7;"></div></div>
    </div>
  </div>

  <!-- Motor D-Pad -->
  <div class="sec-label">🕹 Movement Control</div>
  <div class="dpad-container">
    <div></div>
    <button class="dpad-btn" onclick="send('forward')">▲</button>
    <div></div>
    <button class="dpad-btn" onclick="send('left')">◀</button>
    <button class="dpad-btn dpad-stop" onclick="send('stop')">■</button>
    <button class="dpad-btn" onclick="send('right')">▶</button>
    <div></div>
    <button class="dpad-btn" onclick="send('backward')">▼</button>
    <div></div>
  </div>

  <!-- Actions -->
  <div class="sec-label">💃 Entertainment Actions</div>
  <div class="grid-2">
    <button class="btn btn-motion" onclick="send('dance')">✨ เต้นโชว์ (Dance)</button>
    <button class="btn btn-motion" style="background:#0d9488;" onclick="send('center')">🎯 ท่ายืนตรง</button>
  </div>

  <!-- Jukebox / Songs -->
  <div class="sec-label">🎵 Jukebox & Buzzer Melodies</div>
  <div class="grid-2">
    <button class="btn btn-song btn-full" onclick="send('kaiyang')">🍗 เพลงไก่ย่างถูกเผา (สันทนาการ)</button>
    <button class="btn btn-song" onclick="send('mario')">🍄 Super Mario Theme</button>
    <button class="btn btn-song" style="background:#7c3aed;" onclick="send('coffin')">🕺 Coffin Dance</button>
    <button class="btn btn-song" style="background:#334155; border:1px solid #475569;" onclick="send('starwars')">⚔️ Imperial March</button>
    <button class="btn btn-song" style="background:#0284c7;" onclick="send('babyshark')">🦈 Baby Shark</button>
    <button class="btn btn-song" style="background:#16a34a;" onclick="send('hbd')">🎂 เพลง HBD</button>
    <button class="btn btn-song" style="background:#d97706;" onclick="send('seven')">🔥 Seven Nation</button>
    <button class="btn btn-sound" onclick="send('beep')">🔊 บี๊บสั้น</button>
    <button class="btn btn-sound" onclick="send('snd_happy')">🌟 เสียงทักทาย</button>
  </div>

  <!-- OLED Expressions -->
  <div class="sec-label">😊 OLED Face Expressions</div>
  <div class="grid-3">
    <button class="btn btn-face" onclick="send('normal')">ตาปกติ</button>
    <button class="btn-face btn" onclick="send('blink')">กระพริบตา</button>
    <button class="btn btn-face" onclick="send('happy')">ยิ้มดีใจ</button>
    <button class="btn btn-face" onclick="send('surprise')">ตาโตตกใจ</button>
    <button class="btn btn-face" onclick="send('look_left')">มองซ้าย</button>
    <button class="btn btn-face" onclick="send('look_right')">มองขวา</button>
  </div>
</div>

<script>
function send(cmd) { fetch('/action?cmd=' + cmd); }

setInterval(() => {
  fetch('/telemetry')
    .then(r => r.json())
    .then(d => {
      document.getElementById('volText').innerText = d.vol;
      let soundPct = Math.min(100, Math.round((d.vol / 600) * 100));
      let soundBar = document.getElementById('barSound');
      soundBar.style.width = soundPct + '%';
      soundBar.style.backgroundColor = d.vol > 150 ? '#ef4444' : '#10b981';

      let distVal = d.dist < 0 ? '--' : d.dist;
      document.getElementById('distText').innerText = distVal;
      let distPct = d.dist < 0 ? 0 : Math.min(100, Math.round((d.dist / 100) * 100));
      let distBar = document.getElementById('barDist');
      distBar.style.width = distPct + '%';
      distBar.style.backgroundColor = (d.dist > 0 && d.dist < 15) ? '#ef4444' : '#0284c7';
    });
}, 80);
</script>

</body>
</html>
)rawliteral";

#endif