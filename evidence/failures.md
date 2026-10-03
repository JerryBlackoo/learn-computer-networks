# Failures

Do not delete failed attempts. Record task, input/seed, observed behavior, violated expectation/invariant, root cause, fix or open reason, and regression evidence.

---

## 未通过项与 not-run（`graduation_status = NOT_PASS`）

### 平台能力缺口

| 项目 | 结果 | 说明 |
|------|------|------|
| `tc netem` | not-run | `tc qdisc add ... netem` → `Specified qdisc kind is unknown`；`kmod` 已装但 `/lib/modules/6.12.94+` 无 `sch_netem` |
| `tc tbf`（带宽） | not-run | 同内核 qdisc 集 |
| QUIC / HTTP/3 | not-run | `curl --http3-only`：libcurl 未编译 HTTP/3 |

### G6 / G7

1. **慢上传**：无 `symptom_slow_upload.pcap`（netem/tbf not-run）；未编造时延数字。
2. **流式卡顿**：无 `symptom_streaming_stall.pcap`；同上。
3. **连接重置**：`symptom_tcp_reset.pcap`（re-audit 重抓）帧 5 SYN / 帧 6 RST,ACK；仅传输层 → G7 partial。

### Re-audit 2026-10-03

- 假设成立：DNS/TLS/HTTP/1.1/HTTP/2 基线 pcap 为真实流量；`symptom_tcp_reset` 为 netns+iptables 真实 RST。
- `sch_netem` 仍不可用；未新增 netem 或 QUIC pcap。
- 慢上传 / 流式卡顿：**无 pcap，禁止编造时延或层判定**。

### 边界

- 模块 02/05/12 仿真 TCP **不得**作毕业协议证据。
- `projects/wire_probe` 计时不替代抓包。
