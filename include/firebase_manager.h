#pragma once
// ============================================================
//  firebase_manager.h  —  Firestore push + offline queue
// ============================================================
#include <Arduino.h>
#include "sensor_data.h"

namespace FirebaseManager {
    void begin();                             // Call once after WiFi is up
    bool isReady();
    void pushReading(const SensorReading& r); // Thread-safe enqueue
    void processQueue();                      // Called inside FirebaseTask
}
