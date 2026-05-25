// ============================================================
//  firebase_task.cpp
//  FreeRTOS task — initialises Firebase and drains the upload queue.
//  Runs independently; the sensor task keeps filling the queue
//  whether or not the network is available.
// ============================================================
#include "firebase_manager.h"
#include "wifi_manager.h"
#include "config.h"
#include <Arduino.h>
#include <esp_task_wdt.h>

namespace {

void firebaseTaskFn(void* /*pvParams*/) {
    // Wait for WiFi before touching Firebase
    Serial.println("[FirebaseTask] Waiting for WiFi…");
    while (!WiFiManager::isConnected()) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    FirebaseManager::begin();

    TickType_t    lastPush = xTaskGetTickCount();
    const TickType_t pushPeriod = pdMS_TO_TICKS(FIREBASE_PUSH_INTERVAL_MS);

    for (;;) {
        // Always call processQueue — it keeps the Firebase state-machine ticking
        // and drains accumulated readings when online.
        FirebaseManager::processQueue();

        // Brief yield to keep other tasks healthy
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

} // anonymous namespace

// Called from main.cpp
void startFirebaseTask() {
    xTaskCreatePinnedToCore(
        firebaseTaskFn,
        "FirebaseTask",
        TASK_STACK_FIREBASE,
        nullptr,
        TASK_PRIO_FIREBASE,
        nullptr,
        0           // Core 0 — alongside the WiFi/BT stack
    );
}
