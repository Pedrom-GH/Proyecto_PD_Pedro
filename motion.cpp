// ===========================================================================
//  motion.cpp · Detección de movimiento por diferencia de fotogramas
//  Decodifica el JPEG a baja resolución, lo reduce a una rejilla de brillo
//  (32x24) y compara con el fotograma anterior. Ligero y robusto.
// ===========================================================================
#include <Arduino.h>
#include <string.h>
#include "esp_camera.h"
#include "img_converters.h"
#include "app_state.h"

#define GRID_W 32
#define GRID_H 24
#define CELLS  (GRID_W * GRID_H)
#define CELL_DIFF_THRESH 18        // diferencia de brillo (0-255) para "celda cambiada"

static uint8_t  prevGrid[CELLS];
static bool     primed = false;
static uint32_t cooldownUntil = 0;

// buffers de trabajo en estático para no cargar la pila del loop
static uint32_t sumBuf[CELLS];
static uint16_t cntBuf[CELLS];

void motionInit() {
  primed = false;
  memset(prevGrid, 0, sizeof(prevGrid));
  cooldownUntil = 0;
}

// Construye la rejilla de brillo del fotograma actual. Devuelve false si no
// se pudo (sin frame, resolución insuficiente, sin memoria...).
static bool buildGrid(uint8_t* grid) {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) return false;
  if (fb->format != PIXFORMAT_JPEG) { esp_camera_fb_return(fb); return false; }

  int w = fb->width, h = fb->height;
  size_t len = fb->len;
  uint8_t* jpg = (uint8_t*)ps_malloc(len);
  if (!jpg) { esp_camera_fb_return(fb); return false; }
  memcpy(jpg, fb->buf, len);
  esp_camera_fb_return(fb);          // libera el framebuffer enseguida

  int dw = w / 8, dh = h / 8;        // dimensiones tras JPG_SCALE_8X
  if (dw < GRID_W || dh < GRID_H) { free(jpg); return false; }

  uint8_t* rgb = (uint8_t*)ps_malloc((size_t)(dw + 2) * (dh + 2) * 2);   // margen defensivo
  if (!rgb) { free(jpg); return false; }
  bool ok = jpg2rgb565(jpg, len, rgb, JPG_SCALE_8X);
  free(jpg);
  if (!ok) { free(rgb); return false; }

  memset(sumBuf, 0, sizeof(sumBuf));
  memset(cntBuf, 0, sizeof(cntBuf));
  for (int y = 0; y < dh; y++) {
    int gy = y * GRID_H / dh;
    for (int x = 0; x < dw; x++) {
      int gx = x * GRID_W / dw;
      int p = (y * dw + x) * 2;
      uint16_t px = ((uint16_t)rgb[p] << 8) | rgb[p + 1];
      uint8_t r = ((px >> 11) & 0x1F) << 3;
      uint8_t g = ((px >> 5)  & 0x3F) << 2;
      uint8_t b = ( px        & 0x1F) << 3;
      uint8_t gray = (uint8_t)((r * 30 + g * 59 + b * 11) / 100);
      int idx = gy * GRID_W + gx;
      sumBuf[idx] += gray; cntBuf[idx]++;
    }
  }
  free(rgb);
  for (int i = 0; i < CELLS; i++)
    grid[i] = cntBuf[i] ? (uint8_t)(sumBuf[i] / cntBuf[i]) : 0;
  return true;
}

// Devuelve true SOLO cuando hay un evento válido (ya gestiona el cooldown).
bool motionCheck() {
  uint32_t now = millis();
  uint8_t grid[CELLS];
  if (!buildGrid(grid)) return false;

  if (!primed) { memcpy(prevGrid, grid, CELLS); primed = true; return false; }

  int changed = 0;
  for (int i = 0; i < CELLS; i++) {
    int d = (int)grid[i] - (int)prevGrid[i];
    if (d < 0) d = -d;
    if (d > CELL_DIFF_THRESH) changed++;
  }
  memcpy(prevGrid, grid, CELLS);

  // sensibilidad (1..100) -> fracción de celdas que deben cambiar
  float t = (app.motionSensitivity - 1) / 99.0f;     // 0..1
  float frac = 0.22f * (1.0f - t) + 0.012f;           // ~1.2% (100) .. ~23% (1)
  int need = (int)(CELLS * frac);
  if (need < 2) need = 2;

  if (changed >= need && now >= cooldownUntil) {
    cooldownUntil = now + app.motionCooldownMs;
    return true;
  }
  return false;
}
