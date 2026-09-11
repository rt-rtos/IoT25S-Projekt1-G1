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
   Node by node below.
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

## Ingest flow in detail (step 2)

Four nodes wired left to right. Each one reads or rewrites `msg.payload`
and `msg.topic` and passes the message on.

1. `mqtt in`. Broker host is `mosquitto`, not `localhost`: Node-RED runs
   in its own container, and Compose gives each service a DNS name equal
   to its service name. Topic `microhydros/+/telemetry`; `+` matches one
   level, so every node id but not the `status` topic. Output:
   `msg.topic` is the full topic, `msg.payload` the JSON as a string.

2. `json`. Parses the string into an object, so `msg.payload.t_in` is
   24.1 and `msg.payload.t_water` is `null`. Optional (`mqtt in` can
   output a parsed object), kept so a parse error has a visible place to
   land.

3. `function`. Builds the row. The payload lacks two columns: the device
   id, which is the middle part of the topic, and the timestamp, which
   the backend supplies because the node has no clock. Values go into
   `msg.params` with `$` names for the prepared statement:

       const parts = msg.topic.split("/");   // ["microhydros", "node01", "telemetry"]
       const p = msg.payload;
       msg.params = {
           $received_at:   new Date().toISOString(),
           $device_id:     parts[1],
           $seq:           p.seq,
           $uptime_s:      p.uptime_s,
           $t_in:          p.t_in,
           $rh_in:         p.rh_in,
           $t_out:         p.t_out,
           $t_water:       p.t_water,
           $fault_t_in:    p.faults.t_in,
           $fault_rh_in:   p.faults.rh_in,
           $fault_t_out:   p.faults.t_out,
           $fault_t_water: p.faults.t_water,
       };
       return msg;

   A JSON `null` becomes a JavaScript `null` and is bound as SQL `NULL`,
   which is why an invalid channel is `null` in the payload and not a
   sentinel number.

4. `sqlite` (`node-red-node-sqlite`). Mode "Prepared Statement", database
   `/data/telemetry.db`, statement:

       INSERT INTO telemetry (received_at, device_id, seq, uptime_s,
                              t_in, rh_in, t_out, t_water,
                              fault_t_in, fault_rh_in, fault_t_out, fault_t_water)
       VALUES ($received_at, $device_id, $seq, $uptime_s,
               $t_in, $rh_in, $t_out, $t_water,
               $fault_t_in, $fault_rh_in, $fault_t_out, $fault_t_water);

   Bound values are never pasted into the SQL text, so strings and nulls
   need no quoting. Building the SQL string in the function instead
   ("Via msg.topic" mode) works but is where quoting bugs come from.

The table must exist before the first insert: a second flow, `inject`
set to fire once at startup -> `sqlite` with the `CREATE TABLE IF NOT
EXISTS` from step 3, same database file.

Why `/data`: the compose file mounts the `node-red-data` volume there.
The rest of the container filesystem is discarded whenever the container
is recreated (image update, config change), and a database anywhere else
loses its rows with it.

Checking: a `debug` node on the function output shows `msg.params` in
the sidebar; publish the sample message from "Start" and the row should
appear. An `inject` -> `sqlite` in "Via msg.topic" mode sending
`SELECT * FROM telemetry ORDER BY received_at DESC LIMIT 5` makes a
query button in the editor.
