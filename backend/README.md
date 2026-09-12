# Backend

Mosquitto + Node-RED + SQLite in one docker-compose (outline section 6).
Owner: backend area. The flow is built node by node in the Node-RED
editor following the steps below, then exported to `node-red/flows.json`.

## Start

    cd backend
    docker compose up -d

- Broker: `localhost:1883`, anonymous allowed (dev only).
- Node-RED editor: http://localhost:1880

Check that the broker answers, from any machine with the mosquitto clients
installed (`apt install mosquitto-clients`, or the Windows installer from
mosquitto.org). Two terminals: the subscriber blocks and prints nothing
until a message arrives, the publisher sends one message and exits
silently.

Terminal 1:

    mosquitto_sub -h localhost -t "microhydros/#" -v

Terminal 2, from this directory. `payload.json` is the fake node message
used by every check below:

    mosquitto_pub -h localhost -t microhydros/node01/telemetry -f payload.json

Terminal 1 should print the topic followed by the JSON on one line.

## export and import the flow

Node-RED keeps its live `flows.json` in the `node-red-data` volume at
`/data`. Nothing in this directory is mounted into the container, so
`node-red/flows.json` in the repo is a snapshot: export it when the flow
changes, commit it, and import it on another machine. The broker config
comes with it; credentials do not, but the dev broker has none.

Export, from this directory, with the containers running:

    docker compose cp node-red:/data/flows.json node-red/flows.json

Import on another machine, then restart so Node-RED loads the new file:

    docker compose cp node-red/flows.json node-red:/data/flows.json
    docker compose restart node-red

The import overwrites whatever flow that machine had. Export first if any
of it is worth keeping.

The editor works too: Menu -> Export -> "all flows" -> Download, and
Menu -> Import -> select the file -> Deploy. Same file format.

Before exporting, delete the Docker image's default "WARNING: please
check you have started this container with a volume" comment node so it
does not end up in the repo.

The SQLite file itself stays in the volume and is not committed.


Windows notes:

- Use double quotes around the topic. In cmd single quotes are literal
  characters and the subscription silently never matches.
- Publish from the file, not with `-m`. Neither cmd nor PowerShell passes
  the JSON's inner double quotes through to the exe unchanged.
- If you copy the JSON into your own file, save it as UTF-8 without BOM.
  PowerShell 5 redirection writes UTF-16 and Notepad may add a BOM; both
  make the message unparseable even though the text looks identical.
  A correct file is 176 bytes plus at most one newline.
- The Windows mosquitto installer registers a broker service on port
  1883. If it is running, the docker broker cannot bind the port and
  Node-RED never sees your messages. Check `netstat -ano | findstr 1883`
  and stop the service.

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

## Step 1: palette

Menu -> Manage palette -> Install tab. Install:

- `node-red-node-sqlite`
- `@flowfuse/node-red-dashboard` (Dashboard 2.0; the older
  `node-red-dashboard` also works but is no longer maintained)

The sqlite node compiles a native module on install and can take a few
minutes. If it fails in the container, add a `Dockerfile` next to the
compose file:

    FROM nodered/node-red:latest
    USER root
    RUN apk add --no-cache python3 make g++
    USER node-red

and replace `image: nodered/node-red:latest` with `build: .` under the
`node-red` service, then `docker compose up -d --build`.

## Step 2: broker config and mqtt in

Drag an `mqtt in` node onto the flow, open it, click the pencil next to
Server and fill in the broker config:

- Connection tab: Name `microhydros`, Server `mosquitto`, Port `1883`,
  Protocol MQTT V3.1.1, Connect automatically checked, Use TLS unchecked.
- Security tab: username and password empty. The dev broker allows
  anonymous.
- Messages tab: defaults.

Server is `mosquitto`, not `localhost`. Node-RED runs in its own
container, where `localhost` is the container itself and nothing listens
on 1883. Compose gives each service a DNS name equal to its service name.
The `mosquitto_sub` on your PC uses `localhost` because the port is
published to the host.

Then the `mqtt in` node itself:

- Server: the config above
- Action: Subscribe to single topic
- Topic: `microhydros/+/telemetry` (`+` matches one level, so every node
  id but not the `status` topic)
- QoS: 0
- Output: `a parsed JSON object`
- Name: `telemetry in`

