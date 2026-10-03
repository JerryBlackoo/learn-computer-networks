# Final Graduation Report

graduation_status: **NOT_PASS**

## Tested revision

- commit: `6510ad67466ba446e91dca0b7a80cce81589cf66` (main) + re-audit commit on branch
- environment: `evidence/environment.json`
- re-audit log: `evidence/raw/reaudit_20261003_tshark.log`

## Gate summary

| Gate | Status | Primary evidence |
|------|--------|------------------|
| G0 | pass | environment + GRADUATION.md |
| G1–G4 | pass | DNS/TCP/TLS/HTTP/1.1 pcaps in `raw/` |
| G5 | not-run | HTTP/2 pcap present; QUIC/HTTP3 not-run |
| G6 | not-run | TCP reset pcap; netem not-run |
| G7 | not-run | reset partial; slow upload / streaming stall not-run |

## Claims supported

- Real pcaps for DNS, TCP, TLS, HTTP/1.1, HTTP/2 (see `results.json` inventory).
- TCP RST observable in `symptom_tcp_reset.pcap` (`tcp.flags.reset==1`).

## Claims not supported / limitations

- QUIC/HTTP3 not-run (`curl --http3-only` unsupported in this libcurl build).
- netem delay/loss/reorder/bandwidth not-run (`sch_netem` absent on kernel `6.12.94+`; no fabricated latency).
- G7 three-symptom full evidence chain incomplete (no slow-upload or streaming-stall pcaps).

## Independent assessment (re-audit 2026-10-03, pcap-only)

Second operator pass on the cloud VM: re-ran `run_fault_netns.sh`, refreshed `symptom_tcp_reset.pcap`, and verified all archived pcaps with `tshark`. Simulated TCP in modules 02/05/12 was **not** used as graduation protocol evidence.

### Why is an upload slow?

**Not answerable from archived pcaps.** There is no `symptom_slow_upload.pcap` (netem `rate` / tbf path was skipped). No RTT, throughput, or retransmission timeline may be invented.

### Why does a stream stall?

**Not answerable from archived pcaps.** There is no `symptom_streaming_stall.pcap`. Chunked HTTP in the fault script was never exercised under netem delay in this environment.

### Why does the connection reset?

**Transport layer (TCP), evidence in `symptom_tcp_reset.pcap`:**

| Frame | Observation |
|-------|-------------|
| 5 | Client `10.255.200.1` sends **SYN** to `10.255.200.2:18080` on the isolated veth pair. |
| 6 | Peer `10.255.200.2` returns **RST, ACK** (`Win=0`, no payload). |

There is no TLS or HTTP in this trace. The reset is issued by the destination stack/firewall policy (consistent with `iptables REJECT --reject-with tcp-reset` in `run_fault_netns.sh`), not an application-level error response. G7 remains **partial** because DNS/TLS/HTTP layers are absent on this symptom path.

---

# 真实协议毕业诊断报告（证据轮次 2026-10-03）

**结论：`graduation_status = NOT_PASS`** — 真实 pcap 已复核；G6 netem 全量与 G7 三症状完整链仍未闭合。

协议与故障说明见上文 Gate summary 与 `failures.md`；pcap 未经合成或手工伪造。
