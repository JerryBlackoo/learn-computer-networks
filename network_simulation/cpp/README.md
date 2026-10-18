# C++17 五层计算机网络仿真模型与核心数据结构

本工程在 `network_simulation/cpp/` 目录下用现代 C++17 完整实现了五层计算机网络仿真体系与核心算法。体系严格对齐谢希仁《计算机网络》（第8版）、参考答案疑难辨析、期末试卷综合大题与王道考研考纲要求。

---

## 1. 协议分层与实现映射

| 网络层次 | 核心机制与算法 | 核心数据结构 / 类 | 源码文件 |
|----------|----------------|-------------------|----------|
| **物理层 (Physical)** | 奈奎斯特极限、香农公式、信道误码率 (BER)、丢包率、单程传播时延 | `capacity.hpp` (`nyquist_bitrate`, `shannon_bitrate`), `PhyLink`, `PhyParams` | `include/netsim/capacity.hpp`<br>`src/phy.cpp` |
| **数据链路层 (Data Link)** | 以太网成帧、IEEE 802.3 CRC-32 模2校验和、CSMA/CD 载波监听与截断二进制指数退避、GBN 滑动窗口与累计确认 | `EthernetFrame`, `CsmaCdMac`, `GbnSender`, `GbnReceiver` | `src/crc.cpp`<br>`src/ethernet.cpp`<br>`src/csma_cd.cpp`<br>`src/sliding_window.cpp` |
| **网络层 (Network)** | RFC 1071 16位反码求和、IPv4 分组首部封装/解析、CIDR 最长前缀匹配 (LPM)、ARP 请求/应答与缓存毒化、Bellman-Ford (DV) 与 Dijkstra (LS) 路由算法 | `Ipv4Packet`, `Ipv4Header`, `RouteTable`, `ArpTable`, `DistanceVectorNode`, `dijkstra_routes` | `src/checksum.cpp`<br>`src/ipv4.cpp`<br>`src/route.cpp`<br>`src/arp.cpp`<br>`src/routing_algo.cpp` |
| **传输层 (Transport)** | UDP 伪首部校验和、TCP 11 状态机、三次握手、四次挥手、字节流序号/确认号追踪、RFC 5681 Reno 拥塞控制 (慢开始/拥塞避免/快重传/快恢复/超时回退) | `UdpDatagram`, `TcpSegment`, `TcpPcb`, `CongestionControl` | `src/udp.cpp`<br>`src/tcp.cpp` |
| **应用层 (Application)** | DNS 域名与 IPv4 A 记录映射解析、HTTP/1.1 请求/响应行与首部解析器及序列化 | `DnsResolver`, `HttpRequest`, `HttpResponse` | `src/dns.cpp`<br>`src/http.cpp` |
| **端到端集成 (End-to-End)** | 双主机跨层协议栈组装 (HTTP $\to$ TCP $\to$ IPv4 $\to$ Ethernet $\to$ PhyLink) | `Host`, `Interface` | `src/host.cpp`<br>`tests/e2e_http.cpp`<br>`src/main_demo.cpp` |
| **SDN 控制/执行平面** | 跨 L2/L3/L4 通配匹配、优先级流表、动作执行、Packet-In 与 Flow-Mod | `FlowTable`, `SdnSwitch`, `SdnController` | `src/flow_table.cpp`<br>`src/sdn_switch.cpp`<br>`src/sdn_controller.cpp` |

---

## 2. 编译与自动化测试

系统提供 CMake 与 Makefile 双重编译支持：

```bash
# 进入工程目录
cd network_simulation/cpp

# 1. 使用 CMake 配置并构建
cmake -B build -S .
cmake --build build

# 2. 运行 CTest 自动化测试套件 (覆盖全层算法与端到端集成，100% 通过)
ctest --test-dir build --output-on-failure -V

# 3. 运行交互式控制台全景演练 Demo
./build/netsim_demo

# 4. 运行 SDN 流表控制演示（也作为 netsim_sdn_e2e 纳入 CTest）
./build/netsim_sdn_demo
```

