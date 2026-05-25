// ============================================================
//  bt_task.cpp
//  FreeRTOS task — keeps the Bluetooth SPP loop running.
// ============================================================
#include "bt_manager.h"
#include "config.h"
#include <Arduino.h>

namespace {

void btTaskFn(void* /*pvParams*/) {
    BTManager::begin();

    for (;;) {
        // Handle incoming commands from connected BT device
        BTManager::processIncoming();

        // Small delay — avoids starving other tasks
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

} // anonymous namespace

// Called from main.cpp
void startBluetoothTask() {
    xTaskCreatePinnedToCore(
        btTaskFn,
        "BluetoothTask",
        TASK_STACK_BLUETOOTH,
        nullptr,
        TASK_PRIO_BLUETOOTH,
        nullptr,
        0        // Core 0 alongside WiFi/BT stack
    );
}
