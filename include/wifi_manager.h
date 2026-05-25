#pragma once
// ============================================================
//  wifi_manager.h  —  Non-blocking WiFi helper
// ============================================================
#include <Arduino.h>

namespace WiFiManager {
    void     begin();                     // Call once in setup()
    bool     isConnected();
    bool     ensureConnected();           // Reconnects if needed; returns state
    void     printStatus();
    String   localIP();
}
