// Component tests for the PC-compilable modules (outline 11, "Component").
// Run with:  pio test -e native
#include <unity.h>
#include <math.h>
#include "Sht4xDecoder.h"
#include "Sht4xSimulated.h"
#include "NtcMath.h"
#include "Validity.h"
#include "Payload.h"
#include <string.h>

void setUp() {}
void tearDown() {}

// SHT4x datasheet test vector
void test_crc8_datasheet_vector() {
    const uint8_t v[2] = {0xBE, 0xEF};
    TEST_ASSERT_EQUAL_HEX8(0x92, sht4x::crc8(v, 2));
}

void test_decode_roundtrip() {
    uint8_t frame[6];
    sht4x::encode(sht4x::temperatureToWord(24.5f), sht4x::humidityToWord(61.0f), frame);
    float t = 0, rh = 0;
    TEST_ASSERT_TRUE(sht4x::decode(frame, t, rh));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 24.5f, t);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 61.0f, rh);
}

void test_decode_rejects_bad_crc() {
    uint8_t frame[6];
    sht4x::encode(0x6666, 0x8000, frame);
    frame[5] ^= 0x01;
    float t = -1, rh = -1;
    TEST_ASSERT_FALSE(sht4x::decode(frame, t, rh));
    TEST_ASSERT_EQUAL_FLOAT(-1.0f, t);   // outputs untouched on false
    TEST_ASSERT_EQUAL_FLOAT(-1.0f, rh);
}

void test_decode_boundary_words() {
    uint8_t frame[6];
    float t, rh;
    sht4x::encode(0x0000, 0x0000, frame);
    TEST_ASSERT_TRUE(sht4x::decode(frame, t, rh));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -45.0f, t);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, rh);          // -6 clipped to 0
    sht4x::encode(0xFFFF, 0xFFFF, frame);
    TEST_ASSERT_TRUE(sht4x::decode(frame, t, rh));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 130.0f, t);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, rh);        // 119 clipped to 100
}

void test_emulator_steady_decodes_badcrc_fails() {
    Sht4xSimulated sim;
    uint8_t frame[6];
    float t, rh;
    sim.startMeasurement(0);
    TEST_ASSERT_TRUE(sim.frameReady(0));
    TEST_ASSERT_TRUE(sim.readFrame(frame));
    TEST_ASSERT_TRUE(sht4x::decode(frame, t, rh));
    TEST_ASSERT_TRUE(sim.setScenarioByName("badcrc", 1000));
    sim.startMeasurement(1000);
    TEST_ASSERT_TRUE(sim.readFrame(frame));
    TEST_ASSERT_FALSE(sht4x::decode(frame, t, rh));
    TEST_ASSERT_FALSE(sim.setScenarioByName("nope", 0));
}

void test_ntc_midpoint_is_25c() {
    ntc::Params p;
    // ADC at exactly half scale: R_ntc == R_s == R0 -> T0
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 25.0f, ntc::temperatureFromAdc(8192, 16383, p));
}

void test_ntc_hand_computed_points() {
    ntc::Params p;
    // R = R0 * exp(B * (1/T - 1/T0)) with B = 3950, R0 = 10 k, T0 = 25 C
    TEST_ASSERT_FLOAT_WITHIN(0.05f,  0.0f, ntc::temperatureFromResistance(33620.6f, p));
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 50.0f, ntc::temperatureFromResistance(3588.2f, p));
}

void test_ntc_short_open_are_faults() {
    TEST_ASSERT_TRUE(ntc::adcIsShort(0));
    TEST_ASSERT_TRUE(ntc::adcIsOpen(16383, 16383));
    TEST_ASSERT_FALSE(ntc::adcPlausible(3, 16383));
    TEST_ASSERT_TRUE(ntc::adcPlausible(8000, 16383));
}

void test_payload_null_for_invalid_and_fixed_point() {
    Snapshot s;
    s.seq = 123; s.uptimeMs = 4567890;
    s.tIn.set(24.06f, 0, 0);
    s.rhIn.set(61.0f, 0, 0);
    s.tOut.set(-3.5f, 0, 0);
    s.tWater.fail(FAULT_NO_DEVICE, 0);
    const SourceInfo src = { "sim", "hw", "hw" };
    char buf[256];
    int n = buildTelemetryJson(buf, sizeof buf, s, src);
    const char* expect =
        R"({"seq":123,"uptime_s":4567,)"
        R"("t_in":24.1,"rh_in":61.0,"t_out":-3.5,"t_water":null,)"
        R"("faults":{"t_in":0,"rh_in":0,"t_out":0,"t_water":1},)"
        R"("src":{"sht40":"sim","ds18b20":"hw","ntc":"hw"}})";
    TEST_ASSERT_EQUAL_STRING(expect, buf);
    TEST_ASSERT_EQUAL_INT((int)strlen(expect), n);
    // truncation is reported, buffer stays terminated
    char small[32];
    TEST_ASSERT_EQUAL_INT(n, buildTelemetryJson(small, sizeof small, s, src));
    TEST_ASSERT_EQUAL_UINT(31, strlen(small));
}

void test_validity_assert_range(Reading& r){
    TEST_ASSERT_FALSE(r.valid);
    TEST_ASSERT_EQUAL_INT(FAULT_RANGE,r.fault);
    TEST_ASSERT_FALSE(isnan(r.value));
}
void test_validity_out_of_range(){
    Snapshot s;
    Validity v;
    s.seq = 123; s.uptimeMs = 3236890;
    s.tIn.set(3.0f, 0, 0);
    s.rhIn.set(110.0f, 0, 0);
    s.tOut.set(-20.1f, 0, 0);
    s.tWater.set(40.1f,0,0);

    v.check(s,s);

    test_validity_assert_range(s.tIn);
    test_validity_assert_range(s.rhIn);
    test_validity_assert_range(s.tOut);
    test_validity_assert_range(s.tWater);
}

void test_validity_rate_to_high(){
    Snapshot current;
    Snapshot prev;

    Validity validity;

    current.rhIn.set(15.0f,0,0);
    prev.rhIn.set(25.0f,0,0);
    current.rhIn.sampledAtMs = 60000;
    prev.rhIn.sampledAtMs = 0;

    validity.check(current,prev);

    TEST_ASSERT_FALSE(current.rhIn.valid);
    TEST_ASSERT_EQUAL_INT(FAULT_RATE,current.rhIn.fault);
}

void test_validity_sensor_stuck(){
    Snapshot snapshot;
    Validity validity;

    int limit = 30;

    snapshot.rhIn.set(15.0f,0,0);

    for(int iteration = 0;iteration < limit;iteration++){
        validity.check(snapshot,snapshot);
    }

    TEST_ASSERT_FALSE(snapshot.rhIn.valid);
    TEST_ASSERT_EQUAL_INT(FAULT_STUCK,snapshot.rhIn.fault);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_crc8_datasheet_vector);
    RUN_TEST(test_decode_roundtrip);
    RUN_TEST(test_decode_rejects_bad_crc);
    RUN_TEST(test_decode_boundary_words);
    RUN_TEST(test_emulator_steady_decodes_badcrc_fails);
    RUN_TEST(test_ntc_midpoint_is_25c);
    RUN_TEST(test_ntc_hand_computed_points);
    RUN_TEST(test_ntc_short_open_are_faults);
    RUN_TEST(test_payload_null_for_invalid_and_fixed_point);
    RUN_TEST(test_validity_out_of_range);
    RUN_TEST(test_validity_rate_to_high);
    RUN_TEST(test_validity_sensor_stuck);
    return UNITY_END();
}
