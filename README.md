# ESP32-S3 LED Breather Web Controller

An autonomous embedded web server and lighting controller running on the **ESP32-S3** microcontroller. Serves a modern, mobile-friendly web dashboard directly from onboard flash memory, allowing users to toggle an RGB LED between breathing **Green** and breathing **Red** in real time with continuous phase continuity.

---

## 1. Project Overview & Features

* **Respiratory Photometry**: Non-blocking `millis()` breathing loop producing an organic sine-based fade over a 4-second cycle.
* **Perceptual Gamma Correction**: Stevens / CIE 1931 gamma 2.2 curve mapping raw sine values to perceived brightness for smooth, uniform human eye response.
* **Continuous Phase Switching**: When switching between Green and Red, the time base is preserved so the LED continues its breath at the exact current duty cycle without jumping, blinking, or flickering.
* **Embedded Glass-Box Web Dashboard**: Responsive, mobile-first single-page web app embedded directly into flash memory via a PlatformIO pre-build script (`scripts/embed_web.py`).
* **Standalone Wi-Fi SoftAP**: Broadcasts an open access point (`ESP32-Breathe`, IP `http://192.168.4.1/`) so you can connect and control the LED anywhere without an external router.
* **Optional Router Connection + mDNS**: Connects to your home Wi-Fi via git-ignored `secrets.h` and broadcasts `http://esp32.local/` on your network.
* **Local Mock Server**: Includes `tools/mock_server.py` to serve the exact same HTML and fake the REST API on your computer without needing physical hardware connected.

---

## 2. Knowledge Carried Over vs. Dropped

Derived from the previous Linea hardware implementation (`/Users/pareshprabhuraj/Downloads/Linea/hardware`):

### Carried Over
* **Target Microcontroller**: ESP32-S3 Dev Module with Native USB CDC (`/dev/cu.usbmodem101`).
* **Pin Assignments**: GPIO 4 (Red anode via 220R), GPIO 5 (Green anode via 220R), GND (Common Cathode).
* **Hardware PWM**: 5000 Hz, 8-bit resolution LEDC peripheral driver.
* **USB CDC Compiler Flags**: `-D ARDUINO_USB_CDC_ON_BOOT=1` and `-D ARDUINO_USB_MODE=1` for native USB serial output.

### Dropped
* Linea dispatch SSE event stream.
* Twilio voice calling and driver relay pipelines.
* Mock agent logic and logistics truck simulation.

---

## 3. Hardware Architecture & Wiring Table

All components are standard parts from the UCSD ECE 5 lab kit.

### Bill of Materials (BOM)
* 1x ESP32-S3 Development Board (Native USB-C interface)
* 1x 4-Pin Discrete RGB LED (Common Cathode)
* 2x 220 ohm to 330 ohm current-limiting resistors (Red-Red-Brown or Orange-Orange-Brown)
* 1x Half-size solderless breadboard
* 3x Male-to-Male jumper wires
* 1x USB-C data cable

### 4-Pin RGB LED Pinout

Hold the LED up to the light and locate the flat notch on the collar:

```
          +-----------+
         /             \
        |   (LENS)      |
        |               |
         \             /  <- Flat edge on left
          +-----+-----+
           |  |  |  |
           |  |  |  |
           |  |  |  |
           1  2  3  4
```

* **Pin 1 (Red)**: Adjacent to flat collar notch.
* **Pin 2 (Common Cathode / GND)**: The LONGEST metal lead.
* **Pin 3 (Green)**: Adjacent to longest lead.
* **Pin 4 (Blue)**: Rightmost outer lead (optional / unused for two-color breathing).

### Breadboard Wiring Matrix

| LED Pin | Channel | Circuit Component | ESP32-S3 GPIO | Rationale |
|---|---|---|---|---|
| **Pin 1** | Red | 220 ohm Resistor in series | **GPIO 4** | Safe ADC1 pin, no bootloader/strapping conflicts |
| **Pin 2** | Common Cathode | Direct Jumper | **GND** | Direct ground return for common cathode |
| **Pin 3** | Green | 220 ohm Resistor in series | **GPIO 5** | Safe ADC1 pin, no bootloader/strapping conflicts |

