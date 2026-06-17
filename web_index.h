// ===========================================================================
//  web_index.h  ·  Panel web embebido (se sirve desde la propia placa)
//  Estética: "sala de control" de vigilancia. Oscuro, técnico, animado.
// ===========================================================================
#pragma once

static const char INDEX_HTML[] PROGMEM = R"HTMLPAGE(<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1">
<title>GOOUUU · SENTINEL</title>
<style>
:root{
  --bg:#0a0e12; --bg2:#0e141a; --panel:#10171f; --panel2:#141d27;
  --line:#1f2b38; --line2:#26384a;
  --txt:#cdd9e3; --dim:#6b7d8e; --dimmer:#46586a;
  --amber:#ffb000; --amber-d:#cc8c00; --teal:#22d3ee; --red:#ff3b54; --green:#3ddc84;
  --mono:ui-monospace,'SF Mono',SFMono-Regular,Menlo,Consolas,'Roboto Mono',monospace;
  --sans:system-ui,-apple-system,'Segoe UI',Roboto,'Helvetica Neue',Arial,sans-serif;
  --disp:var(--sans);
}
*{box-sizing:border-box}
html,body{margin:0;padding:0}
body{
  background:
    radial-gradient(1200px 600px at 80% -10%, rgba(34,211,238,.07), transparent 60%),
    radial-gradient(900px 500px at -10% 110%, rgba(255,176,0,.06), transparent 55%),
    var(--bg);
  color:var(--txt); font-family:var(--sans); font-size:14px;
  min-height:100vh; -webkit-font-smoothing:antialiased;
}
/* textura de líneas tenues sobre todo el fondo */
body::before{
  content:""; position:fixed; inset:0; pointer-events:none; z-index:0; opacity:.4;
  background-image:repeating-linear-gradient(0deg, rgba(255,255,255,.015) 0 1px, transparent 1px 3px);
}
.wrap{position:relative; z-index:1; max-width:1320px; margin:0 auto; padding:18px 18px 60px}

/* ---------- TOP BAR ---------- */
header{
  display:flex; align-items:center; gap:16px; padding:14px 18px; margin-bottom:18px;
  background:linear-gradient(180deg,var(--panel2),var(--panel));
  border:1px solid var(--line); border-radius:14px;
  box-shadow:0 10px 40px rgba(0,0,0,.45), inset 0 1px 0 rgba(255,255,255,.03);
}
.brand{display:flex; align-items:center; gap:12px}
.logo{width:40px;height:40px;border-radius:10px;display:grid;place-items:center;
  background:radial-gradient(circle at 30% 30%, #1a2733, #0c1218);
  border:1px solid var(--line2); box-shadow:0 0 0 1px rgba(255,176,0,.12) inset}
.logo svg{width:22px;height:22px}
.brand h1{font-family:var(--disp); font-weight:700; font-size:20px; letter-spacing:2.5px; margin:0; line-height:1}
.brand small{display:block; color:var(--dim); font-family:var(--mono); font-size:10.5px; letter-spacing:3px; margin-top:3px}
.spacer{flex:1}
.statchip{display:flex; align-items:center; gap:8px; font-family:var(--mono); font-size:12px; color:var(--dim);
  padding:7px 12px; border:1px solid var(--line); border-radius:9px; background:var(--bg2)}
.dot{width:9px;height:9px;border-radius:50%; background:var(--dimmer); box-shadow:0 0 0 0 rgba(0,0,0,0)}
.dot.ok{background:var(--green); box-shadow:0 0 10px var(--green); animation:pulse 2.2s infinite}
.dot.bad{background:var(--red); box-shadow:0 0 10px var(--red)}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.45}}
#clock{font-family:var(--mono); font-size:14px; color:var(--teal); letter-spacing:1px}

/* ---------- LAYOUT ---------- */
.grid{display:grid; grid-template-columns:1.55fr 1fr; gap:18px}
@media(max-width:920px){.grid{grid-template-columns:1fr}}

/* ---------- VIDEO ---------- */
.stage{position:relative; background:#05080b; border:1px solid var(--line); border-radius:14px;
  overflow:hidden; box-shadow:0 14px 50px rgba(0,0,0,.5)}
