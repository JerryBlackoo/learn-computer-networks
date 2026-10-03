# GRADUATION — 计算机网络学习路径（毕业闸门）

**仓库状态：`NOT_CLOSED`**（证据未闭合）  
**本轮证据包：`graduation_status = NOT_PASS`**（见 `evidence/results.json`）— **禁止**在未满足下列门槛时改为 PASS。

## Frozen scope（冻结范围）

以**真实协议抓包 + 可控环境故障注入**毕业；模块 02/05/12 等**教学仿真 TCP/可靠通道** alone **不能**作为毕业宣告依据。

覆盖协议：DNS、TCP、TLS、HTTP/1.1、HTTP/2；QUIC/HTTP3 在环境不支持时标 **not-run**。

## 最终考核

1. 对自己控制的流量完成可复现抓包（pcap 存于 `evidence/raw/`）。  
2. 在 network namespace + **netem（或等价）** 下注入 delay、loss、reorder、带宽限制、reset，并用抓包说明「慢上传 / 流式卡顿 / 连接重置」更可能落在哪一层。  
3. 每个诊断必须给出**可引用的 trace 事件**（帧/标志/应用日志），不得空泛断言。

## 目标平台闸门（G5）

声称已验证的协议须有**真实 socket + 抓包**；不可用则 **not-run**，不得伪造包或时延数字。

## PASS 规则

- 全部 G0–G7 为 `pass` 或合规 `not-run`（且 not-run 不用于必修症状逃避）。  
- G7 三症状均需完整四层证据链。  
- 另一位操作者可按 `evidence/commands.md` 复现至少一条时间线。

---

## G0–G7 检查单（与 `evidence/results.json` 同步）

| 闸门 | 内容 | 本轮 |
|------|------|------|
| **G0** | 环境、工具版本、网卡/权限 | `evidence/environment.json` |
| **G1** | DNS 真实抓包 | `raw/dns_query_example_com.pcap` |
| **G2** | TCP 连接可辨认 | 含于 HTTPS/HTTP pcap |
| **G3** | TLS 握手可辨认 | `raw/https_example_http11.pcap` |
| **G4** | HTTP/1.1 | 明文 + TLS 隧道各一 pcap |
| **G5** | HTTP/2；QUIC/HTTP3 | HTTP/2 有 pcap；QUIC **not-run** |
| **G6** | netns + 故障注入 | netns **run**；netem **not-run**；TCP reset **run** |
| **G7** | 三症状诊断 | 重置 **partial**；慢上传/卡顿 **not-run** |

## 证据包布局

```
GRADUATION.md
evidence/
  environment.json
  commands.md
  results.json      # graduation_status 必须为 NOT_PASS 直至人工闭合
  failures.md
  final-report.md
  raw/*.pcap
  scripts/          # 复现脚本（非伪造器）
```

## 阅读顺序

1. `evidence/final-report.md` — 本轮结论与层判定  
2. `evidence/failures.md` — not-run 与缺口  
3. `docs/ZHENSHI_XIEYI_ZHENDUAN.md` — 与模块 13 诊断课对齐（不新增模块章节）
