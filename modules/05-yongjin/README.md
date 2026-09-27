# 模块 05：传输层教学模型（拥塞 + 发送直觉）

## 学习目标

- `YongjinMoxing`：离散时间步上的**慢启动 / 拥塞避免 / 丢包 AIMD**（非 RFC 9293 全状态机）。
- `ChuanShuJiaoxue`：**发送窗口**、在途段数、`RTO ≈ 2×RTT` 简化、**重复 ACK 触发快重传标志**（直觉演示，非完整 TCP）。

## 核心概念

- **cwnd**：拥塞窗口（浮点教学模型）；丢包时 `ssthresh = cwnd/2` 且 `cwnd = 1`。
- **滑动窗口**：`zaiwang` 不超过 `chuangkou`；ACK 前进释放窗口。
- **dup ACK**：对同一 `ack_num` 的重复确认累计；连续 3 次置 `kuai_chongchuan`（不实现真实 SACK/FACK）。

## 明确非目标（诚实）

- 无完整 TCP 状态图、无 SACK、BBR、SYN 握手、无与 `tcpdump` 逐字段对比。
- `chaoshi` 仅比较时间与 `rto`，不模拟重传报文本身。

## 不变量

- 窗口满时 `fasong_bao` 应失败（`RuntimeError`）。
- `gengxin_rtt` 后 `rto` 至少 0.2 秒（教学下限）。

## 常见失败模式

- 将教学 `YongjinMoxing` 与 Linux `tcp_cong` 参数一一对应。
- 在 dup ACK 未达 3 次时就触发快重传（测试会检查计数）。

## 作业

完成 `starter/yongjin_lab.py` 中两个类。

## 讨论题

1. 为何快重传通常配合「不退回 cwnd 到 1」？本模型为何只设置布尔标志？
2. RTO 估计为何需要 Karn 算法排除重传样本？本模块省略了什么？

## 验证

```bash
pytest modules/05-yongjin/tests -q
```

证据标签：`host-verified`。
