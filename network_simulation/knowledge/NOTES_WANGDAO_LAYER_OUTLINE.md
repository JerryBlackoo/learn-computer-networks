# 王道各章考纲/复习提示（章首 OCR）

> 来源：`ocr_raw/wd_ch_p42/62/139/236/279.txt`。与谢希仁章次对照见 `FIVE_LAYER_CORE.md`。

## 第2章 物理层（p42）

考纲：通信基础（奈氏/香农、编码调制）、传输介质、物理层设备（中继器/集线器）。  
复习：极限容量公式与编码是计算题重点。

## 第3章 数据链路层（p62）

考纲：功能；组帧；差错控制；流量控制与可靠传输（停等/GBN/SR）；介质访问（复用、ALOHA、CSMA/CD/CA、令牌）；局域网/VLAN/以太网；PPP；交换机。  
复习重点：滑动窗口、可靠传输、CSMA/CD/CA、以太网帧、争用期与最小帧长、二进制退避、中继器/网卡/集线器/网桥/交换机区别。

## 第4章 网络层（p139）

考纲：功能；IPv4（分组、地址、CIDR、NAT、ARP、DHCP、ICMP）；IPv6；路由算法与协议（DV/LS、RIP/OSPF/BGP）；设备（冲突域/广播域、路由器）。  
与谢希仁一致：CIDR+LPM、ARP、路由为核心。

## 第5章 传输层（p236）

考纲：服务与端口；UDP；TCP（连接、可靠、流量、拥塞）。  
复习：TCP 报文分析、连接管理、流量/拥塞控制——选择+综合高频。  
地位对照：链路层相邻节点逻辑通信；网络层主机间；传输层**进程间**。

## 第6章 应用层（p279）

考纲：C/S 与 P2P；DNS；FTP；电子邮件；HTTP/WWW。  
与期末试卷：DNS+HTTP 访问路径综合。

## 与仿真对照

| 王道重点 | 代码 |
|----------|------|
| 奈氏/香农 | `capacity.hpp` |
| 停等利用率 / GBN | `channel_util.hpp`、`sliding_window.hpp` |
| CSMA/CD、最小帧 | `csma_cd.hpp` |
| 交换机自学习 | `mac_switch.hpp` |
| CRC / 校验和 | `crc.hpp`、`checksum.hpp` |
| ARP / LPM / DV / LS | `arp.hpp`、`route.hpp`、`routing_algo.hpp` |
| TCP 拥塞与连接 | `tcp.hpp` |
| DNS / HTTP | `dns.hpp`、`http.hpp` |
