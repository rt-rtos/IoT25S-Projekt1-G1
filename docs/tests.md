# Test records

What / how / expected / actual for the tests in outline section 11
(brief 10.4). The component tests run on the PC (`docs/native_tests.md`);
everything below runs on the board with the serial monitor open:

    pio device monitor --filter time

The `time` filter prefixes every line with the PC clock, which is what the
actual column needs. Card D10 adds the remaining levels; this file starts
with the reconnect tests from card N5.

## Useful Commands

**Live Logs**
        
    docker compose exec mosquitto   mosquitto_sub -h localhost -p 1883 -t 'microhydros/#' -v

**Build or Rebuild + Test Backend**
    
    ./check.sh

**Check Backend Logs for Connections**

    docker compose logs -f mosquitto

**Serial Monitor with successful config**

```
MicroHydros climate node node01 (type 'help')
state: Boot -> WifiConnecting
state: WifiConnecting -> MqttConnecting
SSID: RasmusSSID
RSSI: -43
IP: 192.168.1.2
mqtt: connected
state: MqttConnecting -> Online
```



## Integration: reconnect (card N5)

Setup: the node on the bench with `SHT4X_SIMULATED 1`, the broker from
`backend/docker-compose.yml` on a PC on the same LAN, and on that PC

    mosquitto_sub -h <host> -t 'microhydros/#' -v

running for the whole session. Timings assume the defaults in
`src/config.h`: `WIFI_RETRY_MS` 5 s, `MQTT_RETRY_MS` 5 s,
`MQTT_RETRY_MAX_MS` 60 s, keep-alive 60 s, sample 10 s. The MQTT retry
schedule after a failed connect is 5, 10, 20, 40, 60, 60, ... s, so the
time to notice a returned broker grows with the outage length (test 6).

The `state:` lines come from the transition log in `runStateMachine()`
(card F2). Without it the `mqtt:` lines and the `seq` gap are the only
evidence.

| # | What | How | Expected | Actual |
| - | ---- | --- | -------- | ------ |
| 1 | Broker stopped and restarted | While Online: `docker compose stop mosquitto`, wait 60 s, `docker compose start mosquitto` | Within one loop pass after the stop: `state: Online -> MqttConnecting`, then `mqtt: connect failed, error -2, next try in ms 5000`, then the same with 10000, 20000, 40000 at 5, 15, 35 s. The attempt at 75 s finds the broker: `mqtt: connected`, `status online` on the PC, telemetry resumes with a `seq` gap of 7 or 8 | |
| 2 | Wi-Fi AP off and on | While Online: power the AP (or the phone hotspot) off, wait 60 s, on | `state: Online -> WifiConnecting` within a poll after the module reports the loss; `WiFi.begin()` every 5 s plus its own blocking time. After the AP returns: `WifiConnecting -> MqttConnecting`, `mqtt: connected` on the first attempt, `-> Online`. `seq` gap of about 6 plus the association time | |
| 3 | Last Will | While Online: pull the node's USB | `microhydros/node01/status offline` on the PC within 90 s (1.5 x keep-alive, the broker has to time the dead socket out) | |
| 4 | Clean disconnect | While Online: press the board's reset | `status offline` at once (the socket closes), `status online` again after boot and connect | |
| 5 | Sampling continues while down | During test 1, watch the serial log | One `#seq` line per 10 s throughout the outage; `seq` never skips on serial, only on the broker | |
| 6 | Back-off | Test 1 with the broker left stopped for 5 min | `next try in ms` 5000, 10000, 20000, 40000, 60000, 60000, ...; attempts at 0, 5, 15, 35, 75, 135, 195, 255, 315 s after the stop. Reconnect at the first attempt after the broker returns, up to 60 s later. Each attempt blocks the loop until the connect is refused, so sampling pauses briefly around each `mqtt:` line | |

Notes for the actual column: paste the timestamped `state:` and `mqtt:`
lines and the first `seq` before and after the gap. For test 3 write
down the keep-alive arithmetic next to the observed delay.
One entry per test, as what / how / expected / actual. The
test plan, with the levels and the list of tests each level should end
up with, is outline section 11. The native tests and how to add to them
are in `native_tests.md`. Card D10 owns this file; each card that closes
with a test adds its own entry.

An entry names the command or the document steps under "how" so anyone
can repeat it, and puts the observed output under "actual" with the date
and who ran it.

## Component

Recorded per test function in `native_tests.md` section 4. The count and
date of the last known run are in outline 11.

