# IoT25S Projekt 1 - MicroHydros climate node

Firmware for an Arduino Uno R4 WiFi that measures inside air temperature
and humidity (SHT40, emulated for now), outside air temperature (DS18B20)
and nutrient water temperature (NTC thermistor), validates the readings and
publishes them over MQTT.

## Plain-language descriptions for the project board

A few terms that appear throughout, explained once:

- The node: the Arduino Uno R4 WiFi board with the three sensors on it.
- Indoor sensor (SHT40): measures air temperature and humidity inside the
  grow space. We do not have the physical part yet, so the firmware
  contains a software stand-in (the "emulator") that produces realistic
  fake readings.
- Outdoor sensor (DS18B20): a digital temperature sensor on a cable,
  measuring air temperature outside the grow space.
- Water sensor (NTC): a small resistor whose resistance changes with
  temperature, sealed into a waterproof probe and dipped in the nutrient
  water.
- The broker (Mosquitto): a small message post office on a PC. The node
  sends its readings there; the backend picks them up.
- The backend (Node-RED + SQLite): the program that receives the readings,
  stores them in a database file, and shows them on a web dashboard.
- Fault code: a small number attached to each reading that says "this
  value is fine" (0) or why it is not (sensor missing, out of range, and
  so on).

---

## Build

PlatformIO project. Install the PlatformIO IDE extension in VS Code or
PlatformIO Core (`pio`).

**One time**
```
    cp src/secrets.h.example src/secrets.h   # fill in Wi-Fi and broker
```
**pio commands:**
```
    pio run                                  # build for uno_r4_wifi
    pio run -t upload                        # flash the board
    pio device monitor                       # serial log, 115200 baud
    pio test -e native                       # PC tests of the pure modules
    pio check -e uno_r4_wifi                 # Runs cppcheck and clangtidy
    or
    pio check -e native
```

Libraries are declared in `platformio.ini` and fetched automatically:
ArduinoMqttClient, OneWire, DallasTemperature. WiFiS3, Wire and
Arduino_LED_Matrix come with the board core.

## Layout

    src/          Arduino-dependent code, one directory per area:
      main.cpp, config.h        scheduler, state machine, settings
      sensors/                  Ds18b20Sensor, NtcSensor, SensorManager
      sensors/sht4x/            Sht4xSensor (adapter), Sht4xHardware (I2C);
                                the emulator and decoder are in lib/Sht4x
      net/                      Network (Wi-Fi), Telemetry (MQTT)
      ui/                       StatusLed (LED matrix), SerialCommand
    lib/          Pure C++ modules that also compile on a PC: Reading types,
                  driver contract, SHT4x decoder and emulator, NTC math,
                  Validity, Payload (JSON builder)
    test/         Unity tests for lib/, run in the native environment
    examples/     Unmodified library examples the modules were derived from
                  (reference only, not built)
    backend/      docker-compose for Mosquitto + Node-RED, see backend/README.md

## Where to start

The headers carry the contracts; the bodies marked `TODO(<area>)` are the
work. Headers under `src/` subdirectories are included with their directory
(`#include "net/Network.h"`); `lib/` headers are included bare. Grep for
your tag:

    TODO(network)    net/Network.cpp: Wi-Fi connect / retry
    TODO(telemetry)  net/Telemetry.cpp: MQTT connect, Last Will, publish, cmd
    TODO(ds18b20)    sensors/Ds18b20Sensor.cpp: async 1-Wire read and fault mapping
    TODO(firmware)   main.cpp: state machine, publish trigger, LED, commands
    TODO(serial)     ui/SerialCommand.cpp: line parser
    TODO(led)        ui/StatusLed.cpp: frames per state
    TODO(validity)   Validity.cpp: range, rate, stuck
    TODO(emulator)   Sht4xSimulated.cpp: scenario model

Each `examples/` directory named in a TODO is the library example to adapt.
The pure modules in `lib/` have tests in `test/test_native`; add a test
when you fill in a `lib/` body. Note that `snprintf("%f")` prints nothing
on this core (newlib-nano), which is why Payload formats numbers itself.

## Contributing

Nothing goes into `main` directly: branch, pull request, one approval,
squash merge. Step by step in `docs/workflow.md`. Native tests and how
to extend them: `docs/native_tests.md`.

## Runtime notes

- Every sample (default every 10 s) is published as one flat JSON object
  on `microhydros/<device_id>/telemetry`. There is no averaging on the
  node; the backend stores the series and averages in queries.
- Commands go over the USB serial port at 115200 baud, one per line:
  `scenario <steady|heatup|cooldown|stuck|badcrc>` (emulator only),
  `sample <seconds>`, `help`. The MQTT cmd topic is not subscribed.
- Wi-Fi association and the MQTT connect block for a few seconds each.
  Sampling pauses during a reconnect attempt and resumes afterwards.

Wiring, sensor motivation, MQTT topics and the test plan are in the
architecture outline (docs to follow).
