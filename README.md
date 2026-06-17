# GOOUUU ESP32-S3-CAM · SENTINEL

Cámara de seguridad completa para la placa **GOOUUU ESP32-S3-CAM** (ESP32-S3-WROOM-1 N16R8 + OV2640/OV5640).
Sirve un **panel web propio** con vídeo en directo fluido (MJPEG), control total de la cámara,
**detección de movimiento** y **avisos por Telegram** con foto, además de grabación en **microSD**.

---

## ✨ Funcionalidades

- **Streaming MJPEG fluido** en un servidor dedicado (puerto 81) para no bloquear el panel.
- **Panel web "sala de control"** servido desde la propia placa (puerto 80), responsive y animado.
- **Control de cámara en vivo**: resolución (QVGA→UXGA), calidad JPEG, brillo, contraste, saturación, efectos, espejo/volteo.
- **Detección de movimiento** por diferencia de fotogramas (rejilla de brillo), con sensibilidad y "tiempo de espera entre alertas" ajustables.
- **Grabación de clips de vídeo** en **AVI Motion-JPEG** a la microSD (al detectar movimiento o con el botón manual), reproducibles en VLC.
- **Telegram**: envía una **foto automática** al detectar movimiento + comandos del bot (`/foto`, `/estado`, `/vigilancia_on`...).
- **Captura instantánea** (descarga) y **guardado en microSD** con marca de tiempo.
- **Galería SD** integrada (fotos y vídeos: ver / abrir / borrar).
- **Extras de visor elegantes**: zoom digital con paneo (arrastrar), pantalla completa, rejilla de encuadre (regla de los tercios), HUD con estado en vivo.
- **Atajos de teclado**: `Espacio` directo · `S` captura · `R` clip · `F` pantalla completa · `G` rejilla.
- **Registro de eventos** en vivo dentro del panel (movimiento, envíos a Telegram, grabaciones).
- **Flash** mediante el LED RGB integrado (GPIO48) con **intensidad ajustable** y **LED de estado**.
- **Telemetría** en vivo: uptime, RAM/PSRAM libres, RSSI, FPS, nº de eventos.
- **mDNS** (`http://goouuu-cam.local`) y **NTP** (hora real en nombres de archivo y pies de foto).

---

## 🛠 Requisitos

- **PlatformIO** (extensión en VS Code).
- Placa GOOUUU ESP32-S3-CAM con la cámara conectada al conector FFC.
- (Opcional) tarjeta microSD para grabar.
- Cuenta de Telegram y un bot (gratis).

> El proyecto usa el core **arduino-esp32** que PlatformIO instala solo. Recomendado core ≥ 2.0.5 (o 3.x).

---

## 🚀 Puesta en marcha

1. **Abre la carpeta** del proyecto en VS Code (con PlatformIO instalado).
2. Edita **`include/config.h`** y rellena:
   - `WIFI_SSID` y `WIFI_PASSWORD`.
   - `TELEGRAM_BOT_TOKEN` (de **@BotFather**) y `TELEGRAM_CHAT_ID`.
     - Para el chat id: escribe algo a tu bot y abre en el navegador
       `https://api.telegram.org/bot<TU_TOKEN>/getUpdates` → busca `"chat":{"id": ...}`.
   - Si no quieres Telegram, deja el token tal cual; se desactiva solo.
3. Conecta la placa por el **puerto USB-C marcado `TTL`** (el del puente serie **CH340C**), **no** el `OTG`.
4. Pulsa **Upload** (flecha →) en la barra de PlatformIO.
5. Abre el **Monitor Serie** (115200). Verás la IP, por ejemplo:
   ```
   [NET] Accede en: http://goouuu-cam.local  o  http://192.168.1.50
   ```
6. Abre esa dirección en el móvil/PC (misma red WiFi) y pulsa **▶ Iniciar**.

### Si no flashea / no hay vídeo
- **No flashea**: mantén pulsado **BOOT**, pulsa y suelta **RST**, suelta **BOOT**, y vuelve a subir.
- **No ves el Monitor Serie**: usa el puerto **`TTL`** (CH340C). Si conectas por el **`OTG`** (USB nativo),
  cambia en `platformio.ini` la bandera a `-DARDUINO_USB_CDC_ON_BOOT=1`.
- **"Camera init failed"**: revisa que el cable FFC de la cámara está bien insertado y en el sentido correcto.
- **El vídeo no carga pero el panel sí**: el stream va por el **puerto 81**; asegúrate de que tu red no bloquea ese puerto.

