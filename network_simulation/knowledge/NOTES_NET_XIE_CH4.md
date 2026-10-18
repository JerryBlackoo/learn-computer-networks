# 谢希仁第 4 章网络层 · 精读笔记

> 来源：教材 OCR（约 p126–p132）+ 答案 FAQ 4-x + 王道第 4 章 + 期末子网/路由/NAT/ARP。

## 本章重点（教材）

1. 虚拟互连网络；网络层两种服务、两个层面  
2. IP 地址与 MAC 地址的关系  
3. 分类地址与 **CIDR**（重点）  
4. 路由选择协议工作原理  

另含：ICMP、IPv6、多播、VPN/NAT、MPLS/SDN 简介。

## 两种服务之争（OCR）

| | 虚电路 | 数据报（IP） |
|--|--------|--------------|
| 连接 | 先建 VC，释放 | 无连接 |
| 可靠谁负责 | 倾向网络负责 | **端系统负责**（E2E） |
| 地址 | VC 号 | 完整目的 IP |
| 资源 | 预留 | 尽力而为 |

互联网选择数据报：便宜、灵活、适应军事/异构需求（呼应答案问题 1-14）。

## IP 与 MAC / ARP

- IP：网络层逻辑地址（虚拟地址，答案 4-6）  
- MAC：链路层硬件地址  
- ARP：同网段解析 IP→MAC；跨网段交给下一跳路由器 MAC  

期末题：逐跳改 MAC、通常不改 IP（除非 NAT）。

## CIDR 与 LPM

- 前缀匹配；多条匹配时选**最长前缀**（答案习题 `<4-49>` 等）  
- 路由聚合减小表项  

## 路由算法（王道 + 教材）

| 算法 | 思想 | 仿真 |
|------|------|------|
| 距离向量 DV | Bellman-Ford；邻居交换整表；RIP | `DistanceVectorNode` |
| 链路状态 LS | 洪泛链路状态 + Dijkstra；OSPF | `dijkstra_routes` |

静态路由：试卷四 `ip route-static 0.0.0.0 0.0.0.0 next-hop`。

## Traceroute / TTL（OCR ≈p160）

利用 TTL 递减至 0 时路由器发 ICMP 超时，逐步加大 TTL 探测路径；常对同一 TTL 发 3 次测 RTT。路径可能不对称、随拥塞变化。

## 答案要点

- 4-5：不能用一台巨大交换机代替全部路由器（广播域/路由策略/规模）。  
- 4-9：`PING 127.0.0.1` 环回，不发到外部。  
- 4-18：只存下一跳 → 表小、路径可变。  
- NAT：私网↔公网改写（试卷问是否做了 NAT）。

## 仿真映射

- `Ipv4Packet` + `internet_checksum`  
- `RouteTable::longest_prefix_match`  
- `DistanceVectorNode` / `dijkstra_routes`  
- `Arp*` + `Host::send_ip` 待发队列  
