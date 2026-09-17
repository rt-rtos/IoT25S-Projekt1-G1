#include "net/Telemetry.h"
#include <cstring>

Telemetry* Telemetry::instance_ = nullptr;

Telemetry::Telemetry(Client& transport, const TelemetryConfig& cfg)
    : mqtt_(transport), cfg_(cfg) {
    instance_ = this;
}

    void Telemetry::begin() {
    mqtt_.setId(cfg_.clientId);

    if (cfg_.user[0] != '\0') {
        mqtt_.setUsernamePassword(cfg_.user, cfg_.pass);
    }

    mqtt_.setKeepAliveInterval(cfg_.keepAliveMs);
    mqtt_.setConnectionTimeout(cfg_.connectTimeoutMs);
    mqtt_.onMessage(dispatch);
}
 


    bool Telemetry::connect()
    {
    mqtt_.beginWill(cfg_.topicStatus, strlen("offline"), true, 1);
    mqtt_.print("offline");
    mqtt_.endWill();
    if (!mqtt_.connect(cfg_.host, cfg_.port)) {
        lastError_ = mqtt_.connectError();
        return false;
    }
    mqtt_.beginMessage(cfg_.topicStatus, strlen("online"), true, 1);
    mqtt_.print("online");
    mqtt_.endMessage();
    if (cfg_.topicCmd != nullptr) {
        mqtt_.subscribe(cfg_.topicCmd);
    }

    return true;
}
        

void Telemetry::poll(uint32_t nowMs) {
     if (mqtt_.connected()) {
        mqtt_.poll();
    } else {
        if (nowMs - lastAttemptMs_ >= cfg_.retryMs) {
            connect();
            lastAttemptMs_ = nowMs;
        }
    }
}

bool Telemetry::connected() {
    return mqtt_.connected();
}

bool Telemetry::publish(const Snapshot& s, const SourceInfo& src) {
    int n = buildTelemetryJson(buf_, sizeof(buf_), s, src);

       if (n < 0 || (size_t)n >= sizeof(buf_)) {
        return false;
    }
    mqtt_.beginMessage(cfg_.topicTelemetry, n, false, 0);
    mqtt_.write((const uint8_t*)buf_, (size_t)n);
    mqtt_.endMessage();
    return true;
}



void Telemetry::dispatch(int size) {
    if (instance_) instance_->handleMessage(size);
}

void Telemetry::handleMessage(int size) {
      size_t n = (size_t)size < sizeof(buf_) - 1 ? (size_t)size : sizeof(buf_) - 1;

    size_t i = 0;
    while (mqtt_.available() && i < n) {
        buf_[i++] = (char)mqtt_.read();   
    }
    buf_[i] = '\0';  
    if (handler_) handler_(buf_, i);
}