Deploy. The node shows a green "connected" badge within a few seconds.
With this output setting a malformed message errors on this node and
`msg.payload` is always an object downstream, so no separate `json` node
is needed. `msg.topic` carries the full topic string.

## Step 3: function node, row builder

Drag a `function` node, wire `telemetry in` to it, name it `row builder`,
leave Outputs at 1, and paste this into the On Message tab:

    // MQTT telemetry -> params for the sqlite prepared statement.
    // Topic is microhydros/<device_id>/telemetry; the middle part is the device.
    // The node has no clock, so received_at is stamped here on ingest.

    const parts = msg.topic.split("/");
    const p = msg.payload;

    if (parts.length < 3 || typeof p !== "object" || p === null) {
        node.warn("dropped message with bad topic or payload: " + msg.topic);
        return null;
    }

    const faults = p.faults || {};

    msg.params = {
        $received_at:   new Date().toISOString(),
        $device_id:     parts[1],
        $seq:           p.seq,
        $uptime_s:      p.uptime_s,
        $t_in:          p.t_in,
        $rh_in:         p.rh_in,
        $t_out:         p.t_out,
        $t_water:       p.t_water,
        $fault_t_in:    faults.t_in,
        $fault_rh_in:   faults.rh_in,
        $fault_t_out:   faults.t_out,
        $fault_t_water: faults.t_water,
    };

    return msg;

A JSON `null` becomes a JavaScript `null` and is bound as SQL `NULL`,
which is why an invalid channel is `null` in the payload and not a
sentinel number. A missing key gives `undefined`, which the sqlite node
also binds as `NULL`, so the insert still succeeds.

## Step 4: sqlite insert

Drag a `sqlite` node, wire `row builder` to it, and configure:

- Database: click the pencil, Database `/data/telemetry.db`, Mode
  `Read-Write-Create`.
- SQL Query: `Prepared Statement`
- SQL: the statement below
- Name: `insert telemetry`

    INSERT INTO telemetry (received_at, device_id, seq, uptime_s,
                           t_in, rh_in, t_out, t_water,
                           fault_t_in, fault_rh_in, fault_t_out, fault_t_water)
    VALUES ($received_at, $device_id, $seq, $uptime_s,
            $t_in, $rh_in, $t_out, $t_water,
            $fault_t_in, $fault_rh_in, $fault_t_out, $fault_t_water);

Bound values are never pasted into the SQL text, so strings and nulls
need no quoting. Building the SQL string in the function instead
("Via msg.topic" mode) works but is where quoting bugs come from.

Why `/data`: the compose file mounts the `node-red-data` volume there.
The rest of the container filesystem is discarded whenever the container
is recreated (image update, config change), and a database anywhere else
loses its rows with it.

## Step 5: create the table at startup

The table must exist before the first insert. A second, separate chain
on the same flow tab:

- `inject`: check "Inject once after 0.1 seconds", Repeat none, name
  `create table`. Payload and topic do not matter.
- `sqlite`: same database config as step 4, SQL Query `Fixed Statement`,
  name `create telemetry`, SQL:

    CREATE TABLE IF NOT EXISTS telemetry (
      received_at TEXT NOT NULL,
      device_id   TEXT NOT NULL,
      seq         INTEGER,
      uptime_s    INTEGER,
      t_in REAL, rh_in REAL, t_out REAL, t_water REAL,
      fault_t_in INTEGER, fault_rh_in INTEGER,
      fault_t_out INTEGER, fault_t_water INTEGER
    );

The inject fires on every deploy of the flow, and `IF NOT EXISTS` makes
that harmless.

## Step 6: check the ingest

- Wire a `debug` node to `row builder`, set Output to "complete msg
  object". Deploy.
- Publish `payload.json` from the Start section. The debug sidebar shows
  `msg.params` with `$t_water: null` and `$fault_t_water: 1`.
- Add a query button: `inject` (fire on click) -> `sqlite` in
  `Fixed Statement` mode with
  `SELECT * FROM telemetry ORDER BY received_at DESC LIMIT 5`
  -> `debug`. Click the inject button; the rows appear as an array.

Done when one row exists with `t_water` NULL and `fault_t_water` 1
(backlog card B2).

## Step 7: status topic and last-seen (card B3)