## Glue

### E4 Snapshot -> JSON -> SQLite mapping

What: the four channel names and their fault codes use the same keys in
the same order at every step from the node to the database, so a value
cannot land in the wrong column.

How, static part: compare the key list in `lib/Payload/Payload.cpp`
(`buildTelemetryJson`) with the pinned string in
`test/test_native/test_main.cpp`, the fake message `backend/payload.json`,
the `row builder` function and the `CREATE TABLE` statement in
`backend/node-red/flows.json`, and the table in outline section 6.

How, ingest part: `backend/check.sh` with the stack running, or the four
manual checks in `backend/README.md` under "Verify the backend". Step 6
of the same README is the in-editor version of the last check.

Expected: `seq`, `uptime_s`, `t_in`, `rh_in`, `t_out`, `t_water` and
`faults.{t_in,rh_in,t_out,t_water}` in the JSON; columns `seq`,
`uptime_s`, `t_in`, `rh_in`, `t_out`, `t_water`, `fault_t_in`,
`fault_rh_in`, `fault_t_out`, `fault_t_water` in the table, plus
`received_at` and `device_id` added on ingest. `check.sh` ends with
`backend verified`; the newest row is `node01` with `t_water` NULL and
`fault_t_water` 1.

Actual, static part (2026-09-18, Rasmus): the names match one to one in
all five places. `payload.json` is byte-identical to the documented
format, 176 bytes.

Actual, ingest part (Rasmus, 2026-09-16): `check.sh` passed all four
checks.

Notes:

- A board-to-row run is not part of this entry: `telemetry.publish` is
  still a `TODO(firmware)` in `src/main.cpp` (card F3). That run belongs
  to system test 1 (card D11).

## Integration

No entries yet. Outline 11 lists the tests; they are run on the board
and logged from the serial monitor.

### End to End: Arduino to Backend test

Ran full Demo workflow with all connected hardware verifying end to end behaviour. Arduino boots, connects, sends payloads to the backend telemtry.db.

Node01 Connected:

    microhydros-mosquitto  | 1789732396: New connection from 172.18.112.1:59348 on port 1883.
    microhydros-mosquitto  | 1789732396: New client connected from 172.18.112.1:59348 as node01 (p4, c1, k60).

Payload Verification Logs:

    mosquitto_sub -h localhost -p 1883 -t 'microhydros/#' -v
    microhydros/node01/status online

**MD Table Format**

| seq | uptime_s | t_in | rh_in | t_out | t_water | fault t_in | fault rh_in | fault t_out | fault t_water | sht40 | ds18b20 | ntc |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|---|---|
| 19 | 191 | 22.0 | 65.0 | 26.1 | 24.6 | 0 | 0 | 0 | 0 | sim | hw | hw |
| 20 | 201 | 22.0 | 65.0 | 26.3 | 24.6 | 0 | 0 | 0 | 0 | sim | hw | hw |
| 21 | 211 | 22.0 | 65.0 | 26.3 | 24.6 | 0 | 0 | 0 | 0 | sim | hw | hw |
| 22 | 221 | 22.0 | 65.0 | 26.3 | 24.6 | 0 | 0 | 0 | 0 | sim | hw | hw |
| 23 | 231 | 22.0 | 65.0 | 26.3 | 24.6 | 0 | 0 | 0 | 0 | sim | hw | hw |
| 24 | 241 | 22.0 | 65.0 | 26.3 | 24.6 | 0 | 0 | 0 | 0 | sim | hw | hw |
| 25 | 251 | 22.0 | 65.0 | 26.3 | 24.6 | 0 | 0 | 0 | 0 | sim | hw | hw |
| 26 | 261 | 22.0 | 65.0 | 26.3 | 24.6 | 0 | 0 | 0 | 0 | sim | hw | hw |
| 27 | 271 | 22.0 | 65.0 | 26.1 | 24.6 | 0 | 0 | 0 | 0 | sim | hw | hw |
| 28 | 281 | 22.0 | 65.0 | 26.1 | 24.6 | 0 | 0 | 0 | 0 | sim | hw | hw |
| 29 | 291 | 22.0 | 65.0 | 26.1 | 24.6 | 0 | 0 | 0 | 0 | sim | hw | hw |
| 30 | 301 | null | null | 26.3 | 24.6 | 6 | 6 | 0 | 0 | sim | hw | hw |
| 31 | 311 | 22.0 | 65.0 | 26.4 | 24.6 | 0 | 0 | 0 | 0 | sim | hw | hw |
| 32 | 321 | 22.0 | 65.0 | 26.4 | 24.6 | 0 | 0 | 0 | 0 | sim | hw | hw |
| 33 | 331 | 22.0 | 65.0 | 26.4 | 24.6 | 0 | 0 | 0 | 0 | sim | hw | hw |

