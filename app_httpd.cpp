// ===========================================================================
//  app_httpd.cpp · Servidores web (esp_http_server)
//  - Puerto 80: panel, estado, control, captura, acciones, galería SD.
//  - Puerto 81: streaming MJPEG (separado para máxima fluidez).
// ===========================================================================
#include <Arduino.h>
#include <WiFi.h>
#include <SD_MMC.h>
#include <FS.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "lwip/sockets.h"

#include "config.h"
#include "app_state.h"
#include "web_index.h"

static httpd_handle_t camera_httpd = NULL;
static httpd_handle_t stream_httpd = NULL;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* STREAM_BOUNDARY     = "\r\n--" PART_BOUNDARY "\r\n";
static const char* STREAM_PART         = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// ---------------------------------------------------------------------------
//  Utilidad: leer un parámetro de la query (?key=value)
// ---------------------------------------------------------------------------
static bool getParam(httpd_req_t* req, const char* key, char* out, size_t outLen) {
  size_t qlen = httpd_req_get_url_query_len(req) + 1;
  if (qlen <= 1) return false;
  char* buf = (char*)malloc(qlen);
  if (!buf) return false;
  bool ok = false;
  if (httpd_req_get_url_query_str(req, buf, qlen) == ESP_OK)
    ok = (httpd_query_key_value(buf, key, out, outLen) == ESP_OK);
  free(buf);
  return ok;
}

static bool safeName(const char* f) {
  if (!f || !*f) return false;
  if (strchr(f, '/') || strchr(f, '\\') || strstr(f, "..")) return false;
  return strstr(f, ".jpg") != NULL || strstr(f, ".avi") != NULL;
}

// ---------------------------------------------------------------------------
//  /  -> panel web
// ---------------------------------------------------------------------------
static esp_err_t index_handler(httpd_req_t* req) {
  httpd_resp_set_type(req, "text/html");
  httpd_resp_set_hdr(req, "Content-Encoding", "identity");
  return httpd_resp_send(req, (const char*)INDEX_HTML, strlen((const char*)INDEX_HTML));
}

// ---------------------------------------------------------------------------
//  /status -> JSON con ajustes y telemetría
// ---------------------------------------------------------------------------
String buildStatusJson() {
  sensor_t* s = esp_camera_sensor_get();
  String j = "{";
  if (s) {
    j += "\"framesize\":"      + String(s->status.framesize)      + ",";
    j += "\"quality\":"        + String(s->status.quality)        + ",";
    j += "\"brightness\":"     + String(s->status.brightness)     + ",";
    j += "\"contrast\":"       + String(s->status.contrast)       + ",";
    j += "\"saturation\":"     + String(s->status.saturation)     + ",";
    j += "\"special_effect\":" + String(s->status.special_effect) + ",";
    j += "\"hmirror\":"        + String(s->status.hmirror)        + ",";
    j += "\"vflip\":"          + String(s->status.vflip)          + ",";
  }
  j += "\"flash\":"           + String(app.flashOn ? 1 : 0)        + ",";
  j += "\"flash_bri\":"       + String(app.flashBrightness)        + ",";
  j += "\"motion\":"          + String(app.motionEnabled ? 1 : 0) + ",";
  j += "\"motion_sens\":"     + String(app.motionSensitivity)     + ",";
  j += "\"motion_cooldown\":" + String(app.motionCooldownMs)      + ",";
  j += "\"tg_alerts\":"       + String(app.telegramAlerts ? 1 : 0)+ ",";
  j += "\"sd_rec\":"          + String(app.sdRecordOnMotion ? 1 : 0) + ",";
  j += "\"rec_clip\":"        + String(app.recordClipOnMotion ? 1 : 0) + ",";
  j += "\"recording\":"       + String(app.recording ? 1 : 0)     + ",";
  j += "\"motion_events\":"   + String(app.motionEvents)          + ",";
  long lastS = app.lastMotionMs ? (long)((millis() - app.lastMotionMs) / 1000) : -1;
  j += "\"last_motion_s\":"   + String(lastS)                     + ",";
  j += "\"uptime\":\""        + humanUptime()                     + "\",";
  j += "\"heap\":"            + String(ESP.getFreeHeap())         + ",";
  j += "\"psram\":"           + String(ESP.getFreePsram())        + ",";
  j += "\"rssi\":"            + String(WiFi.RSSI())               + ",";
  j += "\"ip\":\""            + WiFi.localIP().toString()         + "\",";
  j += "\"fps\":"             + String((float)app.streamFps, 1)   + ",";
  j += "\"sd\":"              + String(sdAvailable ? 1 : 0)       + ",";
  j += "\"tg\":"              + String(app.telegramConfigured ? 1 : 0) + ",";
  j += "\"clients\":"         + String(app.clientStreaming ? 1 : 0) + ",";
  struct tm t; char hb[16] = "--:--:--";
  if (getLocalTime(&t, 50)) strftime(hb, sizeof(hb), "%H:%M:%S", &t);
  j += "\"time\":\""          + String(hb) + "\"";
  j += "}";
  return j;
}