也可以直接执行 `make test` 快捷构建与运行。

---

## 3. 测试套件构成

1. **单元测试 (`netsim_tests` / `tests/test_all.cpp`)**：
   - `test_capacity`：验证奈奎斯特公式与香农公式在典型信噪比下的理论计算值。
   - `test_phy`：验证 `PhyLink` 随机丢包与位翻转误码注入模型。
   - `test_crc_and_ethernet`：验证以太网帧序列化、CRC-32 生成与单比特翻转检错丢弃。
   - `test_csma_cd`：验证信道载波监听（空闲/忙状态）、最小帧长约束（64字节）与碰撞截断二进制指数退避（BEB）。
   - `test_checksum_ip_udp`：验证 RFC 1071 16 位一补码 Internet Checksum、IPv4 首部与 UDP 伪首部校验和。
   - `test_arp_poisoning_model`：对齐期末试卷第 2 题，验证未经验证更新缓存导致的 ARP 欺骗攻击模型。
   - `test_lpm`：验证 CIDR `/25`、`/24`、`/8`、`/0` 路由表的最长前缀匹配正确性。
   - `test_dv_ls`：验证距离向量 (Bellman-Ford) 分布式收敛与链路状态 (Dijkstra) 最短路径与下一跳计算。
   - `test_gbn`：验证滑动窗口协议（窗口尺寸饱和阻塞、收到 ACK 窗口滑动滑出新帧）。
   - `test_congestion`：验证 TCP Reno 拥塞控制（慢开始指数倍增、拥塞避免加法递增、3 次重复确认快重传与乘法减半快恢复、超时重置为 1）。
   - `test_dns_http`：验证 DNS 域名解析与 HTTP/1.1 报文解析与序列化。
   - `test_tcp_data_and_close`：验证 TCP 完整三次握手、全双工数据交互、四次挥手状态变迁（`FIN_WAIT_1` $\to$ `FIN_WAIT_2` $\to$ `TIME_WAIT` / `CLOSE_WAIT` $\to$ `LAST_ACK` $\to$ `CLOSED`）。

2. **端到端集成测试 (`netsim_e2e` / `tests/e2e_http.cpp`)**：
   - 模拟真实客户端主机与服务端主机通过物理链路 `PhyLink` 互连。
   - 客户端开机执行端到端 HTTP GET 访问流程：
     1. 客户端通过 UDP 53 端口查询 DNS 获取服务器 IP（未命中触发 ARP 解析）；
     2. 客户端向服务器 80 端口发起 TCP 三次握手建立可靠连接；
     3. 客户端在 TCP 字节流上发送 HTTP 请求 `GET /xinxi HTTP/1.0`；
     4. 服务端接收请求、生成并回复 HTTP 200 OK 响应报文及 JSON 实体；
     5. 客户端正确接收并解析 HTTP 响应主体，验证整条端到端链路封装与解封装通畅无误。

3. **SDN 单元测试 (`netsim_sdn_tests` / `tests/test_sdn.cpp`)**：
   - 跨层组合匹配、字段通配、零值精确匹配、优先级与同优先级稳定顺序。
   - 规则新增/替换/删除/清空、命中计数、动作顺序、多端口副本、终止丢弃。
   - MAC/IPv4/TCP/UDP 改写、IPv4 与传输层校验和、IPv4/TCP 选项和填充保留。
   - 短报文、错误首部、分片、非法动作/端口、CRC 错误、控制器缺席和策略拒绝。
   - 首包重查且只发送一次、后续命中不再上报、单控制器管理多交换机。
   - 检查不依赖 `assert`，Release 构建下仍会运行。