What the node does: on connect it publishes retained `online` on
`microhydros/<device_id>/status`, and it registers `offline` on the same
topic as the broker's Last Will. If the node disappears without a clean
disconnect (power loss, Wi-Fi drop) the broker publishes `offline` itself
once the keepalive expires; keepalive is 60 s (`src/config.h`), so the
dashboard sees the drop within about 90 s. Retained means a new subscriber
gets the current value at once, so the indicator is filled right after a
deploy without waiting for the node to say anything.

Nodes, a third chain on the same tab:

- `mqtt in`: Server from step 2, Topic `microhydros/+/status`, QoS 1,
  Output `a String`, Name `status in`.
- `function`, Name `last seen`, Outputs 1. Wire two inputs into it:
  `status in` and the existing `telemetry in`. Status messages set
  online/offline, telemetry messages stamp the time of the last sample.
  A node output can feed several nodes; the ingest chain is unaffected.

On Message tab of `last seen`:

    // Per-device last-seen table in flow context. Two inputs share this
    // node: status messages set online/offline, telemetry messages stamp
    // the last sample time. Output is the whole table for a ui-table.
    const parts = msg.topic.split("/");   // ["microhydros", "<device>", "status"|"telemetry"]
    const device = parts[1];
    const kind = parts[2];
    const now = new Date().toISOString();

    const seen = flow.get("lastSeen") || {};
    const entry = seen[device] ||
        { device: device, status: "unknown", last_status: "", last_sample: "" };

    if (kind === "status") {
        entry.status = String(msg.payload).trim();
        entry.last_status = now;
    } else if (kind === "telemetry") {
        entry.last_sample = now;
    }
    seen[device] = entry;
    flow.set("lastSeen", seen);

    msg.payload = Object.keys(seen).sort().map(d => seen[d]);
    return msg;

- `debug` on the output until the dashboard exists; in step 8 a
  `ui-table` takes its place.

Flow context lives in memory and is empty after a Node-RED restart. That
is fine: the retained status refills `status` on reconnect, and
`last_sample` refills with the next telemetry message.

Check with a fake node. `-r` sets the retained flag, the same as the
firmware does:

    mosquitto_pub -h localhost -t microhydros/node01/status -m online -r
    mosquitto_pub -h localhost -t microhydros/node01/status -m offline -r

Done when:

- The debug sidebar shows the array with `status` flipping on each
  publish and `last_status` updating.
- Publishing `payload.json` updates `last_sample` without touching
  `status`.
- After a redeploy, the current status reappears in the sidebar without
  a new publish. That is the retained flag doing its job.
- With the real board: pull the USB cable and the entry goes `offline`
  within about 90 s without anyone publishing.

To remove a stale retained status for a device that no longer exists,
publish an empty retained message:

    mosquitto_pub -h localhost -t microhydros/node01/status -n -r

## Step 8: dashboard (card B4)

Dashboard 2.0 (`@flowfuse/node-red-dashboard`) arranges widgets as
Base -> Page -> Group. Open the Dashboard 2.0 sidebar tab (the icon on
the right of the editor) and create:

- Base: path `/dashboard`
- Page: `MicroHydros`, layout Grid
- Groups on that page: `Current values`, `Charts`, `Status`

Every `ui-*` node below picks one of these groups. The dashboard is
served at http://localhost:1880/dashboard and updates live over a
websocket, no browser refresh needed.

### 8a: current values with fault marking

`function` after `telemetry in`, Name `current values`, Outputs 4:

    // One output per channel. A faulted channel shows the fault name
    // instead of a stale number; the codes are the Fault enum in
    // lib/Reading/Reading.h and the value is null in the payload.
    const FAULT = ["ok", "NO_DEVICE", "CRC", "TIMEOUT", "RANGE",
                   "RATE", "STUCK", "NOT_READY"];
    const p = msg.payload;
    const f = p.faults || {};
    const show = (v, code) => code ? "FAULT " + (FAULT[code] || code) : v;

    return [
        { payload: show(p.t_in,    f.t_in),    topic: "t_in" },
        { payload: show(p.rh_in,   f.rh_in),   topic: "rh_in" },
        { payload: show(p.t_out,   f.t_out),   topic: "t_out" },
        { payload: show(p.t_water, f.t_water), topic: "t_water" },
    ];

Four `ui-text` nodes, one per output, Group `Current values`, Format
`{{msg.payload}}`, Labels `Indoor C`, `Indoor RH %`, `Outdoor C`,
`Water C`. With `payload.json` the water tile reads `FAULT NO_DEVICE`
and the other three show numbers.

