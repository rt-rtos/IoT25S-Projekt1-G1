// MicroHydros climate node: setup(), loop(), millis scheduler and the
// connection state machine (outline 5.3). Sampling and validation run in
// every state; every snapshot is published when Online.
//
// Known blocking spots: WiFi.begin() on WiFiS3 stalls for a few seconds
// while associating, and the MQTT connect for up to
// MQTT_CONNECT_TIMEOUT_MS. Sampling pauses during those attempts; the
// DS18B20 timing is millis-based so a late read is still correct.
#include <Arduino.h>
#include <WiFiS3.h>

#include "config.h"
#include "secrets.h"

#include "sensors/sht4x/Sht4xSensor.h"
#include "Sht4xSimulated.h"
#include "sensors/sht4x/Sht4xHardware.h"
#include "sensors/Ds18b20Sensor.h"
#include "sensors/NtcSensor.h"
#include "sensors/SensorManager.h"
#include "Validity.h"
#include "net/Network.h"
#include "net/Telemetry.h"
#include "ui/StatusLed.h"
#include "ui/SerialCommand.h"

enum class NodeState : uint8_t { Boot, WifiConnecting, MqttConnecting, Online };

#if SHT4X_SIMULATED
static Sht4xSimulated shtSource;
static const char* SHT_SRC = "sim";
#else
static Sht4xHardware  shtSource(Wire1);
static const char* SHT_SRC = "hw";
#endif

static Sht4xSensor   shtSensor(shtSource);
static Ds18b20Sensor dsSensor(PIN_ONEWIRE, DS18B20_RESOLUTION_BITS);
static NtcSensor     ntcSensor(PIN_NTC, ADC_BITS, NTC_SAMPLES,
                               ntc::Params{NTC_R_SERIES, NTC_R0, NTC_BETA, 25.0f, NTC_OFFSET_C});
static SensorManager sensors(shtSensor, dsSensor, ntcSensor, ACQUIRE_TIMEOUT_MS);

static Validity validity;
static Network  network(SECRET_SSID, SECRET_PASS, WIFI_RETRY_MS);

static WiFiClient wifiClient;
static const TelemetryConfig telemetryCfg = {
    MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASS, DEVICE_ID,
    MQTT_TOPIC_TELEMETRY, MQTT_TOPIC_STATUS, nullptr,
    MQTT_RETRY_MS, MQTT_KEEPALIVE_MS, MQTT_CONNECT_TIMEOUT_MS,
};
static Telemetry     telemetry(wifiClient, telemetryCfg);
static StatusLed     led;
static SerialCommand serialCmd(Serial);

static NodeState state = NodeState::Boot;
static Snapshot  current;
static Snapshot  previous;
static uint32_t  sampleIntervalMs = SAMPLE_INTERVAL_MS;
static uint32_t  lastSampleMs     = 0;

static void onSerialCommand(const char* cmd, const char* arg) {
    // TODO(firmware): "scenario <name>" -> shtSource.setScenarioByName (only
    // when SHT4X_SIMULATED), "sample <s>" -> sampleIntervalMs =
    // strtoul(arg, nullptr, 10) * 1000, refused (message, no change) when 0
    // or below SAMPLE_INTERVAL_MIN_MS; print the value that took effect.
    // "help" lists the three. Unknown command -> "unknown: <cmd>".
    Serial.print("cmd: "); Serial.print(cmd); Serial.print(" "); Serial.println(arg);
}

static void logSnapshot(const Snapshot& s) {
    Serial.print("#"); Serial.print(s.seq);
    Serial.print(" t_in=");    Serial.print(s.tIn.value, 2);    Serial.print("/"); Serial.print(faultName(s.tIn.fault));
    Serial.print(" rh_in=");   Serial.print(s.rhIn.value, 1);   Serial.print("/"); Serial.print(faultName(s.rhIn.fault));
    Serial.print(" t_out=");   Serial.print(s.tOut.value, 2);   Serial.print("/"); Serial.print(faultName(s.tOut.fault));
    Serial.print(" t_water="); Serial.print(s.tWater.value, 2); Serial.print("/"); Serial.println(faultName(s.tWater.fault));
}

static void runStateMachine(uint32_t nowMs) {
    // TODO(firmware): transitions per outline 5.3:
    //   Boot -> WifiConnecting
    //   WifiConnecting: network.poll(); connected -> MqttConnecting
    //   MqttConnecting: network.poll(); Wi-Fi lost -> WifiConnecting;
    //                   telemetry.poll(); connected -> Online
    //   Online: network.poll(); Wi-Fi lost -> WifiConnecting;
    //           telemetry.poll(); broker lost -> MqttConnecting
    (void)nowMs;
}

void setup() {
    Serial.begin(SERIAL_BAUD);
    uint32_t t0 = millis();
    while (!Serial && millis() - t0 < 3000) {}
    // TODO(firmware): print free RAM here (risk row "Uno R4 SRAM", outline
    // 10), e.g. the gap between a stack local's address and the heap top
    // from malloc(1), so the number is in every boot log for tests.md.

    led.begin();
    Serial.println("MicroHydros climate node " DEVICE_ID " (type 'help')");

    if (!sensors.begin()) Serial.println("warning: not all sensors answered at begin()");
    if (!network.begin()) Serial.println("error: Wi-Fi module not responding");
    telemetry.begin();
    serialCmd.onCommand(onSerialCommand);
}

void loop() {
    uint32_t now = millis();

    serialCmd.poll();
    runStateMachine(now);

    if (!sensors.busy() && (now - lastSampleMs) >= sampleIntervalMs) {
        lastSampleMs = now;
        sensors.startCycle(now);
    }
    if (sensors.update(now, current)) {
        validity.check(current, previous);
        logSnapshot(current);
        previous = current;
        // TODO(firmware): when Online, telemetry.publish(current, {SHT_SRC, "hw", "hw"})
        // and led.blinkPublish(now) on success.
        (void)SHT_SRC;
    }

    // TODO(firmware): map state to StatusLed::State, led.show(state, any channel invalid), led.update(now).
    (void)state; (void)telemetry; (void)led;
}
