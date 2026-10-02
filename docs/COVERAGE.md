# 主题覆盖说明

本学习路径按 **13 个模块**组织，对齐本科「计算机网络」常见主线，但**不声称**与任何单一课程实验一一对应，也**不模拟**完整多 AS 互联网或生产级协议栈。模块 13 仅补**诊断工作流**（见 `docs/ZHENSHI_XIEYI_ZHENDUAN.md`），不将缺口声明整体撤销。

## 模块与主题映射

| 模块 | 主题 | 实现形态 | 自动验证 |
|------|------|----------|----------|
| 01 | 分层与端到端论证 | 封装/解封装教学 API | host-verified |
| 02 | 成帧与可靠交付 | 单向 `HuachuangTongdao` + 双向 ACK `HuachuangKexinTongdao`（窗口/重传/乱序） | host-verified |
| 03 | IP 寻址与转发（仿真） | LPM `MoNiZhuanfa`（默认路由、/32） | host-verified |
| 04 | 路由（DV / LS 迷你仿真） | `JuliShiliang` / `LianluZhuangtai` + `juli_shouling` | host-verified |
| 05 | 传输教学模型 | `YongjinMoxing` + `ChuanShuJiaoxue` + 包级 `BaoJiKexinYanlian` | host-verified |
| 06 | DNS / HTTP 应用路径 | 本机 stub `BenDiYuming` / `JianYiHttp` | host-verified |
| 07 | 套接字编程实验 | 原创 API `TaoJieLab`（127.0.0.1） | host-verified |
| 08 | NAT / 中间盒直觉 | NAPT 表 `ZhongjianHe` | host-verified |
| 09 | TLS 高层（握手状态机） | `TlsZhuangtaiJi` + 标准库 `ssl` fixture | host-verified |
| 10 | 测量与调试 | 合成 trace `CeLiangJiao`（含 retrans 字段） | host-verified |
| 11 | 故障案例馆 | 丢包/乱序/重复/分区 `GuzhangBowuguan` | host-verified |
| 12 | 大作业 | `XinyunTongdao`（`ShuangxiangLianlu` + 02 可靠层）+ `XinxiYingyong` | host-verified |
| 13 | 真实协议诊断 | `ZhenDuanGongju` + 三案例 JSON + 毕业报告模板 | host-verified + not-run（抓包/netem） |

## 共享库（`wangluo_xuejing/`）

| 组件 | 作用 |
|------|------|
| `KexinLianlu` | 单向不可靠内存信道 |
| `ShuangxiangLianlu` | 前向数据 + 回退 ACK 信道对 |
| `huachuang_zhen` | 数据帧 / ACK 帧编解码（教学校验和） |

## 仓库门禁

- `make verify` = `pytest modules tests` + `tools/verify_repo.py`（starter/solution 成对、证据标签、README 体量、docs 无外链）。
- 模块 README 为中文主讲义；官方参考链接仅出现在 `sources/catalog.toml`。

## 明确未覆盖（见 TEACHING_GAPS）

- 真实网卡驱动、内核协议栈修改、硬件 offload
- BGP 政策、SRv6、大规模 SDN 控制器、数据中心 RDMA 细节
- 无线物理层、蜂窝核心网、QUIC 全栈实现
- 对公网主机的端口扫描、渗透测试框架
- 第三方真实 pcap / 未授权目标的流量采集

## 与「完整协议课实验」的差距

- 传输层为**教学级**模型：可靠发送在内存信道上仿真 ACK/RTO/快重传，非 Linux TCP 比特级一致。
- 链路层为 `KexinLianlu` / `ShuangxiangLianlu` 内存模型，**无** ARP on-wire、**无** 真实 NIC。
- 路由仿真为**小拓扑**教科书算法，非 FRR/BIRD 级实现。
- TLS 验证握手阶段划分与自签 fixture 加载，非生产 PKI 运营。
