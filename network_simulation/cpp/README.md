# C/C++ 五层计算机网络仿真

对应知识点总结见 [`../knowledge/FIVE_LAYER_CORE.md`](../knowledge/FIVE_LAYER_CORE.md)。

## 分层与文件

| 层 | 职责 | 主要文件 |
|----|------|----------|
| 物理层 | 不可靠字节管道、丢包/误码 | `include/netsim/phy.hpp` |
| 数据链路层 | 以太网帧、CRC-32、ARP、GBN 滑动窗口 | `ethernet.hpp` `crc.hpp` `arp.hpp` `sliding_window.hpp` |
| 网络层 | IPv4、Internet 校验和、LPM 路由 | `ipv4.hpp` `checksum.hpp` `route.hpp` |
| 运输层 | UDP/TCP、连接管理、拥塞控制 | `udp.hpp` `tcp.hpp` |
| 应用层 | DNS / HTTP | `dns.hpp` `http.hpp` |
| 主机栈 | 封装/解封装组装 | `host.hpp` |

命名空间：`netsim`。源文件均在 `src/`，与头文件一一对应。

## 构建与测试

```bash
cd network_simulation/cpp
make test
```

端到端用例 `netsim_e2e`：双主机交叉 PhyLink → ARP → DNS(UDP/53) → TCP 三次握手 → HTTP `GET /xinxi`。
