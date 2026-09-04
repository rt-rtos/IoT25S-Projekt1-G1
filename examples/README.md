# Reference examples

Unmodified copies of the library examples the firmware modules are derived
from. They are not part of the PlatformIO build (only `src/`, `lib/` and
`test/` are). Kept here so nobody has to dig through `.pio/libdeps` or the
framework package to see the original.

| Directory | Source library | Used for |
|---|---|---|
| `WiFiSimpleSender` | ArduinoMqttClient 0.1.8 | `Telemetry`: basic publish loop |
| `WiFiAdvancedCallback` | ArduinoMqttClient 0.1.8 | `Telemetry`: Last Will, retain, QoS, subscribe callback (cmd topic) |
| `WiFiSimpleReceiveCallback` | ArduinoMqttClient 0.1.8 | `Telemetry`: receive callback |
| `WaitForConversion2` | DallasTemperature 4.0.5 | `Ds18b20Sensor`: async (non-blocking) conversion |
| `Simple` | DallasTemperature 4.0.5 | `Ds18b20Sensor`: minimal read |
| `Tester` | DallasTemperature 4.0.5 | Bench check: device count, address, resolution, parasite power (clone detection) |
| `DS18x20_Temperature` | OneWire 2.3.8 | Raw 1-Wire protocol: presence, scratchpad, CRC-8, resolution bits |
| `ConnectWithWPA` | WiFiS3 (core 1.6.0) | `Network`: connect, status, RSSI |
| `WiFiUdpNtpClient` | WiFiS3 (core 1.6.0) | Future: NTP time on the node (outline section 12) |
| `RTC_NTPSync` | RTC (core 1.6.0) | Future: on-board RTC set from NTP |
| `DisplaySingleFrame` | Arduino_LED_Matrix (core 1.6.0) | `StatusLed`: frame format for the 12x8 matrix |
| `TextWithArduinoGraphicsAsynchronous` | Arduino_LED_Matrix (core 1.6.0) | Optional: scrolling text on the matrix |
| `BasicAutoConnect` | HiTECH R4 Wifi Manager 2026.6.20 | Optional: captive-portal Wi-Fi setup instead of `secrets.h` |

Licenses: Arduino examples are public domain or LGPL per the library; the
HiTECH example is under that library's LICENSE.
