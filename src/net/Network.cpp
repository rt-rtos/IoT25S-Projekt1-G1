#include "net/Network.h"

Network::Network(const char* ssid, const char* pass, uint32_t retryMs)
    : ssid_(ssid), pass_(pass), retryMs_(retryMs) {}

bool Network::begin() {
    if (WiFi.status() == WL_NO_MODULE) return false;
    status_ = WiFi.status();
    return true;
}

void Network::poll(uint32_t nowMs) {
    // TODO(network): refresh status_ from WiFi.status(); while not
    // connected, call WiFi.begin(ssid_, pass_) at most every retryMs_
    // (use lastAttemptMs_). See examples/ConnectWithWPA and outline 5.3.
    (void)nowMs;
}

void Network::printInfo(Print& out) const {
    // TODO(network): SSID, IP, RSSI, as in examples/ConnectWithWPA printCurrentNet().
    (void)out;
}
