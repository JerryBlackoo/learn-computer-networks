# 模块 02：成帧与不可靠链路上的交付

## 学习目标

- 在内存信道 `KexinLianlu` 上实现**成帧**：序号、FIN 标志、简单校验和（`huachuang_zhen` 共享格式）。
- 理解**固定接收窗口**内对乱序分片的缓存与按序交付（`HuachuangTongdao`，单向无 ACK）。
- 在 **`ShuangxiangLianlu`** 上实现 **`HuachuangKexinTongdao`**：滑动窗口发送、累积 ACK、超时重传、重复 ACK 触发快重传（教学仿真，非 RFC TCP）。
- 将「坏帧丢弃」与「乱序等待」区分开，为模块 11 故障注入实验打基础。

## 核心概念

- **分片**：长应用报文切成 `duan_max` 大小的块，每块独立成帧。
- **序号空间**：单字节序号（教学简化）；接收方维护 `next` 期望序号。
- **窗口**：仅缓存 `seq ∈ [next, next + chuangkou)` 的乱序帧，窗外丢弃以防内存膨胀。
- **ACK 信道**：`huitui` 承载 `bianma_ack` 帧；发送方维护 `zaiwang` 与 `yiqueren`。
- **重传**：`rto` 步进仿真下最低未确认序号超时重传；`dup_ack_lianxu ≥ 3` 时置 `kuai_chongchuan` 并快重传。

## 不变量

- 校验和错误的帧必须丢弃，不得推进 `next`。
- 仅在收到连续序号且 FIN 到达时返回完整 `bytes`；否则返回 `None`（可多次调用 `jieshou` / `jieshou_wanzheng_kexin`）。
- 可靠模式下 `chuanshu_kexin` 在主机测试中会跑至交付完成或步数上限。

## 常见失败模式

| 现象 | 可能原因 |
|------|----------|
| 永远收不齐（单向） | 丢包且无重传（应使用 `HuachuangKexinTongdao`） |
| 可靠模式步数超限 | 丢包率过高且 `rto` 过大，或 ACK 未正确编码 |
| 乱序卡死 | 未实现窗口内缓存 |
| 静默损坏 | 未校验 checksum |

## 作业 API

- `HuachuangTongdao`：`chuanshu` / `jieshou_wanzheng`（单向，保留用于对比）。
- `HuachuangKexinTongdao`：`chuanshu_kexin` / `jieshou_wanzheng_kexin`（双向 ACK + 重传）。

完成 `starter/huachuang_lab.py`。`make verify` 使用 `solution` 目录验收。

## 讨论题

1. 为何以太网需要帧界？若信道本身按消息边界交付，成帧是否仍必要？
2. 固定窗口与滑动窗口在**发送侧**限制有何不同？本模块两种类各覆盖哪一侧？
3. 累积 ACK 与选择确认（SACK）在乱序场景下有何差异？本路径为何只做累积 ACK？

## 验证

```bash
pytest modules/02-huachuang/tests -q
```

证据标签：`host-verified`。