**JSON Live Logs**

<details>

```js

{
  "seq": 19,
  "uptime_s": 191,
  "t_in": 22.0,
  "rh_in": 65.0,
  "t_out": 26.1,
  "t_water": 24.6,
  "faults": {
    "t_in": 0,
    "rh_in": 0,
    "t_out": 0,
    "t_water": 0
  },
  "src": {
    "sht40": "sim",
    "ds18b20": "hw",
    "ntc": "hw"
  }
}
{
  "seq": 20,
  "uptime_s": 201,
  "t_in": 22.0,
  "rh_in": 65.0,
  "t_out": 26.3,
  "t_water": 24.6,
  "faults": {
    "t_in": 0,
    "rh_in": 0,
    "t_out": 0,
    "t_water": 0
  },
  "src": {
    "sht40": "sim",
    "ds18b20": "hw",
    "ntc": "hw"
  }
}
{
  "seq": 21,
  "uptime_s": 211,
  "t_in": 22.0,
  "rh_in": 65.0,
  "t_out": 26.3,
  "t_water": 24.6,
  "faults": {
    "t_in": 0,
    "rh_in": 0,
    "t_out": 0,
    "t_water": 0
  },
  "src": {
    "sht40": "sim",
    "ds18b20": "hw",
    "ntc": "hw"
  }
}
{
  "seq": 22,
  "uptime_s": 221,
  "t_in": 22.0,
  "rh_in": 65.0,
  "t_out": 26.3,
  "t_water": 24.6,
  "faults": {
    "t_in": 0,
    "rh_in": 0,
    "t_out": 0,
    "t_water": 0
  },
  "src": {
    "sht40": "sim",
    "ds18b20": "hw",
    "ntc": "hw"
  }
}
{
  "seq": 23,
  "uptime_s": 231,
  "t_in": 22.0,
  "rh_in": 65.0,
  "t_out": 26.3,
  "t_water": 24.6,
  "faults": {
    "t_in": 0,
    "rh_in": 0,
    "t_out": 0,
    "t_water": 0
  },
  "src": {
    "sht40": "sim",
    "ds18b20": "hw",
    "ntc": "hw"
  }
}
{
  "seq": 24,
  "uptime_s": 241,
  "t_in": 22.0,
  "rh_in": 65.0,
  "t_out": 26.3,
  "t_water": 24.6,
  "faults": {
    "t_in": 0,
    "rh_in": 0,
    "t_out": 0,
    "t_water": 0
  },
  "src": {
    "sht40": "sim",
    "ds18b20": "hw",
    "ntc": "hw"
  }
}
{
  "seq": 25,
  "uptime_s": 251,
  "t_in": 22.0,
  "rh_in": 65.0,
  "t_out": 26.3,
  "t_water": 24.6,
  "faults": {
    "t_in": 0,
    "rh_in": 0,
    "t_out": 0,
    "t_water": 0
  },
  "src": {
    "sht40": "sim",
    "ds18b20": "hw",
    "ntc": "hw"
  }
}
{
  "seq": 26,
  "uptime_s": 261,
  "t_in": 22.0,
  "rh_in": 65.0,
  "t_out": 26.3,
  "t_water": 24.6,
  "faults": {
    "t_in": 0,
    "rh_in": 0,
    "t_out": 0,
    "t_water": 0
  },
  "src": {
    "sht40": "sim",
    "ds18b20": "hw",
    "ntc": "hw"
  }
}
{
  "seq": 27,
  "uptime_s": 271,
  "t_in": 22.0,
  "rh_in": 65.0,
  "t_out": 26.1,
  "t_water": 24.6,
  "faults": {
    "t_in": 0,
    "rh_in": 0,
    "t_out": 0,
    "t_water": 0
  },
  "src": {
    "sht40": "sim",
    "ds18b20": "hw",
    "ntc": "hw"
  }
}
{
  "seq": 28,
  "uptime_s": 281,
  "t_in": 22.0,
  "rh_in": 65.0,
  "t_out": 26.1,
  "t_water": 24.6,
  "faults": {
    "t_in": 0,
    "rh_in": 0,
    "t_out": 0,
    "t_water": 0
  },
  "src": {
    "sht40": "sim",
    "ds18b20": "hw",
    "ntc": "hw"
  }
}
{
  "seq": 29,
  "uptime_s": 291,
  "t_in": 22.0,
  "rh_in": 65.0,
  "t_out": 26.1,
  "t_water": 24.6,
  "faults": {
    "t_in": 0,
    "rh_in": 0,
    "t_out": 0,
    "t_water": 0
  },
  "src": {
    "sht40": "sim",
    "ds18b20": "hw",
    "ntc": "hw"
  }
}
{
  "seq": 30,
  "uptime_s": 301,
  "t_in": null,
  "rh_in": null,
  "t_out": 26.3,
  "t_water": 24.6,
  "faults": {
    "t_in": 6,
    "rh_in": 6,
    "t_out": 0,
    "t_water": 0
  },
  "src": {
    "sht40": "sim",
    "ds18b20": "hw",
    "ntc": "hw"
  }
}
{
  "seq": 31,
  "uptime_s": 311,
  "t_in": 22.0,
  "rh_in": 65.0,
  "t_out": 26.4,
  "t_water": 24.6,
  "faults": {
    "t_in": 0,
    "rh_in": 0,
    "t_out": 0,
    "t_water": 0
  },
  "src": {
    "sht40": "sim",
    "ds18b20": "hw",
    "ntc": "hw"
  }
}
{
  "seq": 32,
  "uptime_s": 321,
  "t_in": 22.0,
  "rh_in": 65.0,
  "t_out": 26.4,
  "t_water": 24.6,
  "faults": {
    "t_in": 0,
    "rh_in": 0,
    "t_out": 0,
    "t_water": 0
  },
  "src": {
    "sht40": "sim",
    "ds18b20": "hw",
    "ntc": "hw"
  }
}
{
  "seq": 33,
  "uptime_s": 331,
  "t_in": 22.0,
  "rh_in": 65.0,
  "t_out": 26.4,
  "t_water": 24.6,
  "faults": {
    "t_in": 0,
    "rh_in": 0,
    "t_out": 0,
    "t_water": 0
  },
  "src": {
    "sht40": "sim",
    "ds18b20": "hw",
    "ntc": "hw"
  }
}
```

