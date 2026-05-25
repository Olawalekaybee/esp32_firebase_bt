# ESP32 — Firebase + Bluetooth Classic + FreeRTOS

A production-ready, reusable ESP32 firmware skeleton that runs
**Firebase Firestore uploads**, **Bluetooth Classic SPP**, and a
**sensor-sampling loop** as independent FreeRTOS tasks with zero
cross-task blocking.

---

## Project structure

```
esp32_firebase_bt/
├── platformio.ini          ← PlatformIO config & library deps
├── include/
│   ├── config.h            ← ⭐ ALL user settings live here
│   ├── sensor_data.h       ← Shared data structure
│   ├── wifi_manager.h
│   ├── firebase_manager.h
│   ├── bt_manager.h
│   └── sensor_task.h
└── src/
    ├── main.cpp            ← Entry point
    ├── wifi_manager.cpp
    ├── firebase_manager.cpp
    ├── firebase_task.cpp   ← RTOS task wrapper
    ├── bt_manager.cpp
    ├── bt_task.cpp         ← RTOS task wrapper
    └── sensor_task.cpp     ← RTOS task wrapper
```

---

## Quick start

### 1 — Prerequisites

| Tool | Version |
|------|---------|
| VS Code | Latest |
| PlatformIO extension | Latest |
| ESP32 Arduino platform | ≥ 6.x |

### 2 — Clone / open

```bash
cd your-workspace
# open esp32_firebase_bt/ in VS Code
```

### 3 — Configure `include/config.h`

```c
#define WIFI_SSID          "your_ssid"
#define WIFI_PASSWORD      "your_password"

#define FIREBASE_API_KEY       "your_web_api_key"
#define FIREBASE_PROJECT_ID    "your_project_id"
#define FIREBASE_USER_EMAIL    "user@example.com"
#define FIREBASE_USER_PASSWORD "secret"
```

### 4 — Firebase setup

1. Create a Firebase project at <https://console.firebase.google.com>.
2. Enable **Firestore** (Native mode).
3. Enable **Email/Password** authentication and add the credentials above.
4. Copy the **Web API Key** from Project Settings → General.

### 5 — Build & flash

```bash
pio run --target upload
pio device monitor   # 115200 baud
```

---

## Adapting to your sensor

Open `src/sensor_task.cpp` and replace the stub section:

```cpp
// ─── BEGIN real sensor code ───
r.temperature = bme.readTemperature();
r.humidity    = bme.readHumidity();
r.pressure    = bme.readPressure() / 100.0F;
// ─── END real sensor code ─────
```

Add your library to `platformio.ini` → `lib_deps`.

---

## Architecture

```
Core 0 (PRO_CPU)                Core 1 (APP_CPU)
────────────────                ────────────────
WiFi / BT stack                 SensorTask
FirebaseTask                      │
BluetoothTask                     ▼
     │               OfflineQueue (mutex-protected)
     │◄──────────────────────────┘
     │
     ▼
Firestore (when online)
BT Client (always, if connected)
```

### Offline behaviour

Sensor readings are always written to an in-memory circular queue
(`OFFLINE_QUEUE_DEPTH` entries, default 50).  When WiFi is unavailable
the queue fills up; when connectivity is restored the FirebaseTask
drains it automatically — **no data is lost** up to the queue depth.

---

## Tuning

All tuneable knobs are in `config.h` and `platformio.ini`:

| Setting | Default | Description |
|---------|---------|-------------|
| `SENSOR_SAMPLE_INTERVAL_MS` | 1 000 | Sample rate |
| `FIREBASE_PUSH_INTERVAL_MS` | 5 000 | Push cadence |
| `OFFLINE_QUEUE_DEPTH` | 50 | Max buffered readings |
| `TASK_STACK_FIREBASE` | 8 192 | Firebase task stack (bytes) |
| `TASK_STACK_BLUETOOTH` | 6 144 | BT task stack (bytes) |
| `TASK_STACK_SENSOR` | 4 096 | Sensor task stack (bytes) |
| `TASK_PRIO_*` | 1-3 | FreeRTOS task priorities |

---

## Extending

- **New sensor fields** — Add fields to `SensorReading` in `sensor_data.h`
  and update `buildDocument()` in `firebase_manager.cpp`.
- **Custom BT commands** — Edit `BTManager::onCommandReceived()` in
  `bt_manager.cpp`.
- **BLE instead of Classic BT** — Swap `bt_manager.*` for a BLE GATT
  service; the rest of the project is unchanged.
- **MQTT** — Replace `firebase_manager.*` with an MQTT client; the
  offline-queue pattern stays the same.
