# 模块 12：大作业（信运通道 + 简易应用）

## 学习目标

- 组合模块 02 的 `HuachuangTongdao`（经 `XinyunTongdao`）与简易 HTTP 式 `XinxiYingyong`。
- 完成一次 **请求–响应** 往返：`GET /xinxi` 返回 JSON；未知路径返回 404。

## 核心概念

- **通道复用**：大作业不重新实现成帧，而是通过 `lab_loader` 引用 02 的 solution 能力（与学习者完成的 02 作业概念对齐）。
- **应用语义**：解析请求首行，构造 `HTTP/1.1` 状态行与 JSON 体。

## 不变量

- 交互基于 `KexinLianlu` 内存信道，不经公网。
- 未收到请求时 `yici_jiaohu` 应失败（`RuntimeError`）。

## 常见失败模式

- 在 capstone 中复制粘贴成帧代码导致与 02 分歧（应走 `XinyunTongdao` 封装）。
- 忽略 `\r\n\r\n` 头体分隔，导致 JSON 解析失败。

## 与路径其他模块的关系

| 模块 | 在大作业中的角色 |
|------|------------------|
| 02 | 成帧与重组 |
| 06 | HTTP 行格式直觉 |
| 11 | 若链路注入故障，交互可能失败（可自行扩展实验） |

## 作业

完成 `starter/capstone_lab.py`。

## 讨论题

1. 若要在 capstone 中加入 NAT（模块 08），映射应插在通道哪一侧？
2. 生产微服务中，等价的一次 JSON RPC 需要哪些本路径未覆盖的组件？

## 验证

```bash
pytest modules/12-capstone/tests -q
make verify
```

证据标签：`host-verified`。
