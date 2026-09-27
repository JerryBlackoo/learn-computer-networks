# 模块 11：故障案例馆

## 学习目标

- 通过 `KexinLianlu` 注入 **丢包、乱序、重复、分区**，并用 `GuzhangBowuguan` 给现象命名。
- 将链路层异常与上层协议行为（模块 02/05）建立因果联想。

## 核心概念

| 注入参数 | 预期现象分类 |
|----------|----------------|
| `diubao_gailv=1` | `diubao` |
| `chongpai_gailv=1` | `chongpai`（相邻帧交换） |
| `chongfu_gailv=1` | `chongfu` |
| `fenqu=True` | `fenqu`（发送不入队） |

`zhengti_baogao` 汇总四类结果，便于实验报告对照。

## 不变量

- 分类函数仅依赖教学信道，不向外网发包。
- 随机行为由 `seed` 固定，测试可重复。

## 常见失败模式

- 在分区仍期待交付（应观察 `fenqu`）。
- 忽略重复帧对序号协议的影响（与模块 02 窗口配合思考）。

## 作业

完成 `starter/guzhang_lab.py`。

## 讨论题

1. 真实网络中「乱序」与「重复」常由哪些设备引起？
2. 若同时开启丢包与乱序，接收方日志应如何排序分析？

## 验证

```bash
pytest modules/11-guzhang/tests -q
```

证据标签：`host-verified`。
