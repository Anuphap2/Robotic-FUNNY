#ifndef WEBUI_H
#define WEBUI_H

#include <Arduino.h>

const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="th">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
  <title>Smart Recreational Robot | หุ่นยนต์สันทนาการอัจฉริยะ</title>
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Plus+Jakarta+Sans:wght@400;600;700;800&family=Prompt:wght@300;400;500;600;700&display=swap" rel="stylesheet">
  <style>
    :root {
      --bg: #070b13;
      --card-bg: #0f172a;
      --card-border: #1e293b;
      --card-hover: #1e293b;
      --primary: #38bdf8;
      --primary-glow: rgba(56, 189, 248, 0.25);
      --accent: #a855f7;
      --accent-glow: rgba(168, 85, 247, 0.25);
      --success: #10b981;
      --warning: #f59e0b;
      --danger: #ef4444;
      --text: #f8fafc;
      --text-muted: #94a3b8;
    }
    * { box-sizing: border-box; -webkit-tap-highlight-color: transparent; margin: 0; padding: 0; }
    body {
      font-family: 'Plus Jakarta Sans', 'Prompt', -apple-system, sans-serif;
      background: radial-gradient(circle at 50% 0%, #1e1b4b 0%, var(--bg) 70%);
      color: var(--text);
      min-height: 100vh;
      display: flex;
      justify-content: center;
      padding: 16px 12px 60px;
    }
    .app {
      width: 100%;
      max-width: 520px;
      display: flex;
      flex-direction: column;
      gap: 16px;
    }

    /* HEADER */
    .header {
      background: rgba(15, 23, 42, 0.75);
      backdrop-filter: blur(12px);
      border: 1px solid var(--card-border);
      border-radius: 24px;
      padding: 16px 20px;
      display: flex;
      align-items: center;
      justify-content: space-between;
      box-shadow: 0 8px 32px rgba(0,0,0,0.4);
    }
    .brand { display: flex; align-items: center; gap: 12px; }
    .brand-icon {
      font-size: 28px;
      background: linear-gradient(135deg, #38bdf8, #818cf8);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
      filter: drop-shadow(0 2px 8px var(--primary-glow));
    }
    .brand-text h1 { font-size: 17px; font-weight: 800; letter-spacing: -0.3px; color: #fff; }
    .brand-text p { font-size: 11px; color: var(--text-muted); font-weight: 500; }
    .status-badge {
      display: flex;
      align-items: center;
      gap: 6px;
      background: rgba(16, 185, 129, 0.12);
      border: 1px solid rgba(16, 185, 129, 0.3);
      padding: 6px 12px;
      border-radius: 999px;
      font-size: 11px;
      font-weight: 600;
      color: var(--success);
    }
    .status-dot {
      width: 8px; height: 8px; border-radius: 50%;
      background: var(--success);
      box-shadow: 0 0 10px var(--success);
      animation: pulse 2s infinite;
    }
    @keyframes pulse { 0%, 100% { opacity: 1; transform: scale(1); } 50% { opacity: 0.4; transform: scale(0.85); } }

    /* TELEMETRY HUD */
    .hud-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; }
    .hud-card {
      background: var(--card-bg);
      border: 1px solid var(--card-border);
      border-radius: 20px;
      padding: 14px 16px;
      display: flex;
      flex-direction: column;
      gap: 6px;
      position: relative;
      overflow: hidden;
    }
    .hud-card::after {
      content: ''; position: absolute; top: 0; left: 0; right: 0; height: 2px;
      background: linear-gradient(90deg, transparent, var(--primary), transparent);
      opacity: 0.6;
    }
    .hud-header { display: flex; align-items: center; justify-content: space-between; }
    .hud-title { font-size: 11px; font-weight: 700; color: var(--text-muted); text-transform: uppercase; letter-spacing: 0.6px; }
    .hud-icon { font-size: 15px; }
    .hud-value-row { display: flex; align-items: baseline; gap: 4px; }
    .hud-val { font-size: 24px; font-weight: 800; color: var(--text); }
    .hud-unit { font-size: 11px; color: var(--text-muted); font-weight: 600; }
    .gauge-bg {
      background: rgba(30, 41, 59, 0.7);
      height: 8px;
      border-radius: 999px;
      overflow: hidden;
      margin-top: 4px;
    }
    .gauge-fill {
      height: 100%;
      width: 0%;
      border-radius: 999px;
      transition: width 0.12s linear, background-color 0.2s;
    }

    /* SECTION CARD */
    .section {
      background: var(--card-bg);
      border: 1px solid var(--card-border);
      border-radius: 22px;
      padding: 16px;
      display: flex;
      flex-direction: column;
      gap: 12px;
    }
    .section-header {
      display: flex;
      align-items: center;
      justify-content: space-between;
    }
    .section-title {
      font-size: 13px;
      font-weight: 700;
      color: #e2e8f0;
      display: flex;
      align-items: center;
      gap: 8px;
    }

    /* MODE SELECTOR */
    .mode-pills { display: grid; grid-template-columns: repeat(3, 1fr); gap: 8px; }
    .mode-pill {
      background: #1e293b;
      border: 1px solid #334155;
      padding: 10px 8px;
      border-radius: 14px;
      cursor: pointer;
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 4px;
      transition: all 0.2s;
      color: var(--text-muted);
      font-size: 11px;
      font-weight: 600;
      text-align: center;
    }
    .mode-pill span { font-size: 18px; }
    .mode-pill.active {
      background: linear-gradient(135deg, rgba(56,189,248,0.2), rgba(129,140,248,0.25));
      border-color: var(--primary);
      color: #fff;
      box-shadow: 0 0 16px var(--primary-glow);
    }

    /* D-PAD JOYSTICK */
    .dpad-wrapper {
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 8px;
      padding: 4px 0 8px;
    }
    .dpad-grid {
      display: grid;
      grid-template-columns: repeat(3, 68px);
      grid-template-rows: repeat(3, 60px);
      gap: 8px;
      user-select: none;
    }
    .dpad-btn {
      background: #1e293b;
      border: 1px solid #334155;
      border-radius: 18px;
      color: var(--text);
      font-size: 20px;
      display: flex;
      align-items: center;
      justify-content: center;
      cursor: pointer;
      transition: all 0.1s cubic-bezier(0.4, 0, 0.2, 1);
      box-shadow: 0 4px 12px rgba(0,0,0,0.3);
    }
    .dpad-btn:active, .dpad-btn.pressed {
      transform: scale(0.92);
      background: var(--primary);
      color: #070b13;
      border-color: var(--primary);
      box-shadow: 0 0 20px var(--primary-glow);
    }
    .dpad-stop {
      background: rgba(239, 68, 68, 0.15);
      border-color: rgba(239, 68, 68, 0.4);
      color: #f87171;
    }
    .dpad-stop:active, .dpad-stop.pressed {
      background: var(--danger);
      color: #fff;
      border-color: var(--danger);
      box-shadow: 0 0 20px rgba(239, 68, 68, 0.5);
    }

    /* SHOWCASE CARDS */
    .showcase-grid { display: grid; grid-template-columns: 1fr; gap: 8px; }
    .showcase-btn {
      background: linear-gradient(135deg, rgba(168,85,247,0.18), rgba(56,189,248,0.18));
      border: 1px solid rgba(168,85,247,0.35);
      border-radius: 14px;
      padding: 12px 16px;
      display: flex;
      align-items: center;
      justify-content: space-between;
      color: #fff;
      font-size: 13px;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.15s;
    }
    .showcase-btn:active { transform: scale(0.97); filter: brightness(1.2); }
    .badge-live {
      background: #a855f7;
      color: white;
      font-size: 10px;
      padding: 3px 8px;
      border-radius: 999px;
      font-weight: 700;
      letter-spacing: 0.4px;
    }

    /* DYNAMIC GRID BUTTONS */
    .catalog-grid {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 8px;
    }
    .catalog-grid-3 {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 8px;
    }
    .act-btn {
      background: #1e293b;
      border: 1px solid #334155;
      color: #e2e8f0;
      padding: 10px 12px;
      border-radius: 14px;
      font-size: 12px;
      font-weight: 600;
      cursor: pointer;
      display: flex;
      align-items: center;
      gap: 8px;
      transition: all 0.12s;
      text-align: left;
    }
    .act-btn:hover { background: #25334a; border-color: #475569; }
    .act-btn:active { transform: scale(0.95); }
    .act-btn-icon { font-size: 18px; flex-shrink: 0; }
    .act-btn-label { flex: 1; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }

    /* CATEGORY PILLS */
    .filter-tabs { display: flex; gap: 6px; overflow-x: auto; padding-bottom: 2px; }
    .filter-tab {
      background: #1e293b;
      border: 1px solid #334155;
      color: var(--text-muted);
      padding: 4px 12px;
      border-radius: 999px;
      font-size: 11px;
      font-weight: 600;
      cursor: pointer;
      white-space: nowrap;
      transition: all 0.15s;
    }
    .filter-tab.active {
      background: var(--primary);
      border-color: var(--primary);
      color: #070b13;
    }

    /* EMERGENCY STOP FOOTER */
    .emergency-btn {
      background: rgba(239, 68, 68, 0.15);
      border: 1px solid var(--danger);
      color: #fca5a5;
      font-weight: 700;
      font-size: 13px;
      padding: 12px;
      border-radius: 16px;
      cursor: pointer;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 8px;
      transition: all 0.15s;
      width: 100%;
    }
    .emergency-btn:active {
      background: var(--danger);
      color: #fff;
    }

    /* TOAST FEEDBACK */
    #toast {
      position: fixed;
      bottom: 24px;
      left: 50%;
      transform: translateX(-50%) translateY(100px);
      background: #1e293b;
      border: 1px solid var(--primary);
      color: #fff;
      padding: 8px 18px;
      border-radius: 999px;
      font-size: 12px;
      font-weight: 600;
      box-shadow: 0 10px 30px rgba(0,0,0,0.6);
      transition: transform 0.25s cubic-bezier(0.18, 0.89, 0.32, 1.28);
      pointer-events: none;
      z-index: 999;
    }
    #toast.show { transform: translateX(-50%) translateY(0); }
  </style>
</head>
<body>

<div class="app">
  <!-- HEADER -->
  <header class="header">
    <div class="brand">
      <span class="brand-icon">🤖</span>
      <div class="brand-text">
        <h1>RECREATION BOT</h1>
        <p>หุ่นยนต์สันทนาการ & เพื่อนร่วมกิจกรรม</p>
      </div>
    </div>
    <div class="status-badge" id="onlineBadge">
      <div class="status-dot"></div>
      <span>ONLINE</span>
    </div>
  </header>

  <!-- TELEMETRY HUD -->
  <section class="hud-grid">
    <!-- Sound HUD -->
    <div class="hud-card">
      <div class="hud-header">
        <span class="hud-title">เสียงไมค์ (KY-037)</span>
        <span class="hud-icon">🎙️</span>
      </div>
      <div class="hud-value-row">
        <span class="hud-val" id="valSound">0</span>
        <span class="hud-unit">amp</span>
      </div>
      <div class="gauge-bg">
        <div id="barSound" class="gauge-fill" style="background:var(--success);"></div>
      </div>
    </div>

    <!-- Distance HUD -->
    <div class="hud-card">
      <div class="hud-header">
        <span class="hud-title">ระยะทาง (HC-SR05)</span>
        <span class="hud-icon">📡</span>
      </div>
      <div class="hud-value-row">
        <span class="hud-val" id="valDist">--</span>
        <span class="hud-unit">cm</span>
      </div>
      <div class="gauge-bg">
        <div id="barDist" class="gauge-fill" style="background:var(--primary);"></div>
      </div>
    </div>
  </section>

  <!-- RECREATION MODE SELECTOR -->
  <section class="section">
    <div class="section-header">
      <span class="section-title">🎭 โหมดการทำงานสันทนาการ (Mode)</span>
    </div>
    <div class="mode-pills" id="modeContainer">
      <div class="mode-pill active" onclick="setMode('manual')">
        <span>🕹️</span>Manual
      </div>
      <div class="mode-pill" onclick="setMode('party')">
        <span>🎉</span>Party Beat
      </div>
      <div class="mode-pill" onclick="setMode('interactive')">
        <span>👋</span>Interactive
      </div>
    </div>
  </section>

  <!-- MOVEMENT JOYSTICK -->
  <section class="section">
    <div class="section-header">
      <span class="section-title">🕹️ ควบคุมการเคลื่อนที่ (D-Pad Controller)</span>
    </div>
    <div class="dpad-wrapper">
      <div class="dpad-grid">
        <div></div>
        <button class="dpad-btn" id="btnFwd" data-dir="forward">▲</button>
        <div></div>
        <button class="dpad-btn" id="btnLeft" data-dir="left">◀</button>
        <button class="dpad-btn dpad-stop" id="btnStop" data-dir="stop">■</button>
        <button class="dpad-btn" id="btnRight" data-dir="right">▶</button>
        <div></div>
        <button class="dpad-btn" id="btnBwd" data-dir="backward">▼</button>
        <div></div>
      </div>
    </div>
  </section>

  <!-- SYNC SHOWCASE (เพลง+เต้นสันทนาการ) -->
  <section class="section">
    <div class="section-header">
      <span class="section-title">🌟 การแสดงสันทนาการ (Sync Song & Dance)</span>
    </div>
    <div class="showcase-grid" id="showcaseContainer">
      <button class="showcase-btn" onclick="startShowcase('numalee', 'dance')">
        <span>🐱 หนูมาลีมีลูกแมวเหมียว (เพลงน่ารัก + สันทนาการแดนซ์)</span>
        <span class="badge-live">SHOW</span>
      </button>
      <button class="showcase-btn" onclick="startShowcase('kaiyang', 'chicken')">
        <span>🍗 ไก่ย่างถูกเผา (เพลงสันทนาการ + ท่ากระพือปีก)</span>
        <span class="badge-live">SHOW</span>
      </button>
      <button class="showcase-btn" onclick="startShowcase('kingkanbai', 'dance')">
        <span>🍃 กิ่งก้านใบ (เพลงฮิต + แดนซ์สลับแขน)</span>
        <span class="badge-live">SHOW</span>
      </button>
      <button class="showcase-btn" onclick="startShowcase('coffin', 'disco')">
        <span>🕺 Coffin Dance Show (เพลงไวรัล + ดิสโก้)</span>
        <span class="badge-live">SHOW</span>
      </button>
    </div>
  </section>

  <!-- DYNAMIC JUKEBOX (SONGS) -->
  <section class="section">
    <div class="section-header">
      <span class="section-title">🎵 ตู้เพลงสันทนาการ (Jukebox)</span>
      <button class="filter-tab" style="background:#ef4444; border-color:#ef4444; color:#fff;" onclick="sendCmd('song', 'stop')">⏹ หยุดเพลง</button>
    </div>
    <div class="filter-tabs" id="songTabs">
      <button class="filter-tab active" onclick="filterSongs('all')">ทั้งหมด</button>
      <button class="filter-tab" onclick="filterSongs('recreation')">🎉 สันทนาการ</button>
      <button class="filter-tab" onclick="filterSongs('pop')">✨ ยอดฮิต</button>
      <button class="filter-tab" onclick="filterSongs('game')">🎮 เกม</button>
      <button class="filter-tab" onclick="filterSongs('sfx')">🔊 เอฟเฟกต์</button>
    </div>
    <div class="catalog-grid" id="songsContainer">
      <!-- Dynamically populated from /api/catalog -->
      <div style="grid-column:span 2; font-size:12px; color:var(--text-muted); text-align:center; padding:12px;">กำลังโหลดรายการเพลง...</div>
    </div>
  </section>

  <!-- DYNAMIC OLED FACES -->
  <section class="section">
    <div class="section-header">
      <span class="section-title">😊 สีหน้าอารมณ์ OLED (Face Expressions)</span>
    </div>
    <div class="catalog-grid-3" id="facesContainer">
      <!-- Dynamically populated from /api/catalog -->
      <div style="grid-column:span 3; font-size:12px; color:var(--text-muted); text-align:center; padding:12px;">กำลังโหลดรายการหน้าตา...</div>
    </div>
  </section>

  <!-- DYNAMIC SERVO MOTIONS -->
  <section class="section">
    <div class="section-header">
      <span class="section-title">💃 ท่าทางและการเคลื่อนไหว (Servo Motions)</span>
    </div>
    <div class="catalog-grid" id="motionsContainer">
      <!-- Dynamically populated from /api/catalog -->
      <div style="grid-column:span 2; font-size:12px; color:var(--text-muted); text-align:center; padding:12px;">กำลังโหลดรายการท่าทาง...</div>
    </div>
  </section>

  <!-- EMERGENCY STOP -->
  <button class="emergency-btn" onclick="sendCmd('system', 'stop_all')">
    <span>🛑</span> EMERGENCY STOP (หยุดทุกอย่างทันที)
  </button>
</div>

<div id="toast">คำสั่งถูกส่งแล้ว!</div>

<script>
// ================= CLIENT-SIDE CS DATA ARCHITECTURE =================
let catalogData = { songs: [], faces: [], motions: [] };
let currentSongFilter = 'all';

function showToast(msg) {
  const t = document.getElementById('toast');
  t.innerText = msg;
  t.classList.add('show');
  setTimeout(() => t.classList.remove('show'), 1200);
}

function sendCmd(type, cmd, extra = '') {
  let url = `/action?type=${encodeURIComponent(type)}&cmd=${encodeURIComponent(cmd)}`;
  if (extra) url += extra;
  fetch(url).catch(e => console.error(e));
}

function setMode(mode) {
  sendCmd('mode', mode);
  document.querySelectorAll('.mode-pill').forEach(p => p.classList.remove('active'));
  event.currentTarget.classList.add('active');
  showToast('เปลี่ยนโหมดเป็น: ' + mode);
}

function startShowcase(songId, motionId) {
  sendCmd('showcase', songId, `&motion=${encodeURIComponent(motionId)}`);
  showToast('เริ่มการแสดง: ' + songId);
}

// Render dynamic catalog without hardcoding
function loadCatalog() {
  fetch('/api/catalog')
    .then(r => r.json())
    .then(data => {
      catalogData = data;
      renderSongs();
      renderFaces();
      renderMotions();
    })
    .catch(err => {
      console.warn('Catalog load error, retrying in 2s...', err);
      setTimeout(loadCatalog, 2000);
    });
}

function renderSongs() {
  const container = document.getElementById('songsContainer');
  container.innerHTML = '';
  const filtered = (currentSongFilter === 'all') 
    ? catalogData.songs 
    : catalogData.songs.filter(s => s.cat === currentSongFilter);

  filtered.forEach(s => {
    const btn = document.createElement('button');
    btn.className = 'act-btn';
    btn.innerHTML = `<span class="act-btn-icon">${s.icon || '🎵'}</span><span class="act-btn-label">${s.title}</span>`;
    btn.onclick = () => {
      sendCmd('song', s.id);
      showToast('เล่นเพลง: ' + s.title);
    };
    container.appendChild(btn);
  });
}

function filterSongs(cat) {
  currentSongFilter = cat;
  document.querySelectorAll('#songTabs .filter-tab').forEach(t => t.classList.remove('active'));
  event.target.classList.add('active');
  renderSongs();
}

function renderFaces() {
  const container = document.getElementById('facesContainer');
  container.innerHTML = '';
  catalogData.faces.forEach(f => {
    const btn = document.createElement('button');
    btn.className = 'act-btn';
    btn.innerHTML = `<span class="act-btn-icon">${f.icon || '👀'}</span><span class="act-btn-label">${f.title}</span>`;
    btn.onclick = () => {
      sendCmd('face', f.id);
      showToast('เปลี่ยนหน้า: ' + f.title);
    };
    container.appendChild(btn);
  });
}

function renderMotions() {
  const container = document.getElementById('motionsContainer');
  container.innerHTML = '';
  catalogData.motions.forEach(m => {
    const btn = document.createElement('button');
    btn.className = 'act-btn';
    btn.innerHTML = `<span class="act-btn-icon">${m.icon || '💃'}</span><span class="act-btn-label">${m.title}</span>`;
    btn.onclick = () => {
      sendCmd('motion', m.id);
      showToast('ทำท่าทาง: ' + m.title);
    };
    container.appendChild(btn);
  });
}

// D-Pad touch & mouse event listeners
const dpadButtons = document.querySelectorAll('.dpad-btn');
dpadButtons.forEach(btn => {
  const dir = btn.dataset.dir;
  const startAction = (e) => {
    e.preventDefault();
    btn.classList.add('pressed');
    sendCmd('motor', dir);
  };
  const endAction = (e) => {
    e.preventDefault();
    btn.classList.remove('pressed');
    if (dir !== 'stop') sendCmd('motor', 'stop');
  };

  btn.addEventListener('mousedown', startAction);
  btn.addEventListener('mouseup', endAction);
  btn.addEventListener('touchstart', startAction, { passive: false });
  btn.addEventListener('touchend', endAction, { passive: false });
});

// Telemetry Poller
setInterval(() => {
  fetch('/telemetry')
    .then(r => r.json())
    .then(d => {
      // Sound HUD
      document.getElementById('valSound').innerText = d.vol;
      let soundPct = Math.min(100, Math.round((d.vol / 500) * 100));
      let barS = document.getElementById('barSound');
      barS.style.width = soundPct + '%';
      barS.style.background = (d.vol > 160) ? 'var(--danger)' : 'var(--success)';

      // Distance HUD
      let distVal = (d.dist < 0 || d.dist > 350) ? '--' : d.dist.toFixed(1);
      document.getElementById('valDist').innerText = distVal;
      let distPct = (d.dist < 0) ? 0 : Math.min(100, Math.round((d.dist / 120) * 100));
      let barD = document.getElementById('barDist');
      barD.style.width = distPct + '%';
      barD.style.background = (d.dist > 0 && d.dist < 18) ? 'var(--danger)' : 'var(--primary)';
    })
    .catch(() => {});
}, 90);

loadCatalog();
</script>

</body>
</html>
)rawliteral";

#endif // WEBUI_H
