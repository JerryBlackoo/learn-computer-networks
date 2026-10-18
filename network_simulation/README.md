# 计算机网络五层体系结构仿真与核心知识点工程化系统

本项目结合 `books/` 目录下的四部权威资料：
1. **谢希仁《计算机网络》（第8版）教材**（全书 9 章系统性分层理论体系）
2. **《计算机网络（第八版）（谢希仁）答案》**（核心疑难与设计权衡 FAQ 剖析）
3. **《计算机网络期末试卷.pdf》**（期末全题型真题实测：ARP欺骗、跨层全栈协议分析、TCP抓包指标计算、CIDR规划、NAT探测）
4. **2026/2027 王道考研《计算机网络》复习指导**（考纲全量叶子考点、核心算法与精确定量计算）

在 `knowledge/` 下完成了对五层计算机网络核心知识点的全景梳理与跨层关联，并在 `cpp/` 目录下用现代 C++17 完整实现了模拟真实计算机网络的五层协议仿真系统、核心数据结构与端到端测试流水线。

---

## 1. 知识梳理与核心文档索引

所有系统化整理文档均存放于 `knowledge/` 目录：

- **核心全景梳理文档**：[`knowledge/FIVE_LAYER_CORE.md`](knowledge/FIVE_LAYER_CORE.md)
  - 第 1 章：体系结构与核心基石（分组交换、性能指标、五层模型）
  - 第 2 章：物理层核心原理与计算（奈奎斯特极限、香农公式、编码调制、CDMA 正交计算）
  - 第 3 章：数据链路层核心原理与算法（成帧、透明传输、CRC-32 模2除法、CSMA/CD 退避、滑动窗口 GBN/SR、交换机自学习）
  - 第 4 章：网络层核心原理与路由算法（IPv4 报文与分片、CIDR、最长前缀匹配 LPM、ARP 机制与欺骗漏洞、ICMP、NAT、DV/LS 路由算法）
  - 第 5 章：传输层核心机制与协议分析（UDP 伪首部、TCP 11状态机、三次握手/四次挥手、RTT 自适应、RFC 5681 Reno 拥塞控制）
  - 第 6 章：应用层服务与经典协议（DNS 层次解析、HTTP/1.0/1.1/2/3 演进、DHCP 租约、Email）
  - 第 7 章：《计算机网络（第八版）答案》深度辨析与核心疑难精粹（7大核心疑难问题）
  - 第 8 章：《计算机网络期末试卷》综合大题逐题精解与考点穿透（第 1 题至第 7 题全部解答与计算）
  - 第 9 章：王道考研核心算法与定量计算模型归纳（公式对照表与解题关键）
  - 第 10 章：跨层协议全栈协同与端到端访问生命周期模型（封装/解封装图解）
  - 第 11 章：C++ 五层网络仿真模型与核心数据结构实现体系
- **王道考纲全量知识点索引**：[`knowledge/WANGDAO_CHAPTER_INDEX.md`](knowledge/WANGDAO_CHAPTER_INDEX.md)
- **答案 FAQ 174 问蒸馏**：[`knowledge/FAQ_KNOWLEDGE_POINTS.md`](knowledge/FAQ_KNOWLEDGE_POINTS.md)
- **四资料关联图**：[`knowledge/CROSS_BOOK_LINKS.md`](knowledge/CROSS_BOOK_LINKS.md)
- **王道考纲章首 OCR**：[`knowledge/NOTES_WANGDAO_LAYER_OUTLINE.md`](knowledge/NOTES_WANGDAO_LAYER_OUTLINE.md)
- **阅读完成度**：[`knowledge/READING_LOG.md`](knowledge/READING_LOG.md)
- **分章精读笔记**：`knowledge/NOTES_*_XIE_CH*.md`、`NOTES_EXAM.md`
- **OCR 原文归档**：`knowledge/ocr_raw/`

---

## 2. C++17 五层网络仿真实现架构

仿真系统位于 `cpp/` 目录，命名空间为 `netsim`。模块间严格按照计算机网络协议分层设计，完全实现了自上而下数据封装（Encapsulation）与自下而上数据解封装（Decapsulation）：

```
[应用层: Application]     dns.hpp/cpp, http.hpp/cpp         (DNS 查询/响应, HTTP GET/200 OK)
         │
         ▼
[传输层: Transport]       udp.hpp/cpp, tcp.hpp/cpp          (UDP 伪首部, TCP 11状态机, Reno 拥塞控制)
         │
         ▼
[网络层: Network]         ipv4.hpp/cpp, route.hpp/cpp       (IPv4 报文, LPM 路由, ARP 缓存与欺骗模型,
                         routing_algo.hpp/cpp              Distance-Vector & Dijkstra 路由算法)
         │
         ▼
[链路层: Data Link]       ethernet.hpp/cpp, crc.hpp/cpp     (以太网成帧, IEEE 802.3 CRC-32 校验,
                         csma_cd.hpp/cpp, mac_switch.hpp,  CSMA/CD, 交换机自学习, GBN,
                         sliding_window, channel_util      停等/GBN 利用率)
         │
         ▼
[物理层: Physical]        phy.hpp/cpp, capacity.hpp         (PhyLink 信道仿真, 误码/丢包/时延, 奈氏/香农公式)
```

---

## 3. 快速编译与测试运行

### 3.1 编译全部组件
```bash
cd network_simulation/cpp
cmake -B build -S .
cmake --build build
```

### 3.2 运行自动化测试套件
```bash
# 运行 CTest 自动化测试（测试全部 100% 通过）
ctest --test-dir build --output-on-failure -V
```

### 3.3 运行交互式控制台全景演示 Demo
```bash
# 执行 netsim_demo 输出分层协议演练与端到端 HTTP 通信全流程
./build/netsim_demo
```
