// ===========================================================================
//  GOOUUU ESP32-S3-CAM · SENTINEL · main.cpp
//  Arranque, inicialización de cámara/WiFi/SD/NTP y bucle principal.
// ===========================================================================
#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <SD_MMC.h>
#include <time.h>
#include "esp_camera.h"

#include "config.h"
#include "camera_pins.h"
#include "app_state.h"

// ---- Instancias globales (declaradas extern en app_state.h) ----
AppState app;
bool     sdAvailable = false;

// ===========================================================================
//  LED RGB (flash + estado)
// ===========================================================================
void applyFlash(bool on) {
  app.flashOn = on;
  uint8_t b = on ? app.flashBrightness : 0;
  neopixelWrite(RGB_LED_PIN, b, b, b);
}
void blinkStatus(uint8_t r, uint8_t g, uint8_t b, int ms) {
  neopixelWrite(RGB_LED_PIN, r, g, b);
  delay(ms);
  applyFlash(app.flashOn);           // restaura el estado real del flash
}

// ===========================================================================
//  Tiempo / utilidades
// ===========================================================================
String nowStamp() {
  struct tm t;
  if (getLocalTime(&t, 100)) {
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &t);
    return String(buf);
  }
  return "t" + String(millis() / 1000);   // sin hora aún: usa segundos de uptime
}
String humanUptime() {
  uint32_t s = (millis() - app.bootMillis) / 1000;
  uint32_t d = s / 86400; s %= 86400;
  uint32_t h = s / 3600;  s %= 3600;
  uint32_t m = s / 60;    s %= 60;
  char buf[24];
  if (d) snprintf(buf, sizeof(buf), "%lud %02lu:%02lu", (unsigned long)d, (unsigned long)h, (unsigned long)m);
  else   snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", (unsigned long)h, (unsigned long)m, (unsigned long)s);
  return String(buf);
}

// ===========================================================================
//  Copia rápida de un fotograma JPEG (el llamador debe hacer free()).
//  Se libera el framebuffer enseguida para no frenar el streaming.
// ===========================================================================
bool grabJpegCopy(uint8_t** out, size_t* outLen) {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) return false;
  bool ok = false;
  if (fb->format == PIXFORMAT_JPEG) {
    uint8_t* copy = (uint8_t*)ps_malloc(fb->len);
    if (copy) {
      memcpy(copy, fb->buf, fb->len);
      *out = copy; *outLen = fb->len; ok = true;
    }
  }
  esp_camera_fb_return(fb);
  return ok;
}

// ===========================================================================
//  Guardar JPEG en la microSD
// ===========================================================================
bool sdSaveJpeg(const uint8_t* buf, size_t len, String& outName) {
  if (!sdAvailable) return false;
  String name = "img_" + nowStamp() + ".jpg";
  File f = SD_MMC.open("/" + name, FILE_WRITE);
  if (!f) return false;
  size_t w = f.write(buf, len);
  f.close();
  if (w != len) { SD_MMC.remove("/" + name); return false; }
  outName = name;
  return true;
}

// ===========================================================================
//  Inicialización de la cámara (OV2640/OV5640 de la GOOUUU S3-CAM)
// ===========================================================================
static bool initCamera() {
  camera_config_t config = {};
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM; config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM; config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM; config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM; config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size   = FRAMESIZE_VGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode    = CAMERA_GRAB_LATEST;
  config.fb_location  = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count     = 2;             // 2 = menor latencia en el directo

  if (!psramFound()) {                 // sin PSRAM: degradar con elegancia
    Serial.println("[CAM] Aviso: PSRAM no detectada. Bajando calidad.");
    config.frame_size  = FRAMESIZE_SVGA;
    config.fb_location = CAMERA_FB_IN_DRAM;
    config.fb_count    = 1;
    config.jpeg_quality = 14;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[CAM] esp_camera_init falló: 0x%x\n", err);
    return false;
  }
  // Ajustes iniciales del sensor
  sensor_t* s = esp_camera_sensor_get();
  if (s) {
    s->set_brightness(s, 0);
    s->set_saturation(s, 0);
    if (s->id.PID == OV5640_PID) {     // el OV5640 suele venir invertido
      s->set_vflip(s, 1);
      s->set_hmirror(s, 1);
    }
  }
  return true;
}

// ===========================================================================
//  WiFi
// ===========================================================================
static void connectWiFi() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);                // CLAVE para que el vídeo vaya fluido
  WiFi.setHostname(DEVICE_HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("[WiFi] Conectando a \"%s\"", WIFI_SSID);

  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) {
    delay(400); Serial.print("."); blinkStatus(0, 0, 25, 40);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n[WiFi] OK · IP: %s\n", WiFi.localIP().toString().c_str());
    blinkStatus(0, 40, 0, 250);
  } else {
    Serial.println("\n[WiFi] No se pudo conectar. Reintentos en el bucle.");
    blinkStatus(40, 0, 0, 250);
  }
}

// ===========================================================================
//  microSD (SD_MMC en modo 1 bit)
// ===========================================================================
static void initSD() {
  SD_MMC.setPins(SD_MMC_CLK_PIN, SD_MMC_CMD_PIN, SD_MMC_D0_PIN);
  if (SD_MMC.begin("/sdcard", true)) {          // true = modo 1 bit
    uint8_t type = SD_MMC.cardType();
    if (type != CARD_NONE) {
      sdAvailable = true;
      Serial.printf("[SD] Tarjeta lista (%lluMB)\n", SD_MMC.cardSize() / (1024 * 1024));
      return;
    }
  }
  Serial.println("[SD] Sin tarjeta (las capturas a SD quedan desactivadas).");
}

