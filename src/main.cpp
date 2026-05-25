// ============================================================
//  main.cpp  —  Entry point
//  ESP32 | Firebase + Bluetooth Classic + TinyML + FreeRTOS
//
//  Architecture overview
//  ─────────────────────
//  Core 0 (PRO_CPU):  WiFi stack, BT stack, FirebaseTask, BluetoothTask
//  Core 1 (APP_CPU):  SensorTask, AITask
//
//  Data flow
//  ─────────
//  SensorTask ──► FirebaseManager::queue ──► FirebaseTask ──► Firestore
//             └──► BTManager::sendReading ──► BT Client
//             └──► AITask (reads latest via getLatestReading())
//                    │
//                    ├── AnomalyDetector::infer()  [TinyML on-device]
//                    │
//                    └── AlertManager::dispatch()
//                           ├── BTManager::sendAlert()   ← always
//                           └── FirebaseManager::pushAlert() ← when online
//
//  Offline behaviour
//  ─────────────────
//  Sensor + AI tasks always run — no network dependency.
//  Alerts are sent over BT immediately; Firebase queues them for
//  upload the moment WiFi is restored.
// ============================================================
#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "sensor_task.h"
#include "ai_task.h"
#include "alert_manager.h"

// Forward declarations (defined in their respective task files)
void startFirebaseTask();
void startBluetoothTask();

// ────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println("\n================================================");
    Serial.println("  ESP32  Firebase + BT Classic + TinyML + RTOS");
    Serial.println("================================================\n");

    // WiFi — non-blocking; tasks handle reconnection internally
    WiFiManager::begin();

    // Spawn all RTOS tasks
    startBluetoothTask();   // BT task (Core 0, no WiFi needed)
    startFirebaseTask();    // Firebase task (Core 0, waits for WiFi internally)
    SensorTask::start();    // Sensor sampling (Core 1)
    AITask::start();        // TinyML inference + alerts (Core 1)

    Serial.println("[Main] All tasks started.  setup() complete.\n");
    Serial.printf("[Main] AI baseline needs %d samples (~%d seconds).\n\n",
                  AI_BASELINE_SAMPLES,
                  (AI_BASELINE_SAMPLES * SENSOR_SAMPLE_INTERVAL_MS) / 1000);
}

// ────────────────────────────────────────────────────────────
// loop() is intentionally minimal; all work is in RTOS tasks.
void loop() {
    static uint32_t lastPrint = 0;
    if (millis() - lastPrint > 30000) {
        lastPrint = millis();
        WiFiManager::printStatus();
        Serial.printf("[Main] Total alerts fired: %u\n",
                      AlertManager::totalAlertCount());
    }
    delay(1000);
}
