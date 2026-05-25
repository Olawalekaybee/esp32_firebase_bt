#pragma once
// ============================================================
//  config.h  —  All user-configurable settings live HERE
//  Edit this file only; the rest of the code is hands-off.
// ============================================================

// ── WiFi ────────────────────────────────────────────────────
#define WIFI_SSID          "YOUR_WIFI_SSID"
#define WIFI_PASSWORD      "YOUR_WIFI_PASSWORD"
#define WIFI_CONNECT_TIMEOUT_MS  15000   // ms before giving up

// ── Firebase ─────────────────────────────────────────────────
#define FIREBASE_API_KEY        "YOUR_FIREBASE_WEB_API_KEY"
#define FIREBASE_PROJECT_ID     "YOUR_FIREBASE_PROJECT_ID"
#define FIREBASE_USER_EMAIL     "YOUR_FIREBASE_USER_EMAIL"
#define FIREBASE_USER_PASSWORD  "YOUR_FIREBASE_USER_PASSWORD"

// Firestore collection/document where sensor data is written
#define FIREBASE_COLLECTION     "sensor_data"

// How often to push data to Firebase (ms)
#define FIREBASE_PUSH_INTERVAL_MS  5000

// ── Bluetooth Classic ────────────────────────────────────────
#define BT_DEVICE_NAME    "ESP32_DataHub"   // advertised name

// ── Sensor (example: replace with your real sensor logic) ────
// How often to sample the sensor (ms)
#define SENSOR_SAMPLE_INTERVAL_MS  1000

// ── Offline / local storage ──────────────────────────────────
// Max number of readings to buffer when there is no network
#define OFFLINE_QUEUE_DEPTH  50

// ── Watchdog ─────────────────────────────────────────────────
#define WDT_TIMEOUT_SECONDS  30

// ── TinyML / Anomaly Detection ───────────────────────────────
// Number of readings used to learn the baseline (warm-up period)
#define AI_BASELINE_SAMPLES      30

// How many recent readings the sliding window holds for inference
#define AI_WINDOW_SIZE           10

// Anomaly score threshold — readings scoring above this fire an alert
// Range 0.0–1.0.  Lower = more sensitive.
#define AI_ANOMALY_THRESHOLD     0.75f

// Minimum ms between two alerts for the same sensor (debounce)
#define AI_ALERT_COOLDOWN_MS     10000

// How often the AI task runs inference (ms)
#define AI_INFERENCE_INTERVAL_MS 2000

// Stack size for the AI RTOS task (bytes)
#define TASK_STACK_AI            6144

// Priority for the AI task (lower than BT, higher than sensor)
#define TASK_PRIO_AI             2

// ── Alert thresholds (hard limits, checked alongside ML score) ──
#define ALERT_TEMP_HIGH_C        40.0f   // °C — critical high
#define ALERT_TEMP_LOW_C         0.0f    // °C — critical low
#define ALERT_HUMIDITY_HIGH_PCT  90.0f   // %  — critical high
#define ALERT_HUMIDITY_LOW_PCT   10.0f   // %  — critical low
#define ALERT_PRESSURE_HIGH_HPA  1040.0f // hPa — critical high
#define ALERT_PRESSURE_LOW_HPA   970.0f  // hPa — critical low
