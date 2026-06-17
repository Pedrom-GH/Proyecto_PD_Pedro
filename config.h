// ===========================================================================
//  config.h  ·  CONFIGURACIÓN DEL USUARIO
//  --> Edita SOLO este archivo para poner tus credenciales y ajustes.
// ===========================================================================
#pragma once

// ---------------------------------------------------------------------------
// 1) WiFi
// ---------------------------------------------------------------------------
#define WIFI_SSID       "iPhone de Pedro"
#define WIFI_PASSWORD   "pedro1000"

// Nombre de red local. Podrás abrir la cámara en:  http://goouuu-cam.local
#define DEVICE_HOSTNAME "goouuu-cam"

// ---------------------------------------------------------------------------
// 2) TELEGRAM
//    - Crea un bot con @BotFather y copia el TOKEN.
//    - Para el CHAT_ID: escribe a tu bot y visita en el navegador
//      https://api.telegram.org/bot<TOKEN>/getUpdates  -> busca "chat":{"id":...}
//    Si dejas el token a "" se desactiva Telegram automáticamente.
// ---------------------------------------------------------------------------
#define TELEGRAM_BOT_TOKEN  "8800248763:AAHygRpvHEMtBvKl8T07kn_mXOc-IspTay0"
#define TELEGRAM_CHAT_ID    "1017662437"

// Responder a comandos enviados al bot (/foto, /estado, /alertas_on ...).
// Si notas que el vídeo se entrecorta, sube TELEGRAM_POLL_INTERVAL_MS o pon
// ENABLE_TELEGRAM_COMMANDS en false.
#define ENABLE_TELEGRAM_COMMANDS    true
#define TELEGRAM_POLL_INTERVAL_MS   4000

// ---------------------------------------------------------------------------
// 3) ZONA HORARIA (para los nombres de archivo y los pies de foto)
//    Por defecto: España peninsular (CET/CEST con cambio horario).
// ---------------------------------------------------------------------------
#define TIMEZONE_TZ     "CET-1CEST,M3.5.0,M10.5.0/3"
#define NTP_SERVER_1    "pool.ntp.org"
#define NTP_SERVER_2    "time.nist.gov"

// ---------------------------------------------------------------------------
// 4) VALORES POR DEFECTO al arrancar (se pueden cambiar luego desde la web)
// ---------------------------------------------------------------------------
#define DEFAULT_MOTION_ENABLED      false   // detección de movimiento al inicio
#define DEFAULT_MOTION_SENSITIVITY  55      // 1 (poco) .. 100 (mucho)
#define DEFAULT_MOTION_COOLDOWN_MS  20000   // tiempo mínimo entre alertas
#define DEFAULT_TELEGRAM_ALERTS     true    // enviar foto a Telegram al detectar
#define DEFAULT_SD_RECORD_ON_MOTION true    // guardar también la foto en la SD

// Cada cuánto se analiza el movimiento (ms). Más bajo = más reactivo, más CPU.
#define MOTION_CHECK_INTERVAL_MS    450

// ---------------------------------------------------------------------------
// 5) GRABACIÓN DE CLIPS DE VÍDEO (AVI Motion-JPEG en la microSD)
//    Requiere una tarjeta microSD en la ranura de la placa.
//    Consejo: para clips fluidos usa resolución <= SVGA (la SD en modo 1-bit
//    no da para grabar UXGA a buen ritmo).
// ---------------------------------------------------------------------------
#define DEFAULT_RECORD_CLIP_ON_MOTION true   // grabar un clip al detectar movimiento
#define CLIP_DURATION_S             6        // duración del clip (segundos)
#define CLIP_FPS                    12       // fotogramas por segundo del clip