static esp_err_t status_handler(httpd_req_t* req) {
  String j = buildStatusJson();
  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, j.c_str(), j.length());
}

// ---------------------------------------------------------------------------
//  /control?var=&val=
// ---------------------------------------------------------------------------
static esp_err_t control_handler(httpd_req_t* req) {
  char var[32], val[32];
  if (!getParam(req, "var", var, sizeof(var)) || !getParam(req, "val", val, sizeof(val)))
    return httpd_resp_send_500(req);
  int v = atoi(val);
  sensor_t* s = esp_camera_sensor_get();
  int res = 0;

  if      (!strcmp(var, "framesize"))      res = s->set_framesize(s, (framesize_t)v);
  else if (!strcmp(var, "quality"))        res = s->set_quality(s, v);
  else if (!strcmp(var, "brightness"))     res = s->set_brightness(s, v);
  else if (!strcmp(var, "contrast"))       res = s->set_contrast(s, v);
  else if (!strcmp(var, "saturation"))     res = s->set_saturation(s, v);
  else if (!strcmp(var, "special_effect")) res = s->set_special_effect(s, v);
  else if (!strcmp(var, "hmirror"))        res = s->set_hmirror(s, v);
  else if (!strcmp(var, "vflip"))          res = s->set_vflip(s, v);
  else if (!strcmp(var, "flash"))          applyFlash(v);
  else if (!strcmp(var, "flash_bri"))    { app.flashBrightness = constrain(v, 0, 255); if (app.flashOn) applyFlash(true); }
  else if (!strcmp(var, "motion"))         app.motionEnabled = v;
  else if (!strcmp(var, "motion_sens"))    app.motionSensitivity = constrain(v, 1, 100);
  else if (!strcmp(var, "motion_cooldown"))app.motionCooldownMs = (uint32_t)constrain(v, 5, 600) * 1000UL;
  else if (!strcmp(var, "tg_alerts"))      app.telegramAlerts = v;
  else if (!strcmp(var, "sd_rec"))         app.sdRecordOnMotion = v;
  else if (!strcmp(var, "rec_clip"))       app.recordClipOnMotion = v;
  else res = -1;

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  if (res < 0) return httpd_resp_send_500(req);
  httpd_resp_set_type(req, "application/json");
  return httpd_resp_send(req, "{\"ok\":1}", 8);
}

// ---------------------------------------------------------------------------
//  /capture           -> JPEG (descarga)
//  /capture?save=1    -> guarda en SD y devuelve JSON
// ---------------------------------------------------------------------------
static esp_err_t capture_handler(httpd_req_t* req) {
  char save[8] = {0};
  bool toSD = getParam(req, "save", save, sizeof(save)) && atoi(save) == 1;

  uint8_t* jpg = nullptr; size_t len = 0;
  if (!grabJpegCopy(&jpg, &len)) return httpd_resp_send_500(req);

  esp_err_t res;
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  if (toSD) {
    String name; bool ok = sdAvailable && sdSaveJpeg(jpg, len, name);
    httpd_resp_set_type(req, "application/json");
    String r = ok ? "{\"ok\":1,\"file\":\"" + name + "\"}" : "{\"ok\":0,\"msg\":\"sin tarjeta SD\"}";
    res = httpd_resp_send(req, r.c_str(), r.length());
  } else {
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=captura.jpg");
    res = httpd_resp_send(req, (const char*)jpg, len);
  }
  free(jpg);
  return res;
}

