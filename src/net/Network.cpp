#include "net/Network.h"

Network::Network(const char* ssid, const char* pass, uint32_t retryMs)
    : ssid_(ssid), pass_(pass), retryMs_(retryMs) {}

bool Network::begin() {
    if (WiFi.status() == WL_NO_MODULE) return false;
    status_ = WiFi.status();
    return true;
}

void Network::poll(uint32_t nowMs) {
   status_ = WiFi.status();
    if (status_ != WL_CONNECTED) {
        if (nowMs - lastAttemptMs_ >= retryMs_) {
            WiFi.begin(ssid_, pass_);
            lastAttemptMs_ = nowMs;
        }
    }
}


void Network::printInfo(Print& out) const {
     out.print("SSID: ");
    out.println(WiFi.SSID());

    out.print("RSSI: ");
    out.println(WiFi.RSSI());

    out.print("IP: ");
    out.println(WiFi.localIP());
}
