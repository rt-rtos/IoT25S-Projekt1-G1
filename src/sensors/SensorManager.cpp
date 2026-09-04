#include "sensors/SensorManager.h"

SensorManager::SensorManager(Sht4xSensor& sht, Ds18b20Sensor& ds18b20, NtcSensor& ntc, uint32_t timeoutMs)
    : sht_(sht), ds_(ds18b20), ntc_(ntc), timeoutMs_(timeoutMs) {}

bool SensorManager::begin() {
    bool a = sht_.begin();
    bool b = ds_.begin();
    bool c = ntc_.begin();
    return a && b && c;
}

void SensorManager::startCycle(uint32_t nowMs) {
    sht_.start(nowMs);
    ds_.start(nowMs);
    ntc_.start(nowMs);
    cycleStartMs_ = nowMs;
    cycleActive_  = true;
}

bool SensorManager::update(uint32_t nowMs, Snapshot& out) {
    if (!cycleActive_) return false;
    bool allReady = sht_.ready(nowMs) && ds_.ready(nowMs) && ntc_.ready(nowMs);
    bool timedOut = (nowMs - cycleStartMs_) >= timeoutMs_;
    if (!allReady && !timedOut) return false;

    if (sht_.ready(nowMs)) sht_.read(out.tIn, out.rhIn, nowMs);
    else { out.tIn.fail(FAULT_TIMEOUT, nowMs); out.rhIn.fail(FAULT_TIMEOUT, nowMs); }

    if (ds_.ready(nowMs)) ds_.read(out.tOut, nowMs);
    else out.tOut.fail(FAULT_TIMEOUT, nowMs);

    if (ntc_.ready(nowMs)) ntc_.read(out.tWater, nowMs);
    else out.tWater.fail(FAULT_TIMEOUT, nowMs);

    out.seq      = ++seq_;
    out.uptimeMs = nowMs;
    cycleActive_ = false;
    return true;
}
