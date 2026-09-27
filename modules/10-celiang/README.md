# 模块 10：测量与调试

## 学习目标

- 读取 `fixtures/synthetic_trace.json`（**合成事件流**，非第三方真实 pcap）。
- `CeLiangJiao` 估算 SYN–SYN/ACK **RTT**、统计数据包、识别 **retrans** 标记、计算平均载荷大小。

## 核心概念

- **时间线**：每条记录含 `t`、`dir`（c2s/s2c）、`len`、`flags`、可选 `seq`/`retrans`。
- **RTT 估计**：首个客户端 SYN 到服务器 SYN+ACK 的时间差（教学简化，未处理时钟偏移）。
- **重传计数**：`retrans: true` 的事件计入 `chongchuan_bao_shu`，且不计入「首次」数据包统计。

## 不变量

- 仅解析 JSON；不读取外部抓包文件路径。
- 无 SYN 或无 SYN/ACK 时 `rtt_guji` 返回 `None`。

## 常见失败模式

- 将合成 trace 当作真实用户流量做隐私推断。
- 把重传与重复 ACK 混淆（本 trace 用显式字段标注）。

## 明确非目标

- 无 Wireshark 集成、无 TCP 流重组、无跨流关联。

## 作业

完成 `starter/celiang_lab.py`。

## 讨论题

1. 若 SYN 重传发生，`rtt_guji` 应取哪一对报文？本实现选择了哪种简化？
2. 测量结果如何影响模块 05 的 RTO 直觉？

## 验证

```bash
pytest modules/10-celiang/tests -q
```

证据标签：`host-verified`。
