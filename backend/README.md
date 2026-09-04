# Backend

Mosquitto + Node-RED + SQLite in one docker-compose (outline section 6).
Owner: backend area. This directory is scaffolding; the flow, table and
dashboard are still to be built.

## Start

    cd backend
    docker compose up -d

- Broker: `localhost:1883`, anonymous allowed (dev only).
- Node-RED editor: http://localhost:1880

Check that the broker answers, from any machine with the mosquitto clients
installed (`apt install mosquitto-clients`):

    mosquitto_sub -h localhost -t 'microhydros/#' -v

Publish a fake node message to test the flow before the firmware is ready:

    mosquitto_pub -h localhost -t microhydros/node01/telemetry -m \
      '{"seq":1,"uptime_s":10,"t_in":24.1,"rh_in":61.0,"t_out":20.3,"t_water":null,"faults":{"t_in":0,"rh_in":0,"t_out":0,"t_water":1},"src":{"sht40":"sim","ds18b20":"hw","ntc":"hw"}}'

## What the node sends

Topics (`<device_id>` is `node01` by default, see `src/config.h`):

- `microhydros/<device_id>/telemetry`: one JSON object per sample,
  default every 10 s. Format is defined in `lib/Payload/Payload.h`.
  Invalid channels are `null` with a non-zero fault code; the codes are
  listed in `lib/Reading/Reading.h`.
- `microhydros/<device_id>/status`: `online` / `offline`, retained;
  `offline` is the broker's Last Will when the node drops.

The node has no clock. The backend stamps each row on ingest; `seq` and
`uptime_s` are for ordering and gap detection.

## Suggested next steps

1. In Node-RED, Manage palette -> install `node-red-node-sqlite` and
   `node-red-dashboard` (or `@flowfuse/node-red-dashboard`). If the sqlite
   node fails to build in the container, add a `Dockerfile` based on
   `nodered/node-red` that installs `python3 make g++` first.
2. Flow: `mqtt in` (`microhydros/+/telemetry`) -> `json` -> `function`
   (pull `device_id` from `msg.topic`, add `received_at`) -> `sqlite`
   (`INSERT`). Keep the SQLite file under `/data` so it lives in the volume.
3. Table, matching the flat payload:

       CREATE TABLE IF NOT EXISTS telemetry (
         received_at TEXT NOT NULL,
         device_id   TEXT NOT NULL,
         seq         INTEGER,
         uptime_s    INTEGER,
         t_in REAL, rh_in REAL, t_out REAL, t_water REAL,
         fault_t_in INTEGER, fault_rh_in INTEGER,
         fault_t_out INTEGER, fault_t_water INTEGER
       );

4. Second `mqtt in` on `microhydros/+/status` for a last-seen indicator.
5. Dashboard: current values with fault marking, 24 h chart per channel.
   Averages for trend views are `SELECT ... AVG(...) GROUP BY` over the
   stored rows, not something the node does.
6. Export the flow (Menu -> Export -> all flows) to `node-red/flows.json`
   and commit it so the setup is reproducible.
