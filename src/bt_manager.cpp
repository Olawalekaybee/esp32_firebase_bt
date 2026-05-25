// ============================================================
//  bt_manager.cpp  —  Bluetooth Classic SPP (Serial Port Profile)
// ============================================================
#include "bt_manager.h"
#include "config.h"
#include <BluetoothSerial.h>

// ── ESP-IDF BT guard ─────────────────────────────────────────
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
  #error "Bluetooth is not enabled. Check sdkconfig / board settings."
#endif
#if !defined(CONFIG_BT_SPP_ENABLED)
  #error "BT SPP is not enabled — set CONFIG_BT_SPP_ENABLED=y in sdkconfig."
#endif

namespace {
    BluetoothSerial _bt;
    bool            _clientConnected = false;

    void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t* /*param*/) {
        if (event == ESP_SPP_SRV_OPEN_EVT) {
            _clientConnected = true;
            Serial.println("[BT] Client connected.");
        } else if (event == ESP_SPP_CLOSE_EVT) {
            _clientConnected = false;
            Serial.println("[BT] Client disconnected.");
        }
    }
} // anonymous namespace

namespace BTManager {

void begin() {
    _bt.register_callback(btCallback);
    if (!_bt.begin(BT_DEVICE_NAME)) {
        Serial.println("[BT] ERROR: Failed to initialise BluetoothSerial.");
        return;
    }
    Serial.printf("[BT] Advertising as '%s'\n", BT_DEVICE_NAME);
}

bool isClientConnected() {
    return _clientConnected;
}

void sendReading(const SensorReading& r) {
    if (!_clientConnected) return;
    String json = sensorReadingToJson(r);
    _bt.println(json);
    Serial.printf("[BT] Sent → %s\n", json.c_str());
}

// Called from the Bluetooth RTOS task every tick.
void processIncoming() {
    if (!_clientConnected) return;

    static String line;
    while (_bt.available()) {
        char c = static_cast<char>(_bt.read());
        if (c == '\n') {
            line.trim();
            if (line.length() > 0) onCommandReceived(line);
            line = "";
        } else {
            line += c;
        }
    }
}

// ── Extend this to handle commands from the paired client app ─
void onCommandReceived(const String& cmd) {
    Serial.printf("[BT] Command: '%s'\n", cmd.c_str());

    if (cmd.equalsIgnoreCase("PING")) {
        _bt.println("{\"response\":\"PONG\"}");
    } else if (cmd.equalsIgnoreCase("STATUS")) {
        _bt.println("{\"response\":\"OK\",\"bt\":true}");
    } else {
        _bt.printf("{\"response\":\"UNKNOWN\",\"cmd\":\"%s\"}\n", cmd.c_str());
    }
}

} // namespace BTManager
