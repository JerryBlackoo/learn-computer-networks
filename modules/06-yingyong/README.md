# 模块 06：DNS 与 HTTP 应用路径

## 学习目标

- `BenDiYuming`：内存区文件式 **A 记录** 查询（非递归 DNS、非 BIND）。
- `JianYiHttp`：构造最小 HTTP/1.1 请求行与解析状态码。

## 核心概念

- **应用层端到端**：DNS 解析与 HTTP 事务在本模块中均为进程内数据结构，不经真实 socket（模块 07 才用套接字）。
- **Host 头**：请求中携带虚拟主机名，与 DNS 查询结果组合成「逻辑 URL」。

## 不变量

- 主机名比较大小写不敏感（`lower()`）。
- 状态行解析失败返回 `0`（调用方可视为非法响应）。

## 常见失败模式

- 假设本模块实现了缓存 TTL、CNAME 链或 DNSSEC。
- 将 `JianYiHttp` 当作完整客户端（无 chunk、无 pipeline）。

## 作业

完成 `starter/yingyong_lab.py`。

## 讨论题

1. 递归解析器与权威服务器分工是什么？本模块省略了哪一侧？
2. HTTP/1.1 持久连接如何影响「一次请求一行」的教学模型？

## 验证

```bash
pytest modules/06-yingyong/tests -q
```

证据标签：`host-verified`。
