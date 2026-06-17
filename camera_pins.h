// ===========================================================================
//  camera_pins.h
//  Pinout de la cámara para la GOOUUU ESP32-S3-CAM.
//  Es IDÉNTICO al modelo CAMERA_MODEL_ESP32S3_EYE (verificado con la doc de la
//  placa). NO lo cambies salvo que tengas una variante distinta.
// ===========================================================================
#pragma once

#define PWDN_GPIO_NUM   -1   // No usado (la cámara no tiene power-down dedicado)
#define RESET_GPIO_NUM  -1   // Reset por software
#define XCLK_GPIO_NUM   15
#define SIOD_GPIO_NUM    4   // SCCB SDA
#define SIOC_GPIO_NUM    5   // SCCB SCL

#define Y9_GPIO_NUM     16   // D7
#define Y8_GPIO_NUM     17   // D6
#define Y7_GPIO_NUM     18   // D5
#define Y6_GPIO_NUM     12   // D4
#define Y5_GPIO_NUM     10   // D3
#define Y4_GPIO_NUM      8   // D2
#define Y3_GPIO_NUM      9   // D1
#define Y2_GPIO_NUM     11   // D0

#define VSYNC_GPIO_NUM   6
#define HREF_GPIO_NUM    7
#define PCLK_GPIO_NUM   13

// --- LED RGB NeoPixel integrado (se usa como "flash") ---
#define RGB_LED_PIN     48

// --- Tarjeta microSD (SD_MMC en modo 1 bit) ---
#define SD_MMC_CLK_PIN  39
#define SD_MMC_CMD_PIN  38
#define SD_MMC_D0_PIN   40