> **Safety Notice**: Avoid GPIO 26 through 32 (integrated SPI flash) and GPIO 0, 3, 45, 46 (strapping pins). GPIO 4 and 5 are completely safe general-purpose I/O pins.

---

## 4. REST API Specification

| Method | Endpoint | Description | Payload / Response |
|---|---|---|---|
| `GET` | `/` | Serves the responsive web UI | HTML (`text/html`) |
| `GET` | `/api/state` | Returns the current LED breathing color | `{"color":"green"}` or `{"color":"red"}` |
| `POST` | `/api/toggle` | Toggles between Green and Red | `{"color":"red"}` |
| `POST` | `/api/color` | Explicitly sets color (`green` or `red`) | `{"color":"green"}` |

---

## 5. Build, Upload & Monitor Commands

The project uses PlatformIO with pinned packages in `platformio.ini`.

### Build Firmware
```bash
~/.local/bin/pio run
```

### Upload to ESP32-S3
```bash
~/.local/bin/pio run --target upload --upload-port /dev/cu.usbmodem101
```

### Open Serial Monitor (115200 Baud)
```bash
~/.local/bin/pio device monitor -b 115200
```

---

## 6. How to Connect

### Method A: Standalone SoftAP (Default)
1. On your phone or laptop, open Wi-Fi settings.
2. Connect to the open network: **`ESP32-Breathe`**.
3. Open your browser and navigate to:
   ```
   http://192.168.4.1/
   ```

### Method B: Home / Office Router (Optional)
1. Copy `include/secrets.example.h` to `include/secrets.h`:
   ```bash
   cp include/secrets.example.h include/secrets.h
   ```
2. Enter your Wi-Fi credentials in `include/secrets.h`.
3. Re-flash the board. The ESP32-S3 will connect to your router and serve mDNS at:
   ```
   http://esp32.local/
   ```

---

## 7. Local Mock Server (Testing Without Hardware)

You can run and test the complete web interface on your computer without an ESP32:

```bash
python3 tools/mock_server.py
```
Open **`http://localhost:8080/`** in your browser. The mock server fakes the exact ESP32 API endpoints and logs color changes directly to your terminal.

---

## 8. Troubleshooting

* **Board Not Detected (`/dev/cu.usbmodem101` missing)**:
  Ensure your USB-C cable supports data transfer (some cables are charge-only).
* **Upload Hangs at `Connecting...`**:
  Put the ESP32-S3 into ROM bootloader mode:
  1. Press and **HOLD the `BOOT` button** (labelled `BOOT` or `IO0`).
  2. Press and release the **`RST` button ONCE** (labelled `RST` or `EN`).
  3. **Release the `BOOT` button**.
  4. Run `~/.local/bin/pio run --target upload`.
* **LED Stuck ON or OFF**:
  If using Common Anode instead of Common Cathode, set `#define COMMON_ANODE 1` in `include/config.h` and re-compile.

---

## 9. File Architecture

```
esp32-breathe-web/
├── platformio.ini         # Pinned platform & board configuration
├── include/
│   ├── config.h           # Pins, timing, brightness cap, Wi-Fi parameters
│   ├── led_breather.h     # Non-blocking breather class & pure math functions
│   ├── web_api.h          # WebServer route declarations & CORS handlers
│   ├── secrets.example.h  # Router credential template (secrets.h is git-ignored)
│   └── web_index_html.h   # Auto-generated embedded HTML header
├── src/
│   ├── main.cpp           # Setup and non-blocking loop
│   ├── led_breather.cpp   # PWM driver & gamma curve implementation
│   └── web_api.cpp        # HTTP endpoint handlers
├── web/
│   └── index.html         # Single source of truth for responsive web UI
├── tools/
│   └── mock_server.py     # Zero-dependency Python mock server
└── scripts/
    └── embed_web.py       # Pre-build hook embedding HTML into C++
```
