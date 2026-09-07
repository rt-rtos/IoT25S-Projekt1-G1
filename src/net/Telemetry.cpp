#include "net/Telemetry.h"

Telemetry* Telemetry::instance_ = nullptr;

Telemetry::Telemetry(Client& transport, const TelemetryConfig& cfg)
    : mqtt_(transport), cfg_(cfg) {
    instance_ = this;
}

void Telemetry::begin() {
    // TODO(telemetry): setId, setUsernamePassword when cfg_.user is non-empty,
    // setKeepAliveInterval, setConnectionTimeout, onMessage(dispatch).
    // See examples/WiFiAdvancedCallback setup().
}

bool Telemetry::connect() {
    // TODO(telemetry): Last Will on cfg_.topicStatus ("offline", retained,
    // QoS 1) via beginWill/print/endWill BEFORE connect(); then connect,
    // publish retained "online" on the status topic, subscribe to
    // cfg_.topicCmd if set. Store connectError() in lastError_ on failure.
    // See examples/WiFiAdvancedCallback and outline 5.4.
    return false;
}

void Telemetry::poll(uint32_t nowMs) {
    // TODO(telemetry): if connected, mqtt_.poll() (keep-alives, delivers
    // incoming messages to dispatch); otherwise call connect() at most every
    // cfg_.retryMs using lastAttemptMs_. See examples/WiFiSimpleSender loop().
    (void)nowMs;
}

bool Telemetry::connected() {
    return mqtt_.connected();
}

bool Telemetry::publish(const Snapshot& s, const SourceInfo& src) {
    // TODO(telemetry): buildTelemetryJson into buf_, refuse on truncation,
    // then beginMessage(topic, length, retain=false, qos=0) / write / endMessage.
    // See examples/WiFiSimpleSender loop().
    (void)s; (void)src;
    return false;
}

void Telemetry::dispatch(int size) {
    if (instance_) instance_->handleMessage(size);
}

void Telemetry::handleMessage(int size) {
    // TODO(telemetry): while mqtt_.available() and room in buf_, buf_[n++] =
    // mqtt_.read(); terminate; call handler_(buf_, n) if set. Drain the rest
    // if size exceeds buf_. See examples/WiFiSimpleReceiveCallback
    // onMqttMessage(). mqtt_.messageTopic() gives the topic if ever needed.
    (void)size;
}