---

## 📲 Comandos del bot de Telegram

| Comando | Acción |
|---|---|
| `/foto` | Envía una foto al instante |
| `/estado` | IP, WiFi, uptime, estado de vigilancia/alertas |
| `/vigilancia_on` · `/vigilancia_off` | Activa/desactiva la detección de movimiento |
| `/alertas_on` · `/alertas_off` | Activa/desactiva los avisos por movimiento |
| `/flash_on` · `/flash_off` | Enciende/apaga el flash (LED RGB) |
| `/ayuda` | Lista de comandos |

> Si notas que el vídeo se entrecorta, sube `TELEGRAM_POLL_INTERVAL_MS` o pon
> `ENABLE_TELEGRAM_COMMANDS false` en `config.h` (los avisos por movimiento seguirán funcionando).

---

## 🎥 Vídeo, capturas y galería

- **Captura** (`⬇ Captura`): descarga una foto al instante en tu dispositivo.
- **Guardar SD** (`💾`): guarda la foto en la microSD.
- **Grabar clip** (`● Grabar clip` o tecla `R`): graba un clip **AVI** de `CLIP_DURATION_S` segundos a `CLIP_FPS` fps en la SD.
  También se graba automáticamente al detectar movimiento si activas *"Grabar clip al detectar"*.
- **Galería SD**: muestra fotos (miniatura) y vídeos (icono 🎞). Toca para abrir/descargar; la **×** borra.
- Los `.avi` Motion-JPEG se reproducen en **VLC** y la mayoría de reproductores. Para clips fluidos usa
  resolución **≤ SVGA** (la SD en modo 1-bit no da para grabar UXGA a buen ritmo).
- **Requiere microSD** insertada en la ranura de la placa (FAT32). Sin ella, los clips y el guardado en SD se desactivan solos.

## 🖱 Extras del visor

- **Zoom digital**: usa el deslizador 🔍; con zoom >1, **arrastra** la imagen para mover el encuadre, **doble clic** para resetear.
- **Pantalla completa** (`⛶` o tecla `F`) y **rejilla de encuadre** (`▦` o tecla `G`).
- **Registro de eventos** en vivo en el panel (movimiento, envíos a Telegram, grabaciones).

## 🧠 Cómo funciona (resumen técnico)

- **Dos servidores `esp_http_server`**: control en `:80`, streaming en `:81`. Es el método más estable y fluido en el S3.
- **Detección de movimiento**: cada ~450 ms se decodifica un fotograma a 1/8 de resolución (`jpg2rgb565`),
  se reduce a una rejilla de brillo 32×24 y se compara con el fotograma anterior. Si cambian suficientes
  celdas (según la sensibilidad) y se respeta el *cooldown*, se dispara el evento.
- **Telegram**: las fotos se mandan por HTTPS (`multipart/form-data`). El fotograma se **copia** y se libera
  el buffer de la cámara antes de subirlo, para que el streaming **no se congele** durante el envío.
- **Pines**: ver `include/camera_pins.h` (idénticos al modelo `ESP32-S3-EYE`, verificados para la GOOUUU).

---

## 📁 Estructura

```
platformio.ini            Configuración de la placa (N16R8: PSRAM OPI + Flash QIO)
include/
  config.h                ← EDITA AQUÍ tus credenciales y ajustes
  camera_pins.h           Pinout de la cámara / SD / LED
  app_state.h             Estado compartido + prototipos
  web_index.h             Panel web embebido (HTML/CSS/JS)
src/
  main.cpp                Arranque, cámara, WiFi, SD, NTP, bucle principal
  app_httpd.cpp           Servidores web y manejadores
  motion.cpp              Detección de movimiento
  avi_recorder.cpp        Grabación de clips AVI Motion-JPEG (no bloqueante)
  telegram_bot.cpp        Envío de fotos/textos y comandos del bot
```

---

## ⚠️ Notas

- Para un proyecto académico, Telegram usa `setInsecure()` (no valida el certificado TLS). Es suficiente y habitual,
  pero tenlo en cuenta si lo llevas a producción.
- La detección de movimiento es por software (no usa la NPU). Es ligera y suficiente para vigilancia doméstica.
- La microSD usa modo **SDMMC 1-bit** (pines 38/39/40). Formatéala en **FAT32** si no la detecta.
