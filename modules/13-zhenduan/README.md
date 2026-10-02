# 模块 13：真实协议诊断（计划第 11 节）

## 定位

本模块**只补诊断工作流与证据结构**，不把本仓库升级为内核级协议栈或公网抓包平台。自动测试使用仓库内**合成案例 JSON**；与 `tcpdump`、Wireshark、netem 相关的步骤标为 **`not-run`**，由你在本机双服务实验上自行完成并写入毕业报告。

先修：完成模块 07（本机套接字）、10（合成 trace 测量）、11（链路故障分类）。建议与模块 09 TLS 讲义对照阅读。

## 学习目标

- 把一次 HTTPS 请求拆成 **DNS → TCP → TLS → HTTP** 分段，并记录各段耗时身份（`QingqiuLianJieduan`）。
- 用模块 10 同形的 trace 事件，生成**教学模型对照表**（`duizhao_jiaoxue_trace`），明确哪些结论来自仿真课、哪些需要真实抓包复核。
- 为三类症状建立 **packet → socket → process → request** 证据链：`shangchuan_bianman`、`liushi_kadun`、`lianjie_chongzhi`。
- 在报告中写明 **已选瓶颈层** 与 **仍无法判断** 的条目（不得把单次 RTT 当作全局结论）。

## 核心 API

| 类型 | 作用 |
|------|------|
| `ZhenDuanGongju` | 症状→候选层、证据链缺口、trace 对照、案例加载 |
| `ZhenDuanBaogao` | 单个合成/真实案例的结构化报告 |
| `QingqiuLianJieduan` | 请求链四段计时占位（not-run 填数） |

`fixtures/` 下三个 JSON 为**教学案例**，不代表真实用户流量。

## not-run：本机实验清单（不入 CI）

1. **实际请求链**：在 `127.0.0.1` 或你控制的两个服务之间完成一次 HTTPS 调用；仅对自己的实验流量抓包，禁止对未授权目标扫描。
2. **TCP 深化**：导出 trace，标出 SYN、重传、窗口相关字段；与模块 05 `ChuanShuJiaoxue` 的 RTO/窗口直觉对照，并写下差异说明。
3. **故障注入**：在隔离环境用 netem/tc 注入 loss、delay、reorder 或带宽上限；区分连接超时、传输重传、应用读超时。
4. **应用协议（阅读）**：连接复用、HTTP/2 流、QUIC 与 HTTP/3 的分层差异——先读 RFC 条目（见 `sources/catalog.toml`），再做窄客户端实验（可选）。
5. **路由分支（可选）**：network namespace 或小型虚拟路由拓扑，理解转发与策略路由入门；不要求 ISP 级 BGP 工程。
6. **毕业报告**：使用 `moban_biyelun()` 输出结构，三个案例各一份证据链；说明瓶颈层与未知项。

## 不变量

- `make verify` 不访问外网、不读取学习者本机 pcap 路径。
- 分类与对照逻辑仅依赖给定 JSON/trace 字段，不假装与 Linux TCP 比特级一致。

## 常见失败模式

- 只有 Wireshark 截图，没有 socket/进程/请求层交叉验证。
- 把模块 11 的内存信道故障直接等同于 Wi‑Fi 或运营商网络，不做层次区分。
- 在重传很少时仍断言「一定是拥塞」，忽略应用背压或对端限速。

## 作业

完成 `starter/zhenduan_lab.py`，使 `pytest modules/13-zhenduan/tests` 通过。

## 讨论题

1. 若 DNS 很快但 TLS 很慢，瓶颈可能在哪些层？还需要哪些证据？
2. 流式卡顿案例中，如何用时间线区分「网络无包」与「应用未消费」？
3. 教学 trace 的 `retrans` 字段与真实抓包判定重传有何简化？

## 验证

```bash
pytest modules/13-zhenduan/tests -q
```

证据标签：`host-verified`（合成案例逻辑）+ `not-run`（本机抓包与 netem 实验）。
