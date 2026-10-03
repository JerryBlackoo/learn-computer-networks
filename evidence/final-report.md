# Final Graduation Report

graduation_status: **NOT_PASS**

## Tested revision

- commit: see `environment.json` `commit_sha` after rebase push
- environment: `evidence/environment.json`

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

- QUIC/HTTP3 not-run.
- netem delay/loss/reorder/bandwidth not-run (no fabricated latency).
- G7 three-symptom full evidence chain incomplete.

## Independent assessment

NOT-RUN — awaiting second operator reproduction per `evidence/commands.md`.

---

# 真实协议毕业诊断报告（证据轮次 2026-10-03）

**结论：`graduation_status = NOT_PASS`** — 已提交真实 pcap，未满足 G6 全量故障注入与 G7 完整证据链。

协议与故障说明见上文 Gate summary 与 `failures.md`；pcap 未经合成或手工伪造。
