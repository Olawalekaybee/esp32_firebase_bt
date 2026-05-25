// ============================================================
//  sensor_task.cpp
//  FreeRTOS task — samples sensor, shares latest reading, and
//  enqueues data for Firebase and Bluetooth.
// ============================================================
#include "sensor_task.h"
#include "firebase_manager.h"
#include "bt_manager.h"
#include "config.h"

#include <Arduino.h>
#include <WiFi.h>          // for WiFi.RSSI()

// ── Replace this block with your real sensor library ─────────
// e.g.  #include <Adafruit_BME280.h>
//       static Adafruit_BME280 bme;
// ─────────────────────────────────────────────────────────────

namespace {

SemaphoreHandle_t _readingMutex = nullptr;
SensorReading     _latest       = {};

// ── ISO-8601 timestamp via SNTP ───────────────────────────────
String getTimestamp() {
    struct tm ti;
    if (!getLocalTime(&ti, 100)) return "1970-01-01T00:00:00Z";
    char buf[30];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &ti);
    return String(buf);
}

// ── Sensor read — replace stub with real hardware calls ───────
SensorReading readSensor() {
    SensorReading r;

    // ─── BEGIN real sensor code ───────────────────────────────
    // r.temperature = bme.readTemperature();
    // r.humidity    = bme.readHumidity();
    // r.pressure    = bme.readPressure() / 100.0F;
    // r.valid       = !isnan(r.temperature);
    // ─── END real sensor code ─────────────────────────────────

    // Placeholder: sine-wave demo values so the project compiles & runs
    static uint32_t tick = 0;
    r.temperature = 25.0f + 5.0f  * sinf(tick * 0.1f);
    r.humidity    = 50.0f + 10.0f * cosf(tick * 0.1f);
    r.pressure    = 1013.25f;
    r.valid       = true;
    tick++;

    r.rssi      = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;
    r.timestamp = getTimestamp();
    return r;
}

// ── RTOS task body ────────────────────────────────────────────
void sensorTaskFn(void* /*pvParams*/) {
    // Best-effort NTP sync (needs WiFi — retries automatically)
    configTime(0, 0, "pool.ntp.org", "time.google.com");

    TickType_t    lastWake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(SENSOR_SAMPLE_INTERVAL_MS);

    for (;;) {
        SensorReading r = readSensor();

        // Store latest reading thread-safely
        if (xSemaphoreTake(_readingMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            _latest = r;
            xSemaphoreGive(_readingMutex);
        }

        // Firebase — enqueue (works offline; drained when connected)
        FirebaseManager::pushReading(r);

        // Bluetooth — send immediately if a client is paired
        BTManager::sendReading(r);

        Serial.printf("[Sensor] T=%.1f°C  H=%.1f%%  P=%.1fhPa  RSSI=%d dBm\n",
                      r.temperature, r.humidity, r.pressure, r.rssi);

        vTaskDelayUntil(&lastWake, period);
    }
}

} // anonymous namespace

// ── Public API ───────────────────────────────────────────────
namespace SensorTask {

void start() {
    _readingMutex = xSemaphoreCreateMutex();
    if (_readingMutex == nullptr) {
        Serial.println("[Sensor] FATAL: could not create mutex.");
        return;
    }

    xTaskCreatePinnedToCore(
        sensorTaskFn,
        "SensorTask",
        TASK_STACK_SENSOR,
        nullptr,
        TASK_PRIO_SENSOR,
        nullptr,
        1   // Core 1 — APP_CPU, away from WiFi/BT stack on Core 0
    );
    Serial.println("[Sensor] Task started on Core 1.");
}

SensorReading getLatestReading() {
    SensorReading snapshot = {};
    if (_readingMutex &&
        xSemaphoreTake(_readingMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        snapshot = _latest;
        xSemaphoreGive(_readingMutex);
    }
    return snapshot;
}

} // namespace SensorTask
