#!/usr/bin/env bash
# Local veth + network namespace (netem unavailable on this kernel build).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
RAW="$ROOT/evidence/raw"
NS=grad-fault-ns
VETH_H=veth-gf0
VETH_N=veth-gf1
HOST_IP=10.255.200.1
NS_IP=10.255.200.2
PORT=18080

cleanup() {
  sudo ip netns exec "$NS" iptables -D INPUT -p tcp --dport "$PORT" -j REJECT --reject-with tcp-reset 2>/dev/null || true
  sudo ip netns del "$NS" 2>/dev/null || true
  sudo ip link del "$VETH_H" 2>/dev/null || true
}
trap cleanup EXIT
cleanup

if ! sudo tc qdisc add dev lo root netem delay 1ms 2>/dev/null; then
  echo "not-run: tc netem qdisc not available (sch_netem missing)"
  NETEM=0
else
  sudo tc qdisc del dev lo root
  NETEM=1
fi

sudo ip netns add "$NS"
sudo ip link add "$VETH_H" type veth peer name "$VETH_N"
sudo ip link set "$VETH_N" netns "$NS"
sudo ip addr add "${HOST_IP}/24" dev "$VETH_H"
sudo ip link set "$VETH_H" up
sudo ip netns exec "$NS" ip addr add "${NS_IP}/24" dev "$VETH_N"
sudo ip netns exec "$NS" ip link set "$VETH_N" up
sudo ip netns exec "$NS" ip link set lo up

sudo ip netns exec "$NS" python3 -u <<PY &
import socket, time
port = int(${PORT})
srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
srv.bind(('0.0.0.0', port))
srv.listen(5)
while True:
    conn, _ = srv.accept()
    data = conn.recv(65536)
    if data.startswith(b'POST'):
        conn.sendall(b'HTTP/1.1 200 OK\r\nContent-Length: 2\r\nConnection: close\r\n\r\nOK')
    else:
        conn.sendall(b'HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n')
        for i in range(5):
            conn.sendall(f'{i}\r\nchunk{i}\r\n'.encode())
            time.sleep(0.05)
        conn.sendall(b'0\r\n\r\n')
    conn.close()
PY
sleep 0.5

if [[ "$NETEM" == 1 ]]; then
  sudo tc qdisc add dev "$VETH_H" root netem delay 80ms 20ms loss random 3% 25% reorder 25% 50%
  sudo timeout 6 tcpdump -i "$VETH_H" -s 0 -w "$RAW/netem_delay_loss_reorder.pcap" &
  TD1=$!
  sleep 0.2
  curl -sS --max-time 5 "http://${NS_IP}:${PORT}/" -o /dev/null || true
  wait $TD1 2>/dev/null || true
  sudo tc qdisc del dev "$VETH_H" root 2>/dev/null || true

  sudo tc qdisc add dev "$VETH_H" root netem rate 20kbit
  sudo timeout 8 tcpdump -i "$VETH_H" -s 0 -w "$RAW/symptom_slow_upload.pcap" &
  TD2=$!
  sleep 0.2
  dd if=/dev/zero bs=1024 count=64 2>/dev/null | curl -sS --max-time 7 -X POST \
    --data-binary @- "http://${NS_IP}:${PORT}/upload" -o /dev/null || true
  wait $TD2 2>/dev/null || true
  sudo tc qdisc del dev "$VETH_H" root 2>/dev/null || true

  sudo tc qdisc add dev "$VETH_H" root netem delay 400ms 100ms
  sudo timeout 8 tcpdump -i "$VETH_H" -s 0 -w "$RAW/symptom_streaming_stall.pcap" &
  TD3=$!
  sleep 0.2
  curl -sS --max-time 7 "http://${NS_IP}:${PORT}/stream" -o /dev/null || true
  wait $TD3 2>/dev/null || true
  sudo tc qdisc del dev "$VETH_H" root 2>/dev/null || true
else
  echo "skip netem captures (delay/loss/reorder/bandwidth/symptom slow upload & streaming stall)"
fi

echo "=== symptom: TCP reset (iptables REJECT in netns) ==="
sudo ip netns exec "$NS" iptables -I INPUT -p tcp --dport "$PORT" -j REJECT --reject-with tcp-reset
sudo timeout 6 tcpdump -i "$VETH_H" -s 0 -w "$RAW/symptom_tcp_reset.pcap" &
TD4=$!
sleep 0.2
curl -sS --max-time 4 "http://${NS_IP}:${PORT}/" -o /dev/null 2>&1 || true
wait $TD4 2>/dev/null || true

echo "fault netns script complete (netem=${NETEM})"
