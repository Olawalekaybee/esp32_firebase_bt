#pragma once
// ============================================================
//  sensor_task.h  —  FreeRTOS task: sample sensor + distribute data
// ============================================================
#include "sensor_data.h"

namespace SensorTask {
    void start();                                // Spawns the task
    SensorReading getLatestReading();            // Thread-safe snapshot
}