// ---------------------------------------------------------------------------
//  /action?go=restart|tg_test
// ---------------------------------------------------------------------------
static esp_err_t action_handler(httpd_req_t* req) {
  char go[24] = {0};
  getParam(req, "go", go, sizeof(go));
  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  if (!strcmp(go, "restart")) {
    httpd_resp_send(req, "{\"ok\":1}", 8);
    delay(400); ESP.restart();
    return ESP_OK;
  }
  if (!strcmp(go, "tg_test")) {
    uint8_t* jpg = nullptr; size_t len = 0; bool ok = false;
    if (grabJpegCopy(&jpg, &len)) {
      ok = telegramSendPhoto(jpg, len, "✅ Foto de prueba · " + nowStamp());
      free(jpg);
    }
    String r = ok ? "{\"ok\":1}" : "{\"ok\":0,\"msg\":\"revisa token/chat_id\"}";
    return httpd_resp_send(req, r.c_str(), r.length());
  }
  if (!strcmp(go, "rec")) {
    bool ok = recorderStart("manual");
    String r = ok ? "{\"ok\":1}" : "{\"ok\":0,\"msg\":\"sin tarjeta SD o ya grabando\"}";
    return httpd_resp_send(req, r.c_str(), r.length());
  }
  return httpd_resp_send(req, "{\"ok\":0}", 8);
}

// ---------------------------------------------------------------------------
//  Galería SD: /sd/list  /sd/get?f=  /sd/del?f=
// ---------------------------------------------------------------------------
static esp_err_t sd_list_handler(httpd_req_t* req) {
  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  if (!sdAvailable) return httpd_resp_send(req, "{\"files\":[]}", 12);

  std::vector<String> names;
  File root = SD_MMC.open("/");
  if (root) {
    for (File f = root.openNextFile(); f; f = root.openNextFile()) {
      if (!f.isDirectory()) {
        String n = f.name();
        int sl = n.lastIndexOf('/'); if (sl >= 0) n = n.substring(sl + 1);
        if (n.endsWith(".jpg") || n.endsWith(".avi")) names.push_back(n);
      }
    }
    root.close();
  }
  std::sort(names.begin(), names.end(), std::greater<String>());   // más recientes primero

  String j = "{\"files\":[";
  size_t lim = names.size() < 60 ? names.size() : 60;
  for (size_t i = 0; i < lim; i++) { if (i) j += ","; j += "\"" + names[i] + "\""; }
  j += "]}";
  return httpd_resp_send(req, j.c_str(), j.length());
}

static esp_err_t sd_get_handler(httpd_req_t* req) {
  char f[80] = {0};
  if (!sdAvailable || !getParam(req, "f", f, sizeof(f)) || !safeName(f))
    return httpd_resp_send_404(req);

  File file = SD_MMC.open("/" + String(f));
  if (!file || file.isDirectory()) { if (file) file.close(); return httpd_resp_send_404(req); }

  httpd_resp_set_type(req, strstr(f, ".avi") ? "video/x-msvideo" : "image/jpeg");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  uint8_t buf[1024]; size_t n; esp_err_t res = ESP_OK;
  while ((n = file.read(buf, sizeof(buf))) > 0) {
    if (httpd_resp_send_chunk(req, (const char*)buf, n) != ESP_OK) { res = ESP_FAIL; break; }
  }
  file.close();
  httpd_resp_send_chunk(req, NULL, 0);
  return res;
}

static esp_err_t sd_del_handler(httpd_req_t* req) {
  char f[80] = {0};
  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  bool ok = sdAvailable && getParam(req, "f", f, sizeof(f)) && safeName(f) &&
            SD_MMC.remove("/" + String(f));
  return httpd_resp_send(req, ok ? "{\"ok\":1}" : "{\"ok\":0}", 8);
}

