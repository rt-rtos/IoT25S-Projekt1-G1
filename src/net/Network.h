#pragma once
// Wi-Fi state handling on WiFiS3 (outline 5.1, 5.3). Derived from the
// WiFiS3 ConnectWithWPA example (examples/ConnectWithWPA).
//
// poll() is called every loop pass. It re-issues WiFi.begin() at most every
// retryMs while disconnected. Note: WiFi.begin() on WiFiS3 itself blocks
// for a few seconds while the module associates; that is accepted for
// the prototype and documented as a limitation.
#include <WiFiS3.h>

class Network {
public:
    Network(const char* ssid, const char* pass, uint32_t retryMs);

    // false if the Wi-Fi module does not answer at all.
    bool begin();
    void poll(uint32_t nowMs);
    bool connected() const { return status_ == WL_CONNECTED; }
    int  status() const { return status_; }
    long rssi() const { return WiFi.RSSI(); }
    void printInfo(Print& out) const;

private:
    const char* ssid_;
    const char* pass_;
    uint32_t    retryMs_;
    uint32_t    lastAttemptMs_ = 0;
    int         status_ = WL_IDLE_STATUS;
};
