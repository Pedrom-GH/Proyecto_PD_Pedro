// ===========================================================================
//  app_state.h  ·  Estado en tiempo de ejecución compartido entre módulos
//  y prototipos de las funciones públicas de cada módulo.
// ===========================================================================
#pragma once
#include <Arduino.h>
#include "esp_camera.h"

// --- Estado vivo de la aplicación (una sola instancia global: 'app') ---
struct AppState {
  // Ajustes de detección de movimiento
  volatile bool     motionEnabled;
  volatile int      motionSensitivity;     // 1..100
  volatile uint32_t motionCooldownMs;

  // Telegram / grabación
  volatile bool     telegramAlerts;
  volatile bool     sdRecordOnMotion;
  volatile bool     recordClipOnMotion;     // grabar clip AVI al detectar
  volatile bool     recording;              // hay una grabación en curso
  bool              telegramConfigured;     // hay token válido

  // Flash (LED RGB)
  volatile bool     flashOn;
  volatile uint8_t  flashBrightness;        // 0..255

  // Telemetría
  uint32_t          bootMillis;
  volatile uint32_t lastMotionMs;
  volatile uint32_t motionEvents;
  volatile uint32_t framesServed;
  volatile float    streamFps;
  volatile bool     clientStreaming;
};

extern AppState app;
extern bool      sdAvailable;

// --- Cámara / utilidades (definidas en main.cpp) ---
bool   grabJpegCopy(uint8_t** out, size_t* outLen);   // copia un JPEG (caller hace free)
void   applyFlash(bool on);
void   blinkStatus(uint8_t r, uint8_t g, uint8_t b, int ms);
String humanUptime();
String nowStamp();                                     // "2026-06-03_18-42-07"

// --- Servidor web (app_httpd.cpp) ---
void   startCameraServer();
String buildStatusJson();

// --- Detección de movimiento (motion.cpp) ---
void   motionInit();
bool   motionCheck();   // true SOLO cuando hay evento válido (ya respeta cooldown)

// --- Telegram (telegram_bot.cpp) ---
bool   telegramSendPhoto(const uint8_t* jpeg, size_t len, const String& caption);
bool   telegramSendText(const String& text);
void   telegramPollCommands();

// --- SD (main.cpp) ---
bool   sdSaveJpeg(const uint8_t* buf, size_t len, String& outName);

// --- Grabador de clips AVI (avi_recorder.cpp) ---
void   recorderInit();
bool   recorderStart(const String& reason);   // false si no se pudo iniciar
void   recorderService();                       // llamar en cada vuelta del loop
bool   recorderIsActive();
String recorderCurrentName();