4. **SDN 端到端演示/测试 (`netsim_sdn_demo` / `src/main_sdn_demo.cpp`)**：
   - 两个真实 `Host` 经四条单向 `PhyLink` 与 SDN 交换机互连。
   - 空流表启动，ARP 与 TCP 首包触发 Packet-In，由控制器分别下发规则。
   - 客户端连接虚拟 TCP 端口 `8080`；交换机双向改写 `8080 ↔ 80`，完成三次握手与 HTTP `GET /sdn`，HTTP 阶段不再产生 Packet-In。
   - 两个 UDP/9999 报文仅触发一次下发 Drop 规则，并验证均未被转发。

---

## 4. 轻量 SDN API 与边界

这是 C++17、无额外依赖的单线程内存仿真，不连接真实网卡，也不实现 OpenFlow 线协议、异步控制信道、多级流表或超时老化。

- `FlowMatch`：入端口、源/目的 MAC、EtherType、源/目的 IPv4、IP 协议号、源/目的 TCP/UDP 端口。`std::nullopt` 表示整字段通配；设置字段精确匹配，所有条件取 AND；不提供位掩码/CIDR 匹配。L3 要求有效 IPv4 首部，L4 要求非分片且结构有效的 TCP/UDP 首部。
- `FlowTable`：选择最高 `priority`；同优先级按首次插入顺序。`id` 在每台交换机内唯一，同 ID 下发即替换规则并重置 `packet_count`。每次选中规则计一次命中（含 Drop 和动作错误）。
- `Forward{port}`：保存执行到此时的帧副本，允许多个输出与显式发回入端口；`Modify` 按顺序改写现有字段；`Drop{}` 终止并取消本规则的全部输出。空动作或只有 Modify 的规则也丢弃，不会重新查表。
- `Modify`：支持 MAC、IPv4 地址及 TCP/UDP 端口；改写 L3/L4 后重算 IPv4 和 TCP/UDP 校验和（包括 UDP 原本未启用校验和的情况）。IPv4/TCP 选项及填充不变；发送时重新生成以太网 FCS。L3/L4 改写遇到非 IPv4、分片或相关首部结构错误时整包丢弃；MAC 改写不受此限制。
- `SdnSwitch`：端口从 `0` 开始，不自学习、不隐式泛洪。Table-Miss 同步调用一次处理器；返回后对暂存首包重查一次，仍未命中则丢弃。没有处理器、输出未连接或非法入端口也丢弃；非法输出端口的 Flow-Mod 会被拒绝，原规则不变。`stats()` 暴露接收、未命中、Packet-In、输出副本与丢弃计数；输出计数代表提交至物理链路，不保证链路最终送达。
- `SdnController`：策略返回 `std::optional<FlowEntry>`，接收 Packet-In 并向来源交换机下发规则；返回 `nullopt` 表示不安装。也可用 `send_flow_mod` 主动新增/替换、删除或清空。控制器不持有交换机指针，可服务多台交换机。

最小连接示例（`egress` 和回调捕获的 `controller` 须在使用期间保持存活）：

```cpp
#include "netsim/sdn_controller.hpp"

using namespace netsim;

PhyLink egress;
SdnController controller([](const PacketIn& packet) -> std::optional<FlowEntry> {
  FlowEntry entry;
  entry.id = 1;
  entry.priority = 100;
  entry.match.ingress_port = packet.ingress_port;
  entry.match.dst_mac = packet.frame.dst;  // 其余字段通配
  entry.actions = {Forward{1}};
  return entry;
});
SdnSwitch sw(1, 2);
sw.set_port_tx(1, &egress);
sw.set_packet_in_handler([&](SdnSwitch& source, const PacketIn& packet) {
  controller.on_packet_in(source, packet);
});
// sw.on_frame(0, frame.serialize());
// 第一帧：Packet-In → Flow-Mod → 重查并转发；同类后续帧：直接命中流表。
```

只运行 SDN 测试：`ctest --test-dir build -R sdn --output-on-failure`。
