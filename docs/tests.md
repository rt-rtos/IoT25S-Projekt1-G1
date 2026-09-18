# Test records

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
