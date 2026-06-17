// ===========================================================================
//  telegram_bot.cpp · Integración con Telegram (Bot API)
//  - Envía fotos (multipart) y textos.
//  - Sondea comandos entrantes (/foto, /estado, /vigilancia_on...).
//  Usa WiFiClientSecure (setInsecure: sin validar certificado, suficiente para
//  un proyecto académico) + HTTPClient (gestiona el "chunked" de Telegram).
// ===========================================================================
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_camera.h"
#include "config.h"
#include "app_state.h"

static const char* TG_HOST = "https://api.telegram.org";
#define TG_BOUNDARY "----ESP32S3SentinelBoundaryX7f3a91d2"

// --- codifica texto para URL (sendMessage por GET) ---
static String urlencode(const String& s) {
  String out; const char* hex = "0123456789ABCDEF";
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~') out += c;
    else { out += '%'; out += hex[(c >> 4) & 0xF]; out += hex[c & 0xF]; }
  }
  return out;
}

bool telegramSendText(const String& text) {
  if (!app.telegramConfigured || WiFi.status() != WL_CONNECTED) return false;
  WiFiClientSecure client; client.setInsecure();
  HTTPClient https;
  String url = String(TG_HOST) + "/bot" + TELEGRAM_BOT_TOKEN +
               "/sendMessage?chat_id=" + TELEGRAM_CHAT_ID + "&text=" + urlencode(text);
  if (!https.begin(client, url)) return false;
  https.setConnectTimeout(10000);
  https.setTimeout(20000);
  int code = https.GET();
  if (code != 200) Serial.printf("[TG] sendText HTTP %d (%s)\n", code, https.errorToString(code).c_str());
  https.end();
  return code == 200;
}

bool telegramSendPhoto(const uint8_t* jpeg, size_t len, const String& caption) {
  if (!app.telegramConfigured || WiFi.status() != WL_CONNECTED) return false;

  String head =
    "--" TG_BOUNDARY "\r\nContent-Disposition: form-data; name=\"chat_id\"\r\n\r\n" +
    String(TELEGRAM_CHAT_ID) + "\r\n"
    "--" TG_BOUNDARY "\r\nContent-Disposition: form-data; name=\"caption\"\r\n\r\n" +
    caption + "\r\n"
    "--" TG_BOUNDARY "\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"snap.jpg\"\r\n"
    "Content-Type: image/jpeg\r\n\r\n";
  String tail = "\r\n--" TG_BOUNDARY "--\r\n";

  size_t total = head.length() + len + tail.length();
  uint8_t* body = (uint8_t*)ps_malloc(total);
  if (!body) { Serial.println("[TG] Sin memoria para la foto"); return false; }
  memcpy(body, head.c_str(), head.length());
  memcpy(body + head.length(), jpeg, len);
  memcpy(body + head.length() + len, tail.c_str(), tail.length());

  WiFiClientSecure client; client.setInsecure();
  HTTPClient https;
  String url = String(TG_HOST) + "/bot" + TELEGRAM_BOT_TOKEN + "/sendPhoto";
  bool ok = false;
  if (https.begin(client, url)) {
    https.setConnectTimeout(10000);
    https.setTimeout(20000);                  // subir foto por hotspot puede tardar
    https.addHeader("Content-Type", "multipart/form-data; boundary=" TG_BOUNDARY);
    int code = https.POST(body, total);
    ok = (code == 200);
    if (!ok) {
      Serial.printf("[TG] sendPhoto HTTP %d (%s)\n", code, https.errorToString(code).c_str());
      String resp = https.getString();
      if (resp.length()) Serial.println("[TG] respuesta: " + resp);
    } else {
      Serial.println("[TG] Foto enviada OK");
    }
    https.end();
  }
  free(body);
  return ok;
}

// --- ejecuta un comando recibido por el chat ---
static void handleCommand(String cmd) {
  cmd.trim(); cmd.toLowerCase();

  if (cmd.startsWith("/foto")) {
    uint8_t* j = nullptr; size_t l = 0;
    if (grabJpegCopy(&j, &l)) { telegramSendPhoto(j, l, "📸 Foto a petición · " + nowStamp()); free(j); }
  }
  else if (cmd.startsWith("/alertas_on"))    { app.telegramAlerts = true;  telegramSendText("✅ Alertas de movimiento ACTIVADAS"); }
  else if (cmd.startsWith("/alertas_off"))   { app.telegramAlerts = false; telegramSendText("🛑 Alertas de movimiento DESACTIVADAS"); }
  else if (cmd.startsWith("/vigilancia_on")) { app.motionEnabled  = true;  telegramSendText("👁 Vigilancia ACTIVADA"); }
  else if (cmd.startsWith("/vigilancia_off")){ app.motionEnabled  = false; telegramSendText("💤 Vigilancia DESACTIVADA"); }
  else if (cmd.startsWith("/flash_on"))      { applyFlash(true);  telegramSendText("🔦 Flash encendido"); }
  else if (cmd.startsWith("/flash_off"))     { applyFlash(false); telegramSendText("🔦 Flash apagado"); }
  else if (cmd.startsWith("/estado")) {
    String m = "📊 *SENTINEL*\n";
    m += "• IP: " + WiFi.localIP().toString() + "\n";
    m += "• WiFi: " + String(WiFi.RSSI()) + " dBm\n";
    m += "• Activo: " + humanUptime() + "\n";
    m += "• Vigilancia: " + String(app.motionEnabled ? "sí" : "no") + "\n";
    m += "• Alertas: " + String(app.telegramAlerts ? "sí" : "no") + "\n";
    m += "• Eventos: " + String(app.motionEvents);
    telegramSendText(m);
  }
  else if (cmd.startsWith("/start") || cmd.startsWith("/ayuda") || cmd.startsWith("/help")) {
    telegramSendText("🛰 SENTINEL · comandos:\n"
                     "/foto - foto al instante\n/estado - estado del sistema\n"
                     "/vigilancia_on /vigilancia_off\n/alertas_on /alertas_off\n"
                     "/flash_on /flash_off");
  }
}

void telegramPollCommands() {
  static long lastUpdateId = 0;
  WiFiClientSecure client; client.setInsecure();
  HTTPClient https;
  String url = String(TG_HOST) + "/bot" + TELEGRAM_BOT_TOKEN +
               "/getUpdates?timeout=0&limit=5&offset=" + String(lastUpdateId + 1);
  if (!https.begin(client, url)) return;
  int code = https.GET();
  if (code != 200) { https.end(); return; }
  String payload = https.getString();
  https.end();

  // Filtro: solo extraemos lo necesario para ahorrar memoria
  JsonDocument filter;
  filter["result"][0]["update_id"] = true;
  filter["result"][0]["message"]["text"] = true;
  filter["result"][0]["message"]["chat"]["id"] = true;

  JsonDocument doc;
  if (deserializeJson(doc, payload, DeserializationOption::Filter(filter))) return;

  for (JsonObject upd : doc["result"].as<JsonArray>()) {
    long uid = upd["update_id"] | 0L;
    if (uid > lastUpdateId) lastUpdateId = uid;
    const char* text = upd["message"]["text"] | "";
    if (text && *text) handleCommand(String(text));
  }
}
