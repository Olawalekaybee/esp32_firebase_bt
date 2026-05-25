#pragma once
// ============================================================
//  sensor_data.h  —  Plain-old-data structure shared between tasks
// ============================================================
#include <Arduino.h>
#include <ArduinoJson.h>

// ── Severity levels for AI alerts ────────────────────────────
enum class AlertSeverity : uint8_t {
    NONE     = 0,
    INFO     = 1,   // slight deviation detected
    WARNING  = 2,   // significant anomaly
    CRITICAL = 3    // hard threshold breached
};

inline const char* severityLabel(AlertSeverity s) {
    switch (s) {
        case AlertSeverity::INFO:     return "INFO";
        case AlertSeverity::WARNING:  return "WARNING";
        case AlertSeverity::CRITICAL: return "CRITICAL";
        default:                      return "NONE";
    }
}

// ── Anomaly result attached to each reading ───────────────────
struct AnomalyResult {
    float         score;        // 0.0 = normal, 1.0 = max anomaly
    AlertSeverity severity;
    String        reason;       // human-readable description
    bool          triggered;    // true if an alert was raised
};

// ── Core sensor reading ───────────────────────────────────────
struct SensorReading {
    float   temperature;   // °C
    float   humidity;      // %
    float   pressure;      // hPa
    int32_t rssi;          // WiFi RSSI at time of reading
    String  timestamp;     // ISO-8601 string, filled by SensorTask
    bool    valid;         // false = placeholder / read error

    // Filled by AnomalyDetector after inference
    AnomalyResult anomaly = { 0.0f, AlertSeverity::NONE, "", false };
};

// ── JSON serialiser (includes anomaly fields) ─────────────────
inline String sensorReadingToJson(const SensorReading& r) {
    JsonDocument doc;
    doc["temperature"] = r.temperature;
    doc["humidity"]    = r.humidity;
    doc["pressure"]    = r.pressure;
    doc["rssi"]        = r.rssi;
    doc["timestamp"]   = r.timestamp;
    doc["valid"]       = r.valid;

    JsonObject ai = doc["ai"].to<JsonObject>();
    ai["score"]     = r.anomaly.score;
    ai["severity"]  = severityLabel(r.anomaly.severity);
    ai["reason"]    = r.anomaly.reason;
    ai["triggered"] = r.anomaly.triggered;

    String out;
    serializeJson(doc, out);
    return out;
}

// ── Alert-only JSON (sent over BT when anomaly fires) ─────────
inline String alertToJson(const SensorReading& r) {
    JsonDocument doc;
    doc["type"]        = "ALERT";
    doc["severity"]    = severityLabel(r.anomaly.severity);
    doc["score"]       = r.anomaly.score;
    doc["reason"]      = r.anomaly.reason;
    doc["temperature"] = r.temperature;
    doc["humidity"]    = r.humidity;
    doc["pressure"]    = r.pressure;
    doc["timestamp"]   = r.timestamp;
    String out;
    serializeJson(doc, out);
    return out;
}
