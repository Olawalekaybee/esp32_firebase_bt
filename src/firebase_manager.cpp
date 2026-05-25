// ============================================================
//  firebase_manager.cpp  —  Firestore REST push + offline queue
//  Uses: mobizt/FirebaseClient @ ^1.5.x
// ============================================================
#include "firebase_manager.h"
#include "wifi_manager.h"
#include "config.h"

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>
#include <queue>

// ── Internal linkage ─────────────────────────────────────────
namespace {

// SSL + network
WiFiClientSecure  _sslClient;
DefaultNetwork    _network;

// Auth — Email/Password
UserAuth          _auth(FIREBASE_API_KEY,
                        FIREBASE_USER_EMAIL,
                        FIREBASE_USER_PASSWORD);

// Core Firebase objects
FirebaseApp           _app;
AsyncClientClass      _asyncClient(_sslClient, getNetwork(_network));
Firestore::Documents  _docs;

// Offline queue — mutex-protected for RTOS safety
SemaphoreHandle_t         _queueMutex = nullptr;
std::queue<SensorReading> _offlineQueue;

bool _initialised = false;

// ─── Async result callback ────────────────────────────────────
void asyncCB(AsyncResult& res) {
    if (res.isError()) {
        Serial.printf("[Firebase] Error %d: %s\n",
                      res.error().code(),
                      res.error().message().c_str());
    } else if (res.available()) {
        Serial.printf("[Firebase] Push OK (%u bytes)\n",
                      res.payload().length());
    }
}

// ─── Build a Firestore document from a SensorReading ──────────
// FirebaseClient 1.5.x API: Document::add(const String& key, T value)
Document<Values::MapValue> buildDocument(const SensorReading& r) {
    using namespace Values;
    Document<MapValue> doc("fields");

    doc.add("temperature", Value(DoubleValue((double)r.temperature)));
    doc.add("humidity",    Value(DoubleValue((double)r.humidity)));
    doc.add("pressure",    Value(DoubleValue((double)r.pressure)));
    doc.add("rssi",        Value(IntegerValue((int64_t)r.rssi)));
    doc.add("timestamp",   Value(StringValue(r.timestamp)));
    doc.add("valid",       Value(BooleanValue(r.valid)));
    return doc;
}

} // anonymous namespace

// ── Public API ───────────────────────────────────────────────
namespace FirebaseManager {

void begin() {
    _queueMutex = xSemaphoreCreateMutex();
    if (_queueMutex == nullptr) {
        Serial.println("[Firebase] FATAL: could not create queue mutex.");
        return;
    }

    // Accept any server certificate.
    // For production: load a root CA via _sslClient.setCACert(root_ca);
    _sslClient.setInsecure();

    // Initialise the Firebase app with the auth object
    initializeApp(_asyncClient, _app, getAuth(_auth), asyncCB, "fbApp");

    // Bind the Firestore Documents service to the app
    _app.getApp<Firestore::Documents>(_docs);

    _initialised = true;
    Serial.println("[Firebase] Initialised — waiting for auth token...");
}

bool isReady() {
    return _initialised && _app.ready();
}

// Thread-safe enqueue — works offline; FirebaseTask drains the queue.
void pushReading(const SensorReading& r) {
    if (!_initialised || _queueMutex == nullptr) return;

    if (xSemaphoreTake(_queueMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if ((int)_offlineQueue.size() < OFFLINE_QUEUE_DEPTH) {
            _offlineQueue.push(r);
            Serial.printf("[Firebase] Queued (depth=%u)\n", _offlineQueue.size());
        } else {
            // Drop oldest to make room
            _offlineQueue.pop();
            _offlineQueue.push(r);
            Serial.println("[Firebase] WARN: queue full — dropped oldest reading.");
        }
        xSemaphoreGive(_queueMutex);
    }
}

// Called continuously from FirebaseTask — runs the library state-machine
// and drains the offline queue whenever the network is available.
void processQueue() {
    // Must be called regularly — drives token refresh, retries, etc.
    _app.loop();

    if (!WiFiManager::isConnected() || !isReady()) return;
    if (_queueMutex == nullptr) return;

    if (xSemaphoreTake(_queueMutex, pdMS_TO_TICKS(50)) != pdTRUE) return;

    while (!_offlineQueue.empty()) {
        SensorReading r = _offlineQueue.front();
        _offlineQueue.pop();
        xSemaphoreGive(_queueMutex); // release lock while doing network I/O

        auto doc = buildDocument(r);

        // FirebaseClient 1.5.x: use Firestore::Parent(projectId, databaseId)
        // then pass the collection path as a separate argument
        Firestore::Parent parent(FIREBASE_PROJECT_ID, "(default)");

        _docs.createDocument(_asyncClient, parent,
                             FIREBASE_COLLECTION,   // collection ID
                             "",                    // auto-generate document ID
                             DocumentMask(), doc, asyncCB);

        Serial.printf("[Firebase] Sending doc — queue remaining: %u\n",
                      _offlineQueue.size());

        // Yield between documents so other tasks stay healthy
        vTaskDelay(pdMS_TO_TICKS(300));

        if (xSemaphoreTake(_queueMutex, pdMS_TO_TICKS(50)) != pdTRUE) return;
    }

    xSemaphoreGive(_queueMutex);
}

} // namespace FirebaseManager
