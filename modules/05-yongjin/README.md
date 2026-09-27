# 模块 05：传输层教学模型（拥塞 + 发送直觉）

## 学习目标

- `YongjinMoxing`：离散时间步上的**慢启动 / 拥塞避免 / 丢包 AIMD**（非 RFC 9293 全状态机）。
- `ChuanShuJiaoxue`：**发送窗口**、在途段数、`RTO ≈ 2×RTT` 简化、**重复 ACK 触发快重传标志**（直觉演示，非完整 TCP）。
- `BaoJiKexinYanlian`：在主机上把上述窗口直觉与模块 02 **`HuachuangKexinTongdao`** 包级仿真联动（含丢包与 `chongchuan_cishu` 观测）。

## 核心概念

- **cwnd**：拥塞窗口（浮点教学模型）；丢包时 `ssthresh = cwnd/2` 且 `cwnd = 1`。
- **滑动窗口**：`zaiwang` 不超过 `chuangkou`；ACK 前进释放窗口。
- **dup ACK**：对同一 `ack_num` 的重复确认累计；连续 3 次置 `kuai_chongchuan`（不实现真实 SACK/FACK）。
- **包级演练**：`chuanshu_bing_jiankong` 返回完整性、重传次数、最终在途段数——均在内存信道，不经公网。

## 明确非目标（诚实）

- 无完整 TCP 状态图、无 SACK、BBR、SYN 握手、无与 `tcpdump` 逐字段对比。
- `BaoJiKexinYanlian` 不替代内核拥塞控制；`YongjinMoxing` 与真实 `tcp_cong` 参数不一一对应。

## 不变量

- 窗口满时 `fasong_bao` 应失败（`RuntimeError`）。
- `gengxin_rtt` 后 `rto` 至少 0.2 秒（教学下限）。
- 包级演练在合理丢包率下应 `shuju_wanzheng` 且 `zuihou_zaiwang == 0`。

## 常见失败模式

- 将教学 `YongjinMoxing` 与 Linux `tcp_cong` 参数一一对应。
- 在 dup ACK 未达 3 次时就触发快重传（单元测试会检查计数）。
- 在 05 中重复实现成帧（应通过 `lab_loader` 引用 02 solution）。

## 作业

完成 `starter/yongjin_lab.py` 中 `YongjinMoxing`、`ChuanShuJiaoxue`、`BaoJiKexinYanlian`。

## 讨论题

1. 为何快重传通常配合「不退回 cwnd 到 1」？本模型为何只设置布尔标志？
2. RTO 估计为何需要 Karn 算法排除重传样本？本模块省略了什么？
3. 包级演练中，提高 `diubao_gailv` 如何影响 `chongchuan_cishu` 与端到端时延（步数）？

## 验证

```bash
pytest modules/05-yongjin/tests -q
```

证据标签：`host-verified`。
