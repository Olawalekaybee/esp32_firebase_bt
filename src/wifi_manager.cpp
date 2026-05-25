// ============================================================
//  wifi_manager.cpp
// ============================================================
#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>

namespace WiFiManager {

static bool _initialised = false;

void begin() {
    if (_initialised) return;
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.printf("[WiFi] Connecting to '%s'", WIFI_SSID);

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
        delay(500);
        Serial.print('.');
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("[WiFi] Connected — IP: %s  RSSI: %d dBm\n",
                      WiFi.localIP().toString().c_str(), WiFi.RSSI());
    } else {
        Serial.println("[WiFi] Could not connect — running offline.");
    }
    _initialised = true;
}

bool isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

// Attempts a reconnect if needed; returns current state.
bool ensureConnected() {
    if (isConnected()) return true;
    Serial.println("[WiFi] Lost connection — reconnecting…");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    uint32_t start = millis();
    while (!isConnected() && millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
        vTaskDelay(pdMS_TO_TICKS(500));
        Serial.print('.');
    }
    Serial.println();
    if (isConnected()) {
        Serial.printf("[WiFi] Reconnected — IP: %s\n",
                      WiFi.localIP().toString().c_str());
    }
    return isConnected();
}

void printStatus() {
    Serial.printf("[WiFi] Status: %s  IP: %s  RSSI: %d dBm\n",
                  isConnected() ? "CONNECTED" : "DISCONNECTED",
                  WiFi.localIP().toString().c_str(),
                  WiFi.RSSI());
}

String localIP() {
    return WiFi.localIP().toString();
}

} // namespace WiFiManager
