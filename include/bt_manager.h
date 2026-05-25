#pragma once
// ============================================================
//  bt_manager.h  —  Bluetooth Classic SPP wrapper
// ============================================================
#include <Arduino.h>
#include "sensor_data.h"

namespace BTManager {
    void begin();                              // Call once in setup()
    bool isClientConnected();
    void sendReading(const SensorReading& r);  // Sends full JSON over SPP
    void sendAlert(const SensorReading& r);    // Sends alert-only JSON over SPP
    void processIncoming();                    // Call in BT task loop
    void onCommandReceived(const String& cmd); // Extend for custom commands
}