// ---------------------------------------------------------------------------
//  /stream (puerto 81) -> MJPEG
// ---------------------------------------------------------------------------
static esp_err_t stream_handler(httpd_req_t* req) {
  camera_fb_t* fb = NULL;
  esp_err_t res = ESP_OK;
  size_t jpg_len = 0; uint8_t* jpg_buf = NULL;
  char part_buf[80];

  res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
  if (res != ESP_OK) return res;
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "X-Framerate", "60");

  // Desactiva el algoritmo de Nagle en este socket: baja mucho la latencia
  // del MJPEG (envía los fotogramas sin esperar a agrupar paquetes).
  int sockfd = httpd_req_to_sockfd(req);
  if (sockfd >= 0) { int one = 1; setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one)); }

  app.clientStreaming = true;
  int64_t last = esp_timer_get_time();

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) { res = ESP_FAIL; }
    else if (fb->format != PIXFORMAT_JPEG) {
      bool ok = frame2jpg(fb, 80, &jpg_buf, &jpg_len);
      esp_camera_fb_return(fb); fb = NULL;
      if (!ok) res = ESP_FAIL;
    } else { jpg_buf = fb->buf; jpg_len = fb->len; }

    if (res == ESP_OK) res = httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));
    if (res == ESP_OK) {
      size_t hl = snprintf(part_buf, sizeof(part_buf), STREAM_PART, (unsigned)jpg_len);
      res = httpd_resp_send_chunk(req, part_buf, hl);
    }
    if (res == ESP_OK) res = httpd_resp_send_chunk(req, (const char*)jpg_buf, jpg_len);

    if (fb) { esp_camera_fb_return(fb); fb = NULL; jpg_buf = NULL; }
    else if (jpg_buf) { free(jpg_buf); jpg_buf = NULL; }

    if (res != ESP_OK) break;                 // cliente desconectado

    int64_t now = esp_timer_get_time();
    double dt = (now - last) / 1000000.0; last = now;
    if (dt > 0) app.streamFps = app.streamFps * 0.9f + (float)(0.1 / dt);
    app.framesServed++;
  }

  app.clientStreaming = false;
  app.streamFps = 0;
  return res;
}

// ---------------------------------------------------------------------------
//  Arranque de los dos servidores
// ---------------------------------------------------------------------------
void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 16;
  config.lru_purge_enable = true;
  config.server_port = 80;
  config.ctrl_port   = 32768;

  httpd_uri_t uris[] = {
    { .uri = "/",          .method = HTTP_GET, .handler = index_handler,   .user_ctx = NULL },
    { .uri = "/status",    .method = HTTP_GET, .handler = status_handler,  .user_ctx = NULL },
    { .uri = "/control",   .method = HTTP_GET, .handler = control_handler, .user_ctx = NULL },
    { .uri = "/capture",   .method = HTTP_GET, .handler = capture_handler, .user_ctx = NULL },
    { .uri = "/action",    .method = HTTP_GET, .handler = action_handler,  .user_ctx = NULL },
    { .uri = "/sd/list",   .method = HTTP_GET, .handler = sd_list_handler, .user_ctx = NULL },
    { .uri = "/sd/get",    .method = HTTP_GET, .handler = sd_get_handler,  .user_ctx = NULL },
    { .uri = "/sd/del",    .method = HTTP_GET, .handler = sd_del_handler,  .user_ctx = NULL },
  };

  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    for (auto& u : uris) httpd_register_uri_handler(camera_httpd, &u);
    Serial.println("[WEB] Servidor de control en :80");
  }

  // Streaming en su propio servidor/puerto para no bloquear el panel
  config.server_port = 81;
  config.ctrl_port   = 32769;
  httpd_uri_t stream_uri = { .uri = "/stream", .method = HTTP_GET, .handler = stream_handler, .user_ctx = NULL };
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
    Serial.println("[WEB] Servidor de streaming en :81");
  }
}
