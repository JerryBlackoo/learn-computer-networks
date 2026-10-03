#!/usr/bin/env bash
# Real protocol captures for graduation evidence. Run from repo root as user with cap_net_raw.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
RAW="$ROOT/evidence/raw"
IFACE="${CAP_IFACE:-enp0s3}"
mkdir -p "$RAW"

run_cap() {
  local name="$1" filter="$2" duration="$3"
  shift 3
  local out="$RAW/$name"
  sudo timeout "${duration}s" tcpdump -i "$IFACE" -s 0 -w "$out" "$filter" &
  local pid=$!
  sleep 0.3
  "$@"
  wait "$pid" 2>/dev/null || true
  if [[ -f "$out" ]]; then
    echo "wrote $out ($(stat -c%s "$out") bytes)"
  fi
}

echo "=== DNS (UDP/TCP 53) ==="
run_cap dns_query_example_com.pcap 'port 53' 8 \
  dig +time=3 +tries=1 @8.8.8.8 example.com A +stats

echo "=== TCP + TLS + HTTP/1.1 (HTTPS) ==="
run_cap https_example_http11.pcap 'host example.com and (tcp port 443 or udp port 443)' 12 \
  curl -sS --http1.1 -o /dev/null -w 'http_code=%{http_code}\n' https://example.com/

echo "=== HTTP/1.1 cleartext ==="
run_cap http11_example_cleartext.pcap 'host example.com and tcp port 80' 10 \
  curl -sS --http1.1 -o /dev/null -w 'http_code=%{http_code}\n' http://example.com/

echo "=== HTTP/2 ==="
run_cap http2_example.pcap 'host example.com and tcp port 443' 12 \
  curl -sS --http2 -o /dev/null -w 'http_code=%{http_code} http_version=%{http_version}\n' https://example.com/

echo "=== QUIC/HTTP3 ==="
if curl --help 2>/dev/null | grep -q http3; then
  run_cap quic_http3_cloudflare.pcap 'udp port 443' 12 \
    curl -sS --http3-only -o /dev/null https://cloudflare.com/ || true
else
  echo "not-run: libcurl without HTTP/3"
fi

echo "done baseline captures on $IFACE"
