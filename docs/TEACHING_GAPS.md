# 教学缺口（诚实声明）

本文件记录**刻意不做**或**仅点到为止**的内容，避免学习者误以为本仓库等价于完整校园实验或真实运营商网络。

## 传输与链路

- **没有**与真实以太网 NIC 交互的驱动实验；链路层为内存信道 `KexinLianlu` / `ShuangxiangLianlu` 或合成故障参数。
- **没有** ARP 报文级别的 on-wire 实现；L3 转发使用仿真转发表 `MoNiZhuanfa`（若需 ARP 直觉，仅在内存表中仿真邻居解析，不经真实接口）。
- 模块 02 在单向 `HuachuangTongdao` 之外提供 **`HuachuangKexinTongdao`**：双向 ACK 信道上的滑动窗口、超时重传、重复 ACK 快重传标志——仍为**教学仿真**，非内核 TCP 或 RFC 9293 全状态机。
- 模块 05 `ChuanShuJiaoxue` 与 `BaoJiKexinYanlian` 将窗口直觉与 02 的包级仿真联动，**不**声称与 `tcpdump` 逐字段一致。

## 网络层与路由

- 转发表规模为小样本；**无** FIB 压缩、ECMP、SRv6 等生产特性。
- 距离向量与链路状态仅为 **O(n²) 小图** 收敛演示（`juli_shouling`），**无** BGP 路径属性、策略路由、或 Internet 拓扑数据集。

## 传输层（TCP）

- `YongjinMoxing`、`ChuanShuJiaoxue`、`HuachuangKexinTongdao` 均为**教学模型**，非 RFC 9293 全状态覆盖；未实现 SACK、FACK、BBR、真实 SYN 握手、拥塞控制与重传的完整耦合。
- **无** 与 `tcpdump` 抓到的真实握手逐字段对比的硬性 autograder。
- RTO 更新为固定步长或 `RTO ≈ 2×RTT` 简化，**无** Karn/Jacobson 完整细节。

## 应用层

- DNS 为**内存区文件 + 极简解析**，非 BIND/递归解析器。
- HTTP 为**单行请求/响应子集**，非完整 Apache/nginx 行为。

## 安全

- TLS 使用 Python `ssl` 与自签 fixture；**不**教授密码学证明、侧信道或漏洞利用。
- **禁止**将本仓库工具用于未授权目标；缺口包括「真实 PKI 运营」「HSM 部署」「对第三方站点的中间人测试」。

## 测量

- pcap 风格数据为**合成 JSON/trace**（见模块 10 `fixtures/synthetic_trace.json`），非来自第三方真实流量捕获包（避免许可与隐私问题）。

## 真实协议诊断（计划第 11 节）仍存在的缺口

模块 **13-zhenduan** 与 `docs/ZHENSHI_XIEYI_ZHENDUAN.md` 提供：

- 请求链分段、教学 trace 与模块 02/05/10 的对照表结构；
- 「上传变慢 / 流式卡顿 / 连接重置」三案例的**合成**证据链与毕业报告模板；
- `not-run` 清单：本机双服务抓包、netem/tc 故障注入、可选 namespace 路由实验。

**仍未纳入本仓库**（须在你自己的隔离环境中完成，且不得对未授权目标抓包）：

- 与 `tcpdump` 逐字段一致的自动评分；
- HTTP/2、QUIC/HTTP/3 的完整实现或生产级代理行为复现；
- BGP 策略、运营商级拓扑与第三方真实 pcap 数据集。

完成模块 13 的 `host-verified` 部分只证明你能**结构化诊断与对照教学模型**，不证明已完成真实网络环境下的毕业抓包。

## 若你需要补齐

建议在完成本路径与模块 13 not-run 清单后，自行选修：操作系统网络栈阅读、Wireshark 用户文档与 netem 说明（经 `catalog.toml` 的 `tools` 段登记）、以及学校认可的实验课——并遵守其学术诚信政策。
