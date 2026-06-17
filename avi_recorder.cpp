// ===========================================================================
//  avi_recorder.cpp · Grabación de clips de vídeo en formato AVI Motion-JPEG
//  No bloqueante: graba un fotograma por vuelta del loop según CLIP_FPS, así
//  el streaming y Telegram siguen funcionando. Al terminar, parchea la cabecera
//  AVI (tamaños, nº de fotogramas) y escribe el índice idx1.
//  El archivo .avi resultante se reproduce en VLC y la mayoría de reproductores.
// ===========================================================================
#include <Arduino.h>
#include <SD_MMC.h>
#include <FS.h>
#include <vector>
#include "esp_camera.h"
#include "config.h"
#include "app_state.h"

static File     avi;
static bool     active = false;
static String   curName;
static uint32_t fps = CLIP_FPS;
static uint32_t frameInterval = 1000 / CLIP_FPS;
static uint32_t clipMs = (uint32_t)CLIP_DURATION_S * 1000UL;
static uint32_t recStart = 0, lastFrame = 0;
static uint32_t frameCount = 0;
static uint32_t moviStart = 0;          // posición del fourcc "movi"

// posiciones a parchear al cerrar
static uint32_t posRiffSize, posTotalFrames, posStrhLength, posMoviSize;

// índice de fotogramas (offset relativo a "movi" + tamaño)
struct IdxEntry { uint32_t off; uint32_t len; };
static std::vector<IdxEntry> idx;

// ---- helpers de escritura little-endian ----
static void wU32(uint32_t v) { uint8_t b[4]={(uint8_t)v,(uint8_t)(v>>8),(uint8_t)(v>>16),(uint8_t)(v>>24)}; avi.write(b,4); }
static void wU16(uint16_t v) { uint8_t b[2]={(uint8_t)v,(uint8_t)(v>>8)}; avi.write(b,2); }
static void wTag(const char* t){ avi.write((const uint8_t*)t,4); }
static void patchU32(uint32_t pos, uint32_t v) {
  uint32_t cur = avi.position();
  avi.seek(pos); wU32(v); avi.seek(cur);
}

void recorderInit() { active = false; }
bool recorderIsActive() { return active; }
String recorderCurrentName() { return curName; }

// ---- escribe la cabecera AVI con los pines (placeholders) a 0 ----
static void writeHeader(uint16_t w, uint16_t h) {
  wTag("RIFF"); posRiffSize = avi.position(); wU32(0); wTag("AVI ");

  wTag("LIST"); uint32_t posHdrlSize = avi.position(); wU32(0);
  uint32_t hdrlStart = avi.position();
  wTag("hdrl");
    wTag("avih"); wU32(56);
      wU32(1000000UL / fps);   // dwMicroSecPerFrame
      wU32(0);                 // dwMaxBytesPerSec
      wU32(0);                 // dwPaddingGranularity
      wU32(0x10);              // dwFlags = AVIF_HASINDEX
      posTotalFrames = avi.position(); wU32(0);  // dwTotalFrames (parchear)
      wU32(0);                 // dwInitialFrames
      wU32(1);                 // dwStreams
      wU32(0);                 // dwSuggestedBufferSize
      wU32(w); wU32(h);
      wU32(0); wU32(0); wU32(0); wU32(0);        // reserved[4]
    wTag("LIST"); uint32_t posStrlSize = avi.position(); wU32(0);
    uint32_t strlStart = avi.position();
    wTag("strl");
      wTag("strh"); wU32(56);
        wTag("vids"); wTag("MJPG");
        wU32(0); wU16(0); wU16(0);   // flags, priority, language
        wU32(0);                     // dwInitialFrames
        wU32(1);                     // dwScale
        wU32(fps);                   // dwRate (fps con scale=1)
        wU32(0);                     // dwStart
        posStrhLength = avi.position(); wU32(0);  // dwLength = nº frames (parchear)
        wU32(0);                     // dwSuggestedBufferSize
        wU32(0xFFFFFFFF);            // dwQuality
        wU32(0);                     // dwSampleSize
        wU16(0); wU16(0); wU16(w); wU16(h);       // rcFrame
      wTag("strf"); wU32(40);
        wU32(40);                    // biSize
        wU32(w); wU32(h);
        wU16(1); wU16(24);           // planes, bitcount
        wTag("MJPG");                // biCompression
        wU32((uint32_t)w * h * 3);   // biSizeImage
        wU32(0); wU32(0); wU32(0); wU32(0);
    uint32_t strlEnd = avi.position();
    patchU32(posStrlSize, strlEnd - strlStart);
  uint32_t hdrlEnd = avi.position();
  patchU32(posHdrlSize, hdrlEnd - hdrlStart);

  wTag("LIST"); posMoviSize = avi.position(); wU32(0);
  moviStart = avi.position();        // apunta al fourcc "movi"
  wTag("movi");
}

bool recorderStart(const String& reason) {
  if (active || !sdAvailable) return false;

  // primer fotograma: para conocer las dimensiones reales
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb || fb->format != PIXFORMAT_JPEG) { if (fb) esp_camera_fb_return(fb); return false; }
  uint16_t w = fb->width, h = fb->height;

  curName = "vid_" + nowStamp() + ".avi";
  avi = SD_MMC.open("/" + curName, FILE_WRITE);
  if (!avi) { esp_camera_fb_return(fb); return false; }

  fps = CLIP_FPS; frameInterval = 1000 / CLIP_FPS;
  frameCount = 0; idx.clear();
  writeHeader(w, h);

  // escribe el primer fotograma
  IdxEntry e = { (uint32_t)(avi.position() - moviStart), fb->len };
  wTag("00dc"); wU32(fb->len); avi.write(fb->buf, fb->len);
  if (fb->len & 1) avi.write((uint8_t)0);
  idx.push_back(e); frameCount++;
  esp_camera_fb_return(fb);

  recStart = millis(); lastFrame = recStart;
  active = true; app.recording = true;
  Serial.println("[REC] Iniciado: " + curName + " (" + reason + ")");
  return true;
}

static void finalize() {
  uint32_t moviEnd = avi.position();        // fin de los fotogramas (antes del idx1)
  // índice idx1
  wTag("idx1"); wU32(idx.size() * 16);
  for (auto& e : idx) {
    wTag("00dc");
    wU32(0x10);          // AVIIF_KEYFRAME
    wU32(e.off);
    wU32(e.len);
  }
  uint32_t fileEnd = avi.position();
  patchU32(posMoviSize, moviEnd - moviStart);   // tamaño del LIST "movi"
  patchU32(posRiffSize, fileEnd - 8);
  patchU32(posTotalFrames, frameCount);
  patchU32(posStrhLength, frameCount);
  avi.close();
  active = false; app.recording = false;
  Serial.printf("[REC] Guardado %s · %lu fotogramas\n", curName.c_str(), (unsigned long)frameCount);
}

void recorderService() {
  if (!active) return;
  uint32_t now = millis();

  if (now - recStart >= clipMs) { finalize(); return; }   // fin por tiempo

  if (now - lastFrame >= frameInterval) {
    lastFrame = now;
    camera_fb_t* fb = esp_camera_fb_get();
    if (fb && fb->format == PIXFORMAT_JPEG) {
      IdxEntry e = { (uint32_t)(avi.position() - moviStart), fb->len };
      wTag("00dc"); wU32(fb->len); avi.write(fb->buf, fb->len);
      if (fb->len & 1) avi.write((uint8_t)0);
      idx.push_back(e); frameCount++;
    }
    if (fb) esp_camera_fb_return(fb);
  }
}
