#pragma once
// MQTT client and JSON payload (outline 5.4). Derived from the
// ArduinoMqttClient examples WiFiSimpleSender and WiFiAdvancedCallback
// (examples/WiFiSimpleSender, examples/WiFiAdvancedCallback): Last Will on
// the status topic, retained online/offline.
//
// Payload format: see lib/Payload/Payload.h.
//
// Note: connect() blocks for up to connectTimeoutMs while the TCP
// connection is attempted. Accepted for the prototype (see README).
#include <ArduinoMqttClient.h>
#include <Client.h>
#include "Payload.h"

struct TelemetryConfig {
    const char* host;
    uint16_t    port;
    const char* user;        // "" for anonymous
    const char* pass;
    const char* clientId;
    const char* topicTelemetry;
    const char* topicStatus;
    const char* topicCmd;    // nullptr to disable the cmd subscription
    uint32_t    retryMs;
    uint32_t    keepAliveMs;
    uint32_t    connectTimeoutMs;
};

class Telemetry {
public:
    typedef void (*CommandHandler)(const char* payload, size_t len);

    Telemetry(Client& transport, const TelemetryConfig& cfg);

    void begin();
    // Call every loop pass while Wi-Fi is up. Reconnects at most every retryMs.
    void poll(uint32_t nowMs);
    bool connected();
    bool publish(const Snapshot& s, const SourceInfo& src);
    // Plumbing for the optional cmd topic (outline 5.4); unused for now,
    // scenario switching is done over serial (SerialCommand).
    void onCommand(CommandHandler h) { handler_ = h; }
    int  lastError() const { return lastError_; }

private:
    bool connect();
    void handleMessage(int size);
    static void dispatch(int size);

    MqttClient      mqtt_;
    TelemetryConfig cfg_;
    CommandHandler  handler_ = nullptr;
    uint32_t        lastAttemptMs_ = 0;
    int             lastError_ = 0;
    char            buf_[256];
    static Telemetry* instance_;
};
