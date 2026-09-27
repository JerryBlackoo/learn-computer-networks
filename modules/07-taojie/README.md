# 模块 07：套接字编程实验

## 学习目标

- 在本机 **127.0.0.1** 上使用 TCP 套接字完成一次性 echo 交互。
- 理解 `bind` / `listen` / `accept` / `connect` 的最小顺序（原创 API 命名 `TaoJieLab`）。

## 核心概念

- **回环地址**：不离开本机协议栈，仍受 OS 权限与防火墙约束；禁止扫描未授权远程主机。
- **短连接**：单次 `accept` 处理一条消息，便于测试确定性。

## 不变量

- 服务与客户端必须使用相同端口；`SO_REUSEADDR` 便于连续运行测试。
- 仅使用 `127.0.0.1`，不硬编码公网地址。

## 常见失败模式

- 端口占用未设置 `SO_REUSEADDR`。
- 阻塞 `recv` 未考虑对端关闭（本实验 payload 很小，一次 `recv` 足够）。

## 作业

完成 `starter/taojie_lab.py` 中 `echo_fuwu` 与 `echo_kehu`。

## 讨论题

1. 若改为 UDP echo，API 应如何变化？可靠性由谁保证？
2. 多线程服务器与本「单 accept」模型相比，状态在哪一层积累？

## 验证

```bash
pytest modules/07-taojie/tests -q
```

证据标签：`host-verified`。
