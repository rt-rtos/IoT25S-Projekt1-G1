#!/usr/bin/env bash
# Bring the backend up, restart Node-RED so it loads the current
# node-red/flows.json, and run the checks from README "Verify the backend".
#
# Needs docker compose; uses the host mosquitto clients if installed,
# otherwise the ones inside the mosquitto container.
set -euo pipefail
cd "$(dirname "$0")"
export MSYS_NO_PATHCONV=1   # Git Bash: keep /data/... arguments as they are

pass() { printf '  ok    %s\n' "$1"; }
fail() { printf '  FAIL  %s\n' "$1"; exit 1; }

# Node-RED reports UTC in its log; the same stamp filters `logs --since`.
since=$(date -u +%Y-%m-%dT%H:%M:%SZ)

echo "== docker compose up"
docker compose up -d --build

echo "== restart Node-RED to load node-red/flows.json"
docker compose restart node-red

echo "== wait for Node-RED"
for _ in $(seq 1 30); do
    docker compose logs --since "$since" node-red 2>/dev/null \
        | grep -q 'Connected to broker' && break
    sleep 1
done

echo "== 1. flow loaded"
nodes=$(docker compose exec -T node-red node -e \
    "const f=require('/data/repo/flows.json');console.log(f.map(n=>n.type+(n.name?' ['+n.name+']':'')).join('\n'))")
echo "$nodes" | sed 's/^/        /'
for n in 'mqtt in [telemetry in]' 'function [row builder]' \
         'sqlite [insert telemetry]' 'sqlite [create telemetry]'; do
    echo "$nodes" | grep -qF "$n" || fail "node missing: $n"
done
pass "all four nodes present"

echo "== 2. startup log"
log=$(docker compose logs --no-log-prefix --since "$since" node-red)
for line in 'Started flows' 'opened /data/telemetry.db ok' \
            'Connected to broker: mqtt://mosquitto:1883'; do
    echo "$log" | grep -qF "$line" || { echo "$log" | tail -20; fail "log line missing: $line"; }
done
pass "flows started, database opened, broker connected"

echo "== 3. broker delivers"
if command -v mosquitto_sub >/dev/null; then
    sub() { mosquitto_sub -h localhost "$@"; }
    pub() { mosquitto_pub -h localhost "$@"; }
    where="host clients"
else
    sub() { docker compose exec -T mosquitto mosquitto_sub "$@"; }
    pub() { docker compose exec -T mosquitto mosquitto_pub "$@"; }
    where="clients inside the mosquitto container"
fi
sub -t 'microhydros/#' -v -C 1 -W 10 > "${TMPDIR:-/tmp}/microhydros_sub.$$" &
subpid=$!
sleep 1
pub -t microhydros/node01/telemetry -s < payload.json
wait $subpid || fail "subscriber got nothing within 10 s ($where)"
grep -q 'microhydros/node01/telemetry {' "${TMPDIR:-/tmp}/microhydros_sub.$$" \
    || fail "subscriber output unexpected: $(cat "${TMPDIR:-/tmp}/microhydros_sub.$$")"
rm -f "${TMPDIR:-/tmp}/microhydros_sub.$$"
pass "payload.json delivered ($where)"

echo "== 4. row in SQLite"
for _ in $(seq 1 10); do
    row=$(docker compose exec -T -e SINCE="$since" node-red node -e "
        const s=require(require.resolve('sqlite3',{paths:['/usr/src/node-red/node_modules','/data/node_modules']}));
        new s.Database('/data/telemetry.db').get(
            'SELECT * FROM telemetry WHERE received_at > ? ORDER BY received_at DESC LIMIT 1', process.env.SINCE,
            (e,r)=>{ if(e){console.error(e.message);process.exit(2)} console.log(JSON.stringify(r||null)) })")
    [ "$row" != "null" ] && break
    sleep 1
done
echo "        $row"
[ "$row" != "null" ] || fail "no row newer than $since"
echo "$row" | grep -q '"device_id":"node01"' || fail "device_id is not node01"
echo "$row" | grep -q '"t_water":null' || fail "t_water should be null"
echo "$row" | grep -q '"fault_t_water":1' || fail "fault_t_water should be 1"
pass "row stored with t_water NULL and fault_t_water 1"

echo "backend verified"