// ===========================================================================
//  Reacción ante un evento de movimiento
// ===========================================================================
static void handleMotionEvent() {
  app.motionEvents++;
  app.lastMotionMs = millis();
  Serial.println("[MOV] ¡Movimiento detectado!");
  neopixelWrite(RGB_LED_PIN, 70, 0, 0);     // rojo mientras procesa el aviso

  uint8_t* jpg = nullptr; size_t len = 0;
  if (grabJpegCopy(&jpg, &len)) {
    String stamp = nowStamp();
    if (app.sdRecordOnMotion && sdAvailable) {
      String nm; if (sdSaveJpeg(jpg, len, nm)) Serial.println("[SD] Guardado: " + nm);
    }
    if (app.telegramAlerts && app.telegramConfigured) {
      telegramSendPhoto(jpg, len, "🚨 Movimiento detectado\n🕒 " + stamp);
    }
    free(jpg);
  }
  // Clip de vídeo (si está activado y hay SD)
  if (app.recordClipOnMotion && sdAvailable && !recorderIsActive()) {
    recorderStart("movimiento");
  }
  applyFlash(app.flashOn);                   // restaura el LED
}

// ===========================================================================
//  SETUP
// ===========================================================================
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n\n===== GOOUUU ESP32-S3-CAM · SENTINEL =====");

  // Estado inicial desde config.h
  app.bootMillis        = millis();
  app.motionEnabled     = DEFAULT_MOTION_ENABLED;
  app.motionSensitivity = DEFAULT_MOTION_SENSITIVITY;
  app.motionCooldownMs  = DEFAULT_MOTION_COOLDOWN_MS;
  app.telegramAlerts    = DEFAULT_TELEGRAM_ALERTS;
  app.sdRecordOnMotion  = DEFAULT_SD_RECORD_ON_MOTION;
  app.recordClipOnMotion= DEFAULT_RECORD_CLIP_ON_MOTION;
  app.recording         = false;
  app.flashBrightness   = 180;
  app.flashOn           = false;
  app.lastMotionMs      = 0;
  app.motionEvents      = 0;
  app.framesServed      = 0;
  app.streamFps         = 0;
  String tok = TELEGRAM_BOT_TOKEN;
  app.telegramConfigured = (tok.length() > 20 && tok.indexOf("pon_aqui") < 0);

  applyFlash(false);                         // LED apagado
  blinkStatus(20, 12, 0, 200);               // ámbar: arrancando

  if (!initCamera()) {
    Serial.println("[FATAL] Cámara no inicializada. Revisa el conector FFC.");
    while (true) { blinkStatus(80, 0, 0, 150); delay(150); }   // parpadeo rojo
  }
  Serial.println("[CAM] Cámara lista.");

  connectWiFi();
  configTzTime(TIMEZONE_TZ, NTP_SERVER_1, NTP_SERVER_2);
  initSD();
  motionInit();
  recorderInit();

  startCameraServer();
  if (MDNS.begin(DEVICE_HOSTNAME)) {
    MDNS.addService("http", "tcp", 80);
    Serial.printf("[NET] Accede en: http://%s.local  o  http://%s\n",
                  DEVICE_HOSTNAME, WiFi.localIP().toString().c_str());
  }

  Serial.printf("[SYS] Telegram: %s · SD: %s\n",
                app.telegramConfigured ? "configurado" : "sin token",
                sdAvailable ? "ok" : "no");
  Serial.println("===== Sistema operativo =====\n");
  blinkStatus(0, 50, 0, 400);                // verde: todo OK
}

// ===========================================================================
//  LOOP
// ===========================================================================
void loop() {
  uint32_t now = millis();

  // --- Reconexión WiFi ---
  static uint32_t lastWifiTry = 0;
  if (WiFi.status() != WL_CONNECTED && now - lastWifiTry > 8000) {
    lastWifiTry = now;
    Serial.println("[WiFi] Reconectando...");
    WiFi.disconnect(); WiFi.reconnect();
  }

  // --- Grabación de clip en curso (no bloqueante) ---
  recorderService();

  // --- Detección de movimiento (en pausa mientras se graba un clip) ---
  static uint32_t lastMotion = 0;
  if (app.motionEnabled && !recorderIsActive() && now - lastMotion >= MOTION_CHECK_INTERVAL_MS) {
    lastMotion = now;
    if (motionCheck()) handleMotionEvent();
  }

  // --- Comandos entrantes de Telegram ---
  // Se pausa mientras alguien ve el directo, para no robarle ancho de banda
  // al vídeo (los avisos por movimiento sí se siguen enviando).
  static const bool kTgCommands = ENABLE_TELEGRAM_COMMANDS;
  static uint32_t lastPoll = 0;
  if (kTgCommands && app.telegramConfigured && !app.clientStreaming &&
      WiFi.status() == WL_CONNECTED && now - lastPoll >= TELEGRAM_POLL_INTERVAL_MS) {
    lastPoll = now;
    telegramPollCommands();
  }

  delay(5);
}