### 8b: 24 h chart per channel, live points

`function` after `telemetry in`, Name `chart points`, Outputs 4:

    // Live points. Series is the device id so several nodes share a chart.
    // Faulted channels are skipped rather than plotted as null.
    const device = msg.topic.split("/")[1];
    const p = msg.payload;
    const f = p.faults || {};
    const now = Date.now();
    const point = (v, fault) =>
        fault ? null : { topic: device, payload: { x: now, y: v } };

    return [
        point(p.t_in,    f.t_in),
        point(p.rh_in,   f.rh_in),
        point(p.t_out,   f.t_out),
        point(p.t_water, f.t_water),
    ];

Four `ui-chart` nodes, one per output, Group `Charts`, Labels as for the
text tiles, and on each:

- Type `Line`, Action `Append`
- X-axis: Type `Timescale`, Limit `24 hours` (this is what prunes old
  points; the field name varies slightly between versions)
- Series `msg.topic`, X property `x`, Y property `y`

### 8c: backfill the charts from the database

The chart keeps its points in memory and starts empty after a restart.
One more chain reloads the last day from SQLite:

- `inject`: "Inject once after 1 seconds", Name `load history`.
- `sqlite`, `Fixed Statement`, same database config as step 4:

    SELECT received_at, device_id, t_in, rh_in, t_out, t_water
    FROM telemetry
    WHERE received_at > strftime('%Y-%m-%dT%H:%M:%fZ', 'now', '-1 day')
    ORDER BY received_at;

- `function`, Name `history points`, Outputs 4:

    // Rows -> one array of {x, y} per chart. An empty array is sent first
    // so a redeploy does not duplicate points already on the chart.
    const rows = msg.payload || [];
    const cols = ["t_in", "rh_in", "t_out", "t_water"];
    const out = cols.map(() => ({}));

    for (const r of rows) {
        const x = Date.parse(r.received_at);
        cols.forEach((c, i) => {
            if (r[c] === null || r[c] === undefined) return;
            (out[i][r.device_id] = out[i][r.device_id] || []).push({ x, y: r[c] });
        });
    }

    // One message per (chart, device): the chart takes the series from msg.topic.
    cols.forEach((c, i) => {
        node.send(cols.map((_, j) => j === i ? { payload: [] } : null));
        for (const device of Object.keys(out[i])) {
            node.send(cols.map((_, j) =>
                j === i ? { topic: device, payload: out[i][device] } : null));
        }
    });
    return null;

Wire output 1 to the `t_in` chart, 2 to `rh_in`, 3 to `t_out`, 4 to
`t_water`, the same charts the live points go to.

### 8d: last-seen table

`ui-table` in Group `Status`, fed by the `last seen` function from step 7
(replace or keep the debug node). The table takes the array as it is;
columns are `device`, `status`, `last_status`, `last_sample`. Set
Action `Replace` so each message redraws the whole table.

### 8e: check

Publish a short series with changing values so the charts have something
to draw. From this directory:

    for i in $(seq 1 20); do
      jq -c --argjson i "$i" '.seq = $i | .uptime_s = $i * 10 | .t_in = 22 + ($i % 5) / 2' payload.json \
        | mosquitto_pub -h localhost -t microhydros/node01/telemetry -s
      sleep 1
    done

On Windows, publish `payload.json` a few times instead; a flat line is
still a line.

Done when:

- The four tiles update within one sample of a publish, and `Water C`
  reads `FAULT NO_DEVICE`.
- The three valid charts show the series; the water chart stays empty
  because every sample is faulted.
- `docker compose restart node-red`, then reload the dashboard: the
  charts come back with the stored points via 8c.
- The status table lists `node01` with the status from step 7.

### Trend averages

Averages are `AVG(...) GROUP BY` over stored rows, nothing on the node.
Hourly indoor mean for the last day, usable behind another
inject -> sqlite -> `history points`-style function -> chart:

    SELECT strftime('%Y-%m-%dT%H:00:00Z', received_at) AS hour,
           device_id, AVG(t_in) AS t_in
    FROM telemetry
    WHERE received_at > strftime('%Y-%m-%dT%H:%M:%fZ', 'now', '-1 day')
    GROUP BY hour, device_id
    ORDER BY hour;