.video-frame{position:relative; aspect-ratio:4/3; background:#04070a; display:grid; place-items:center}
#video{width:100%; height:100%; object-fit:contain; display:block; image-rendering:auto}
.novideo{position:absolute; inset:0; display:grid; place-items:center; color:var(--dimmer);
  font-family:var(--mono); letter-spacing:3px; font-size:13px}
/* esquinas tipo HUD */
.corner{position:absolute; width:26px;height:26px; border:2px solid var(--amber); opacity:.75; pointer-events:none}
.corner.tl{top:12px;left:12px;border-right:0;border-bottom:0}
.corner.tr{top:12px;right:12px;border-left:0;border-bottom:0}
.corner.bl{bottom:12px;left:12px;border-right:0;border-top:0}
.corner.br{bottom:12px;right:12px;border-left:0;border-top:0}
.hud{position:absolute; left:16px; top:16px; right:16px; bottom:16px; pointer-events:none;
  display:flex; flex-direction:column; justify-content:space-between; font-family:var(--mono); font-size:11.5px}
.hud-row{display:flex; justify-content:space-between; align-items:flex-start}
.hud .tag{background:rgba(5,8,11,.6); border:1px solid rgba(255,255,255,.08); padding:4px 8px; border-radius:6px;
  letter-spacing:1px; backdrop-filter:blur(2px)}
.rec{display:flex; align-items:center; gap:7px; color:var(--red)}
.rec .rdot{width:9px;height:9px;border-radius:50%; background:var(--red); box-shadow:0 0 9px var(--red); animation:pulse 1s infinite}
.rec.off{color:var(--dimmer)}
.rec.off .rdot{background:var(--dimmer); box-shadow:none; animation:none}
.scan{position:absolute; inset:0; pointer-events:none; mix-blend-mode:overlay; opacity:.25;
  background:repeating-linear-gradient(0deg, rgba(255,255,255,.10) 0 1px, transparent 1px 4px)}
/* zoom digital + paneo */
.video-frame{cursor:default}
.video-frame.pannable{cursor:grab}
.video-frame.dragging{cursor:grabbing}
#video{transform-origin:center center; will-change:transform}
/* rejilla de encuadre (regla de los tercios) */
.grid-ov{position:absolute; inset:0; pointer-events:none; opacity:0; transition:opacity .2s}
.grid-ov.on{opacity:1}
.grid-ov i{position:absolute; background:rgba(255,255,255,.28)}
.grid-ov .v1,.grid-ov .v2{top:0;bottom:0;width:1px}
.grid-ov .h1,.grid-ov .h2{left:0;right:0;height:1px}
.grid-ov .v1{left:33.33%}.grid-ov .v2{left:66.66%}
.grid-ov .h1{top:33.33%}.grid-ov .h2{top:66.66%}
/* mini controles en la barra */
.zoomctl{display:flex; align-items:center; gap:7px; padding:0 4px; color:var(--dim); font-size:13px}
.zoomctl input{width:96px}
.btn.icon{padding:10px 12px}
.btn.on{border-color:var(--teal); color:var(--teal); background:rgba(34,211,238,.12)}
/* registro de eventos */
.log{display:flex; flex-direction:column; gap:6px; max-height:210px; overflow:auto; padding-right:4px}
.logrow{display:flex; justify-content:space-between; align-items:center; font-family:var(--mono); font-size:11.5px;
  padding:8px 10px; border:1px solid var(--line); border-radius:8px; background:var(--bg2); animation:rise .3s both}
.logrow .ic{color:var(--red)} .logrow .ic.tg{color:var(--teal)} .logrow .ic.rec{color:var(--amber)}
.logrow .t{color:var(--teal)} .logrow .d{color:var(--dim); flex:1; margin:0 10px}
.kbd{font-family:var(--mono); font-size:10px; background:var(--bg2); border:1px solid var(--line2);
  border-radius:4px; padding:1px 5px; color:var(--dim)}
.toolbar{display:flex; gap:8px; flex-wrap:wrap; padding:12px; border-top:1px solid var(--line); background:var(--panel)}

/* ---------- PANELS ---------- */
.col{display:flex; flex-direction:column; gap:18px}
.card{background:linear-gradient(180deg,var(--panel2),var(--panel)); border:1px solid var(--line);
  border-radius:14px; padding:16px 16px 18px; box-shadow:0 10px 36px rgba(0,0,0,.4), inset 0 1px 0 rgba(255,255,255,.02)}
.card h2{font-family:var(--disp); font-weight:600; font-size:13px; letter-spacing:2.5px; color:var(--txt);
  margin:0 0 14px; display:flex; align-items:center; gap:9px; text-transform:uppercase}
.card h2 .bar{width:4px;height:15px;border-radius:2px;background:var(--amber);box-shadow:0 0 10px var(--amber-d)}
.card h2 .bar.t{background:var(--teal);box-shadow:0 0 10px var(--teal)}
.card h2 .bar.r{background:var(--red);box-shadow:0 0 10px var(--red)}

.field{margin-bottom:13px}
.field:last-child{margin-bottom:0}
.field .lab{display:flex; justify-content:space-between; align-items:center; margin-bottom:6px;
  font-size:11.5px; letter-spacing:.6px; color:var(--dim)}
.field .lab b{color:var(--teal); font-family:var(--mono); font-weight:500}

select,input[type=range]{width:100%}
select{appearance:none; background:var(--bg2); color:var(--txt); border:1px solid var(--line2);
  padding:9px 11px; border-radius:8px; font-family:var(--mono); font-size:12.5px; cursor:pointer}
select:focus{outline:none; border-color:var(--amber)}
input[type=range]{-webkit-appearance:none; height:5px; border-radius:4px; background:var(--line2); outline:none}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none; width:17px;height:17px;border-radius:50%;
  background:var(--amber); cursor:pointer; box-shadow:0 0 9px var(--amber-d); border:2px solid #1a1306}
input[type=range]::-moz-range-thumb{width:15px;height:15px;border:0;border-radius:50%;background:var(--amber);cursor:pointer}

/* toggle */
.tg{display:flex; align-items:center; justify-content:space-between; padding:10px 0}
.tg .nm{font-size:12.5px; color:var(--txt)}
.tg .nm span{display:block; color:var(--dimmer); font-size:10.5px; font-family:var(--mono); margin-top:2px}
.switch{position:relative; width:46px; height:25px; flex:none}
.switch input{display:none}
.slider{position:absolute; inset:0; background:var(--line2); border-radius:30px; cursor:pointer; transition:.25s; border:1px solid var(--line)}
.slider:before{content:""; position:absolute; width:19px;height:19px; left:3px; top:2px; border-radius:50%;
  background:var(--dim); transition:.25s}
.switch input:checked + .slider{background:rgba(255,176,0,.25); border-color:var(--amber)}
.switch input:checked + .slider:before{transform:translateX(21px); background:var(--amber); box-shadow:0 0 9px var(--amber-d)}

/* botones */
.btn{appearance:none; cursor:pointer; font-family:var(--disp); font-weight:600; letter-spacing:1.2px; font-size:12px;
  padding:10px 14px; border-radius:9px; border:1px solid var(--line2); background:var(--bg2); color:var(--txt);
  transition:.16s; text-transform:uppercase; display:inline-flex; align-items:center; gap:7px}
.btn:hover{border-color:var(--amber); color:#fff; transform:translateY(-1px)}
.btn:active{transform:translateY(0)}
.btn.primary{background:linear-gradient(180deg,var(--amber),var(--amber-d)); color:#1a1306; border-color:var(--amber)}
.btn.primary:hover{filter:brightness(1.08)}
.btn.teal{border-color:var(--teal); color:var(--teal)}
.btn.teal:hover{background:rgba(34,211,238,.12)}
.btn.danger{border-color:var(--red); color:var(--red)}
.btn.danger:hover{background:rgba(255,59,84,.12)}
.btn.full{width:100%; justify-content:center}
.presets{display:grid; grid-template-columns:repeat(3,1fr); gap:7px; margin-bottom:14px}
.btn.pre{padding:9px 5px; justify-content:center; font-size:11px}
.btn.pre.on{border-color:var(--amber); color:var(--amber); background:rgba(255,176,0,.12)}

/* telemetría */
.telem{display:grid; grid-template-columns:1fr 1fr; gap:1px; background:var(--line); border-radius:10px; overflow:hidden}
.telem .cell{background:var(--bg2); padding:11px 12px}
.telem .k{font-size:10px; letter-spacing:1.5px; color:var(--dimmer); text-transform:uppercase}
.telem .v{font-family:var(--mono); font-size:16px; color:var(--teal); margin-top:3px; font-weight:500}
.telem .v.amber{color:var(--amber)}

/* galería SD */
.gal{display:grid; grid-template-columns:repeat(3,1fr); gap:8px; max-height:280px; overflow:auto; padding-right:4px}
.gal a{position:relative; display:block; aspect-ratio:4/3; border-radius:8px; overflow:hidden; border:1px solid var(--line2); background:#04070a}
.gal img{width:100%;height:100%;object-fit:cover; display:block}
.gal .nm{position:absolute; left:0; right:0; bottom:0; font-family:var(--mono); font-size:8.5px; color:#cfe; padding:3px 4px;
  background:linear-gradient(transparent,rgba(0,0,0,.8)); letter-spacing:.3px}
.gal .del{position:absolute; top:4px; right:4px; width:20px;height:20px; border-radius:5px; border:0; cursor:pointer;
  background:rgba(255,59,84,.85); color:#fff; font-size:13px; line-height:18px; opacity:0; transition:.15s}
.gal a:hover .del{opacity:1}
.empty{color:var(--dimmer); font-family:var(--mono); font-size:11px; padding:18px 0; text-align:center; letter-spacing:1px}

/* toast */
#toast{position:fixed; left:50%; bottom:26px; transform:translateX(-50%) translateY(20px); opacity:0;
  background:var(--panel2); border:1px solid var(--line2); color:var(--txt); padding:11px 18px; border-radius:10px;
  font-family:var(--mono); font-size:12.5px; z-index:50; transition:.25s; box-shadow:0 10px 40px rgba(0,0,0,.6); pointer-events:none}
#toast.show{opacity:1; transform:translateX(-50%) translateY(0)}
#toast.ok{border-color:var(--green)} #toast.err{border-color:var(--red)}

/* aparición escalonada */
.card,.stage{animation:rise .5s both} .stage{animation-delay:.05s}
.col .card:nth-child(1){animation-delay:.12s}.col .card:nth-child(2){animation-delay:.18s}
.col .card:nth-child(3){animation-delay:.24s}.col .card:nth-child(4){animation-delay:.30s}
.col .card:nth-child(5){animation-delay:.36s}
@keyframes rise{from{opacity:0; transform:translateY(14px)}to{opacity:1; transform:none}}
.foot{margin-top:20px; text-align:center; color:var(--dimmer); font-family:var(--mono); font-size:10.5px; letter-spacing:2px}
</style>
</head>
<body>
<div class="wrap">

  <header>
    <div class="brand">
      <div class="logo">
        <svg viewBox="0 0 24 24" fill="none" stroke="#ffb000" stroke-width="1.7">
          <path d="M12 5c-4.5 0-8 4-8 7s3.5 7 8 7 8-4 8-7-3.5-7-8-7Z"/>
          <circle cx="12" cy="12" r="3.2" fill="#ffb000" stroke="none"/>
        </svg>
      </div>
      <div><h1>SENTINEL</h1><small>GOOUUU · ESP32-S3</small></div>
    </div>
    <div class="spacer"></div>
    <div class="statchip"><span id="netdot" class="dot"></span><span id="ip">--</span></div>
    <div class="statchip" id="clock">--:--:--</div>
  </header>

  <div class="grid">
    <!-- ================= VIDEO ================= -->
    <div class="stage">
      <div class="video-frame">
        <img id="video" alt="">
        <div class="novideo" id="novideo">SIN SEÑAL · PULSA INICIAR</div>
        <div class="scan"></div>
        <div class="grid-ov" id="gridOv"><i class="v1"></i><i class="v2"></i><i class="h1"></i><i class="h2"></i></div>
        <span class="corner tl"></span><span class="corner tr"></span>
        <span class="corner bl"></span><span class="corner br"></span>
        <div class="hud">
          <div class="hud-row">
            <div class="rec off" id="rec"><span class="rdot"></span><span id="recTxt">EN ESPERA</span></div>
            <div class="tag" id="resTag">--</div>
          </div>
          <div class="hud-row">
            <div class="tag" id="fpsTag">0.0 FPS</div>
            <div class="tag" id="stampTag">--:--:--</div>
          </div>
        </div>
      </div>
      <div class="toolbar">
        <button class="btn primary" id="streamBtn" onclick="toggleStream()">▶ Iniciar</button>
        <button class="btn teal" onclick="snapshot()">⬇ Captura</button>
        <button class="btn" onclick="saveSD()">💾 Guardar SD</button>
        <button class="btn" id="flashBtn" onclick="toggleFlash()">🔦 Flash</button>
        <button class="btn" id="recBtn" onclick="recClip()">● Grabar clip</button>
        <button class="btn icon" id="gridBtn" title="Rejilla (G)" onclick="toggleGrid()">▦</button>
        <button class="btn icon" id="fsBtn" title="Pantalla completa (F)" onclick="toggleFullscreen()">⛶</button>
        <div class="zoomctl" title="Zoom digital">🔍<input type="range" id="zoom" min="100" max="300" value="100" oninput="setZoom(this.value)"></div>
        <button class="btn danger" style="margin-left:auto" onclick="restart()">⟳ Reiniciar</button>
      </div>
    </div>

    <!-- ================= CONTROLES ================= -->
    <div class="col">

      <div class="card">
        <h2><span class="bar"></span>Transmisión</h2>
        <div class="presets">
          <button class="btn pre" onclick="preset('fluido',this)">⚡ Fluido</button>
          <button class="btn pre" onclick="preset('equilibrado',this)">⚖ Equilibrado</button>
          <button class="btn pre" onclick="preset('calidad',this)">✦ Calidad</button>
        </div>
        <div class="field">
          <div class="lab"><span>Resolución</span></div>
          <select id="framesize" onchange="ctl('framesize',this.value)">
            <option value="5">QVGA · 320×240 (máx fluidez)</option>
            <option value="6">CIF · 400×296</option>
            <option value="8">VGA · 640×480 (recomendado)</option>
            <option value="9">SVGA · 800×600</option>
            <option value="10">XGA · 1024×768</option>
            <option value="11">HD · 1280×720</option>
            <option value="12">SXGA · 1280×1024</option>
            <option value="13">UXGA · 1600×1200 (máx detalle)</option>
          </select>
        </div>
        <div class="field">
          <div class="lab"><span>Calidad JPEG</span><b id="qualityV">12</b></div>
          <input type="range" id="quality" min="4" max="40" oninput="lv('qualityV',this.value)" onchange="ctl('quality',this.value)">
          <div class="lab" style="margin-top:4px"><span style="color:var(--dimmer)">+ nítido</span><span style="color:var(--dimmer)">+ fluido</span></div>
        </div>
      </div>

      <div class="card">
        <h2><span class="bar t"></span>Imagen</h2>
        <div class="field"><div class="lab"><span>Brillo</span><b id="brightnessV">0</b></div>
          <input type="range" id="brightness" min="-2" max="2" oninput="lv('brightnessV',this.value)" onchange="ctl('brightness',this.value)"></div>
        <div class="field"><div class="lab"><span>Contraste</span><b id="contrastV">0</b></div>
          <input type="range" id="contrast" min="-2" max="2" oninput="lv('contrastV',this.value)" onchange="ctl('contrast',this.value)"></div>
        <div class="field"><div class="lab"><span>Saturación</span><b id="saturationV">0</b></div>
          <input type="range" id="saturation" min="-2" max="2" oninput="lv('saturationV',this.value)" onchange="ctl('saturation',this.value)"></div>
        <div class="field"><div class="lab"><span>Efecto</span></div>
          <select id="special_effect" onchange="ctl('special_effect',this.value)">
            <option value="0">Ninguno</option><option value="1">Negativo</option>
            <option value="2">Escala de grises</option><option value="3">Tono rojo</option>
            <option value="4">Tono verde</option><option value="5">Tono azul</option>
            <option value="6">Sepia</option>
          </select></div>
        <div class="tg"><div class="nm">Espejo horizontal</div>
          <label class="switch"><input type="checkbox" id="hmirror" onchange="ctl('hmirror',this.checked?1:0)"><span class="slider"></span></label></div>
        <div class="tg"><div class="nm">Voltear vertical</div>
          <label class="switch"><input type="checkbox" id="vflip" onchange="ctl('vflip',this.checked?1:0)"><span class="slider"></span></label></div>
        <div class="field"><div class="lab"><span>Intensidad del flash (LED)</span><b id="flash_briV">180</b></div>
          <input type="range" id="flash_bri" min="10" max="255" oninput="lv('flash_briV',this.value)" onchange="ctl('flash_bri',this.value)"></div>
      </div>

      <div class="card">
        <h2><span class="bar r"></span>Detección de movimiento</h2>
        <div class="tg"><div class="nm">Vigilancia activa<span>Analiza la escena en busca de cambios</span></div>
          <label class="switch"><input type="checkbox" id="motion" onchange="ctl('motion',this.checked?1:0)"><span class="slider"></span></label></div>
        <div class="field"><div class="lab"><span>Sensibilidad</span><b id="motion_sensV">55</b></div>
          <input type="range" id="motion_sens" min="1" max="100" oninput="lv('motion_sensV',this.value)" onchange="ctl('motion_sens',this.value)"></div>
        <div class="field"><div class="lab"><span>Espera entre alertas</span><b id="cdV">20s</b></div>
          <input type="range" id="motion_cooldown" min="5" max="120" oninput="lv('cdV',this.value+'s')" onchange="ctl('motion_cooldown',this.value)"></div>
        <div class="tg"><div class="nm">Grabar clip al detectar<span>Vídeo AVI en la microSD</span></div>
          <label class="switch"><input type="checkbox" id="rec_clip" onchange="ctl('rec_clip',this.checked?1:0)"><span class="slider"></span></label></div>
      </div>

      <div class="card">
        <h2><span class="bar"></span>Telegram</h2>
        <div class="tg"><div class="nm">Avisos por movimiento<span id="tgState">Comprobando…</span></div>
          <label class="switch"><input type="checkbox" id="tg_alerts" onchange="ctl('tg_alerts',this.checked?1:0)"><span class="slider"></span></label></div>
        <div class="tg"><div class="nm">Guardar también en SD<span>Copia local de cada evento</span></div>
          <label class="switch"><input type="checkbox" id="sd_rec" onchange="ctl('sd_rec',this.checked?1:0)"><span class="slider"></span></label></div>
        <button class="btn teal full" style="margin-top:8px" onclick="tgTest()">✈ Enviar foto de prueba</button>
      </div>

      <div class="card">
        <h2><span class="bar t"></span>Sistema</h2>
        <div class="telem">
          <div class="cell"><div class="k">Tiempo activo</div><div class="v" id="t_up">--</div></div>
          <div class="cell"><div class="k">Señal WiFi</div><div class="v" id="t_rssi">--</div></div>
          <div class="cell"><div class="k">RAM libre</div><div class="v" id="t_heap">--</div></div>
          <div class="cell"><div class="k">PSRAM libre</div><div class="v" id="t_psram">--</div></div>
          <div class="cell"><div class="k">Eventos mov.</div><div class="v amber" id="t_events">0</div></div>
          <div class="cell"><div class="k">Última alerta</div><div class="v amber" id="t_last">nunca</div></div>
        </div>
      </div>

      <div class="card">
        <h2><span class="bar r"></span>Registro de eventos</h2>
        <div id="log" class="log"></div>
        <div id="logEmpty" class="empty">SIN EVENTOS TODAVÍA</div>
      </div>

      <div class="card">
        <h2><span class="bar"></span>Galería SD</h2>
        <div id="gal" class="gal"></div>
        <div id="galEmpty" class="empty" style="display:none">SIN TARJETA O SIN CAPTURAS</div>
        <button class="btn full" style="margin-top:12px" onclick="loadGallery()">↻ Refrescar galería</button>
      </div>

    </div>
  </div>
  <div class="foot">
    <span class="kbd">Espacio</span> directo · <span class="kbd">S</span> captura · <span class="kbd">R</span> clip · <span class="kbd">F</span> pantalla · <span class="kbd">G</span> rejilla<br><br>
    GOOUUU ESP32-S3-CAM · SENTINEL FIRMWARE · PROYECTO PROCESADORES DIGITALES
  </div>
</div>

<div id="toast"></div>

<script>
const $ = id => document.getElementById(id);
const HOST = location.hostname;
const STREAM_URL = `http://${HOST}:81/stream`;
let streaming = false, firstSync = true;
const SIZES = {5:'320×240',8:'640×480',9:'800×600',10:'1024×768',11:'1280×720',12:'1280×1024',13:'1600×1200'};

function toast(msg, kind){const t=$('toast'); t.textContent=msg; t.className='show '+(kind||'');
  clearTimeout(t._t); t._t=setTimeout(()=>t.className='',2600);}
function lv(id,v){$(id).textContent=v;}

async function ctl(v, val){
  try{ await fetch(`/control?var=${encodeURIComponent(v)}&val=${encodeURIComponent(val)}`); }
  catch(e){ toast('Error de conexión','err'); }
}

/* ---- presets de calidad/fluidez ---- */
const PRESETS={ fluido:{fs:5,q:16}, equilibrado:{fs:8,q:12}, calidad:{fs:9,q:10} };
async function preset(name, btn){
  const p=PRESETS[name]; if(!p) return;
  await ctl('framesize',p.fs);
  await ctl('quality',p.q);
  setIf('framesize',p.fs); setIf('quality',p.q); lv('qualityV',p.q);
  document.querySelectorAll('.btn.pre').forEach(b=>b.classList.remove('on'));
  if(btn) btn.classList.add('on');
  if(streaming){ stopStream(); setTimeout(startStream,300); }   // recarga limpia el stream
  toast('Modo: '+name,'ok');
}

/* ---- streaming ---- */
function toggleStream(){ streaming ? stopStream() : startStream(); }
function startStream(){
  $('video').src = STREAM_URL + '?_=' + Date.now();
  $('novideo').style.display='none';
  $('streamBtn').textContent='⏸ Detener';
  $('rec').className='rec'; $('recTxt').textContent='EN DIRECTO';
  streaming = true;
}
function stopStream(){
  $('video').src=''; $('novideo').style.display='grid';
  $('streamBtn').textContent='▶ Iniciar';
  $('rec').className='rec off'; $('recTxt').textContent='EN ESPERA';
  streaming = false;
}

/* ---- acciones ---- */
function snapshot(){
  const a=document.createElement('a');
  a.href='/capture?_='+Date.now(); a.download='captura.jpg'; a.click();
  toast('Captura descargada','ok');
}
async function saveSD(){
  const r=await fetch('/capture?save=1&_='+Date.now());
  const j=await r.json().catch(()=>({}));
  if(j.ok){ toast('Guardada: '+j.file,'ok'); loadGallery(); }
  else toast(j.msg||'No se pudo guardar (¿SD?)','err');
}
async function toggleFlash(){
  const on = !$('flashBtn').classList.contains('primary');
  await ctl('flash', on?1:0);
  $('flashBtn').classList.toggle('primary', on);
}
async function tgTest(){
  toast('Enviando…');
  const r=await fetch('/action?go=tg_test'); const j=await r.json().catch(()=>({}));
  toast(j.ok?'Foto enviada a Telegram':'Fallo al enviar ('+(j.msg||'?')+')', j.ok?'ok':'err');
}
async function restart(){
  if(!confirm('¿Reiniciar la cámara?')) return;
  fetch('/action?go=restart'); toast('Reiniciando…');
  stopStream(); setTimeout(()=>location.reload(), 9000);
}
async function recClip(){
  const r=await fetch('/action?go=rec'); const j=await r.json().catch(()=>({}));
  toast(j.ok?'Grabando clip de vídeo…':(j.msg||'No se pudo grabar'), j.ok?'ok':'err');
}

/* ---- zoom digital + paneo ---- */
let zoom=1, panX=0, panY=0, dragging=false, sx=0, sy=0;
function applyTransform(){
  $('video').style.transform = `translate(${panX}px,${panY}px) scale(${zoom})`;
  document.querySelector('.video-frame').classList.toggle('pannable', zoom>1);
}
function setZoom(v){
  zoom = v/100;
  if(zoom<=1){ panX=0; panY=0; }
  $('zoom').value=v; applyTransform();
}
function clampPan(){
  const f=document.querySelector('.video-frame').getBoundingClientRect();
  const mx=(f.width*(zoom-1))/2, my=(f.height*(zoom-1))/2;
  panX=Math.max(-mx,Math.min(mx,panX)); panY=Math.max(-my,Math.min(my,panY));
}
(function initPan(){
  const fr=document.querySelector('.video-frame');
  const down=e=>{ if(zoom<=1)return; dragging=true; fr.classList.add('dragging');
    const p=e.touches?e.touches[0]:e; sx=p.clientX-panX; sy=p.clientY-panY; };
  const move=e=>{ if(!dragging)return; const p=e.touches?e.touches[0]:e;
    panX=p.clientX-sx; panY=p.clientY-sy; clampPan(); applyTransform(); e.preventDefault(); };
  const up=()=>{ dragging=false; fr.classList.remove('dragging'); };
  fr.addEventListener('mousedown',down); window.addEventListener('mousemove',move); window.addEventListener('mouseup',up);
  fr.addEventListener('touchstart',down,{passive:false}); window.addEventListener('touchmove',move,{passive:false}); window.addEventListener('touchend',up);
  fr.addEventListener('dblclick',()=>setZoom(100));
})();

/* ---- rejilla / pantalla completa ---- */
function toggleGrid(){
  const on=$('gridOv').classList.toggle('on');
  $('gridBtn').classList.toggle('on', on);
}
function toggleFullscreen(){
  const el=document.querySelector('.stage');
  if(!document.fullscreenElement){ el.requestFullscreen && el.requestFullscreen(); }
  else { document.exitFullscreen && document.exitFullscreen(); }
}
document.addEventListener('fullscreenchange',()=>$('fsBtn').classList.toggle('on',!!document.fullscreenElement));

/* ---- atajos de teclado ---- */
document.addEventListener('keydown',e=>{
  if(/^(INPUT|SELECT|TEXTAREA)$/.test(document.activeElement.tagName)) return;
  const k=e.key.toLowerCase();
  if(e.code==='Space'){ e.preventDefault(); toggleStream(); }
  else if(k==='s') snapshot();
  else if(k==='r') recClip();
  else if(k==='f') toggleFullscreen();
  else if(k==='g') toggleGrid();
});

/* ---- registro de eventos (cliente) ---- */
let lastEvents=null, wasRecording=false;
function pushLog(icon, cls, text, time){
  $('logEmpty').style.display='none';
  const row=document.createElement('div'); row.className='logrow';
  row.innerHTML=`<span class="ic ${cls}">${icon}</span><span class="d">${text}</span><span class="t">${time}</span>`;
  const log=$('log'); log.prepend(row);
  while(log.children.length>12) log.removeChild(log.lastChild);
}

/* ---- galería SD ---- */
async function loadGallery(){
  try{
    const j = await (await fetch('/sd/list?_='+Date.now())).json();
    const g=$('gal'); g.innerHTML='';
    if(!j.files || !j.files.length){ $('galEmpty').style.display='block'; return; }
    $('galEmpty').style.display='none';
    j.files.forEach(f=>{
      const enc=encodeURIComponent(f);
      const isVid=f.toLowerCase().endsWith('.avi');
      const a=document.createElement('a'); a.href='/sd/get?f='+enc; a.target='_blank';
      const thumb = isVid
        ? `<div style="display:grid;place-items:center;height:100%;color:var(--teal);font-size:30px">🎞</div>`
        : `<img loading="lazy" src="/sd/get?f=${enc}">`;
      a.innerHTML=thumb+`<button class="del" title="Borrar">×</button><div class="nm">${f}</div>`;
      a.querySelector('.del').onclick=async(e)=>{e.preventDefault();e.stopPropagation();
        if(!confirm('¿Borrar '+f+'?'))return;
        await fetch('/sd/del?f='+enc); loadGallery();};
      g.appendChild(a);
    });
  }catch(e){ $('galEmpty').style.display='block'; }
}

/* ---- estado / telemetría ---- */
function setIf(id,val){const el=$(id); if(el && el.value!=String(val)) el.value=val;}
function setChk(id,b){const el=$(id); if(el) el.checked=!!b;}
async function poll(){
  try{
    const s = await (await fetch('/status?_='+Date.now())).json();
    $('netdot').className='dot ok';
    $('ip').textContent=s.ip;
    $('clock').textContent=s.time;
    $('stampTag').textContent=s.time;
    $('resTag').textContent=SIZES[s.framesize]||'--';
    $('fpsTag').textContent=(streaming? s.fps.toFixed(1):'0.0')+' FPS';
    // telemetría
    $('t_up').textContent=s.uptime;
    $('t_rssi').textContent=s.rssi+' dBm';
    $('t_heap').textContent=(s.heap/1024).toFixed(0)+' KB';
    $('t_psram').textContent=(s.psram/1024).toFixed(0)+' KB';
    $('t_events').textContent=s.motion_events;
    $('t_last').textContent = s.last_motion_s<0 ? 'nunca'
        : (s.last_motion_s<60? s.last_motion_s+'s' : Math.floor(s.last_motion_s/60)+'m');
    // estado Telegram
    $('tgState').textContent = s.tg ? 'Bot configurado ✓' : 'Sin token (ver config.h)';
    // estado de grabación / armado en el HUD
    if(s.recording){ $('rec').className='rec'; $('recTxt').textContent='● GRABANDO CLIP'; }
    else if(s.motion && streaming){ $('rec').className='rec'; $('recTxt').textContent='VIGILANDO'; }
    else if(streaming){ $('rec').className='rec'; $('recTxt').textContent='EN DIRECTO'; }
    $('recBtn').classList.toggle('danger', !!s.recording);
    $('recBtn').textContent = s.recording ? '● Grabando' : '● Grabar clip';

    // registro de eventos (en cliente)
    if(lastEvents!==null && s.motion_events>lastEvents){
      pushLog('●','','Movimiento detectado', s.time);
      if(s.tg && s.tg_alerts) pushLog('✈','tg','Foto enviada a Telegram', s.time);
    }
    lastEvents=s.motion_events;
    if(s.recording && !wasRecording){ pushLog('●','rec','Grabando clip de vídeo', s.time); setTimeout(loadGallery,7000); }
    wasRecording=s.recording;

    // sincronizar controles solo la 1ª vez (luego manda el usuario)
    if(firstSync){
      setIf('framesize',s.framesize); setIf('quality',s.quality); lv('qualityV',s.quality);
      setIf('brightness',s.brightness); lv('brightnessV',s.brightness);
      setIf('contrast',s.contrast); lv('contrastV',s.contrast);
      setIf('saturation',s.saturation); lv('saturationV',s.saturation);
      setIf('special_effect',s.special_effect);
      setChk('hmirror',s.hmirror); setChk('vflip',s.vflip);
      setIf('flash_bri',s.flash_bri); lv('flash_briV',s.flash_bri);
      setChk('motion',s.motion); setIf('motion_sens',s.motion_sens); lv('motion_sensV',s.motion_sens);
      setIf('motion_cooldown',Math.round(s.motion_cooldown/1000)); lv('cdV',Math.round(s.motion_cooldown/1000)+'s');
      setChk('tg_alerts',s.tg_alerts); setChk('sd_rec',s.sd_rec); setChk('rec_clip',s.rec_clip);
      $('flashBtn').classList.toggle('primary', s.flash);
      firstSync=false;
    }
  }catch(e){ $('netdot').className='dot bad'; }
}

window.addEventListener('load', ()=>{ poll(); loadGallery(); setInterval(poll, 2000); });
</script>
</body>
</html>)HTMLPAGE";
