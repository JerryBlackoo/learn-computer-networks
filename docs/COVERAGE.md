# 主题覆盖说明

本学习路径按 **12 个模块**组织，对齐本科「计算机网络」常见主线，但**不声称**与任何单一课程实验一一对应，也**不模拟**完整多 AS 互联网或生产级协议栈。

## 模块与主题映射

| 模块 | 主题 | 实现形态 | 自动验证 |
|------|------|----------|----------|
| 01 | 分层与端到端论证 | 封装/解封装教学 API | host-verified |
| 02 | 成帧与不可靠链路上的可靠交付 | 滑动窗口教学栈 `HuachuangTongdao` | host-verified |
| 03 | IP 寻址与转发（仿真） | 最长前缀匹配 `MoNiZhuanfa` | host-verified |
| 04 | 路由（DV / LS 迷你仿真） | `JuliShiliang` / `LianluZhuangtai` | host-verified |
| 05 | TCP 可靠性与拥塞教学模型 | 慢启动/拥塞避免简化 `YongjinMoxing` | host-verified |
| 06 | DNS / HTTP 应用路径 | 本机 stub `BenDiYingyong` | host-verified |
| 07 | 套接字编程实验 | 原创 API `TaoJieLab` | host-verified |
| 08 | NAT / 中间盒直觉 | 地址转换表 `ZhongjianHe` | host-verified |
| 09 | TLS 高层（握手状态机） | 标准库 `ssl`，不自研密码学 | host-verified |
| 10 | 测量与调试 | 合成 pcap 风格 trace `CeLiangJiao` | host-verified |
| 11 | 故障案例馆 | 丢包/乱序/重复/分区注入 | host-verified |
| 12 | 大作业 | `XinyunTongdao` + 简易 HTTP 式应用 | host-verified |

## 明确未覆盖（见 TEACHING_GAPS）

- 真实网卡驱动、内核协议栈修改、硬件 offload
- BGP 政策、大规模 SDN 控制器、数据中心 RDMA 细节
- 无线物理层、蜂窝核心网、QUIC 全栈实现
- 对公网主机的端口扫描、渗透测试框架

## 与「完整协议课实验」的差距

- 传输层为**教学级**状态机，窗口与重传语义简化，非与 Linux TCP 比特级一致。
- 路由仿真为**小拓扑**上的教科书算法，非 FRR/BIRD 级实现。
- TLS 模块验证**握手阶段划分**与证书链加载，不包含自定义密码套件实现。
