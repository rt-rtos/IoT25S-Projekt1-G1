# Test records

What / how / expected / actual for the tests in outline section 11
(brief 10.4). The component tests run on the PC (`docs/native_tests.md`);
everything below runs on the board with the serial monitor open:

    pio device monitor --filter time

The `time` filter prefixes every line with the PC clock, which is what the
actual column needs. Card D10 adds the remaining levels; this file starts
with the reconnect tests from card N5.

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

## System

No entries yet. Tests 1 to 6 in outline 11; card D11.