</details>

**JSON Long Soak - NTC + Dallas align**

```js
{
  "seq": 160,
  "uptime_s": 1605,
  "t_in": 22.0,
  "rh_in": 65.0,
  "t_out": 25.2,
  "t_water": 25.0,
  "faults": {
    "t_in": 0,
    "rh_in": 0,
    "t_out": 0,
    "t_water": 0
  },
  "src": {
    "sht40": "sim",
    "ds18b20": "hw",
    "ntc": "hw"
  }
}
{
  "seq": 161,
  "uptime_s": 1615,
  "t_in": 22.0,
  "rh_in": 65.0,
  "t_out": 25.2,
  "t_water": 25.0,
  "faults": {
    "t_in": 0,
    "rh_in": 0,
    "t_out": 0,
    "t_water": 0
  },
  "src": {
    "sht40": "sim",
    "ds18b20": "hw",
    "ntc": "hw"
  }
}
{
  "seq": 162,
  "uptime_s": 1625,
  "t_in": 22.0,
  "rh_in": 65.0,
  "t_out": 25.2,
  "t_water": 25.1,
  "faults": {
    "t_in": 0,
    "rh_in": 0,
    "t_out": 0,
    "t_water": 0
  },
  "src": {
    "sht40": "sim",
    "ds18b20": "hw",
    "ntc": "hw"
  }
}
{
  "seq": 163,
  "uptime_s": 1635,
  "t_in": 22.0,
  "rh_in": 65.0,
  "t_out": 25.3,
  "t_water": 25.2,
  "faults": {
    "t_in": 0,
    "rh_in": 0,
    "t_out": 0,
    "t_water": 0
  },
  "src": {
    "sht40": "sim",
    "ds18b20": "hw",
    "ntc": "hw"
  }
}
{
  "seq": 164,
  "uptime_s": 1645,
  "t_in": 22.0,
  "rh_in": 65.0,
  "t_out": 25.4,
  "t_water": 25.2,
  "faults": {
    "t_in": 0,
    "rh_in": 0,
    "t_out": 0,
    "t_water": 0
  },
  "src": {
    "sht40": "sim",
    "ds18b20": "hw",
    "ntc": "hw"
  }
}
```


## System

No entries yet. Tests 1 to 6 in outline 11; card D11.
