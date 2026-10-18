# 计算机网络学习路径

中文计算机网络学习仓库：以 **C++17 拓扑积木仿真**（`network_simulation/object`）为主线，辅以教材知识库。主题对齐常见本科「计算机网络」课程，但**不是**任何校园实验（含 CS144）的镜像。

> 原 Python 模块作业、真实网络探针、毕业证据包与根目录 `tests/` / `tools/` 已移除。

## 快速开始

```bash
cmake -S network_simulation/object -B network_simulation/object/build
cmake --build network_simulation/object/build
ctest --test-dir network_simulation/object/build --output-on-failure
```

说明与示例见 [network_simulation/object/README.md](network_simulation/object/README.md)（`Network` / 主机 / 二层交换机 / 路由器 / 网卡 / 网关）。

## 文档

| 文件 | 说明 |
|------|------|
| [network_simulation/object/README.md](network_simulation/object/README.md) | 拓扑积木 OOP 与测试 |
| [docs/NETWORK_KNOWLEDGE_BASE.md](docs/NETWORK_KNOWLEDGE_BASE.md) | 五层知识总览 |
| [books/knowledge/FIVE_LAYER_CORE.md](books/knowledge/FIVE_LAYER_CORE.md) | 概念精读（IPv4、子网、CIDR、ARP、ICMP 等） |
| [docs/RESOURCE_POLICY.md](docs/RESOURCE_POLICY.md) | 引用与安全政策 |
| [sources/catalog.toml](sources/catalog.toml) | 官方外链目录 |

`docs/` 中部分历史模块/毕业文档可能尚未与当前目录同步，以本 README 与 `object/` 工程为准。

## 目录怎么用

| 目录 | 用途 |
|------|------|
| [network_simulation/object/](network_simulation/object/) | **主实现**：拓扑积木及 ARP、IPv4、UDP/DNS 依赖；CMake/CTest 验证。 |
| [books/knowledge/](books/knowledge/) | 分章笔记、FAQ、考点映射与阅读记录。 |
| [docs/](docs/) | 知识总览与资源政策。 |
| [sources/](sources/) | RFC / 工具文档外链（`catalog.toml`）。 |
| [books/](books/) | 参考教材与试卷 PDF（Git LFS）。 |

## 拓扑 OOP 一眼用法

```cpp
#include "netsim/topology.hpp"

using namespace netsim;

Network net;
auto& h1 = net.add_host("h1");
auto& sw = net.add_l2_switch("sw", 4);
auto& r  = net.add_router("r1");

h1.add_nic("eth0", /*mac*/, /*ip*/, 24);
r.add_nic("eth0", /*mac*/, /*ip*/, 24);
net.connect(h1.nic("eth0"), sw.port(0));
h1.set_default_gateway(/*gw*/);
net.run(30);
```

## 许可

[MIT LICENSE](LICENSE)

## 用途声明

仅用于防御性、教育性的协议仿真与分析；请勿用于未授权扫描、攻击或恶意软件开发。
