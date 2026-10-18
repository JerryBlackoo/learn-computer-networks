# 模块 04：路由迷你仿真

## 学习目标

- **距离向量（DV）**：`JuliShiliang` 一轮邻居交换与 Bellman-Ford 直觉。
- **链路状态（LS）**：`LianluZhuangtai.zuiduan_lujing` 在小图上 Dijkstra。
- 用 `juli_shouling` 观察多轮 DV **收敛**（非 BGP、非策略路由）。

## 核心概念

| 算法 | 本模块实现范围 |
|------|----------------|
| DV | 单轮 `jiaohuan_yilun` + 多轮 `juli_shouling` 直至表稳定 |
| LS | 静态拓扑全图 Dijkstra，无 HELLO/LSA 报文格式 |

无穷代价用 `10_000` 表示；仅适用于节点数很少的课堂图。

## 与上一章综合题的分工

[模块 03 的学校—公司—ISP 大题](../03-ip-zhuanfa/exercises/01-ip-datagram-forwarding.md)给定三张表，要求按目的 IP 选择出接口，这属于**转发**；本模块的 DV/LS 实验说明如何根据拓扑与代价计算路由。

先核对该题的两点：学校与公司之间有 B0—C0 直连路径；公司只需学校的 `/16` 聚合路由，学校内部再按两个 `/17` 细分。不要因为看见 ISP 就擅自改走默认路由，也不要把 ARP 查询误当成选路算法。

## 不变量

- `chushihua` 后自身距离为 0，未知目的地为无穷。
- DV 更新：`dist[v] = min(dist[v], cost(u,v) + dist_u[v])` 仅通过邻居表间接传播。

## 常见失败模式

- **计数到无穷**（本模块图规模小，测试用链状拓扑验证有限轮收敛）。
- 将 LS 与 DV 混淆：LS 需要全局拓扑副本；DV 仅见邻居距离向量。
- 期待 Internet 级 AS 路径属性：本模块**不包含** BGP。

## 作业 API

- `JuliShiliang.chushihua` / `jiaohuan_yilun`
- `LianluZhuangtai.zuiduan_lujing`
- `juli_shouling(jiedian_lie)`：返回收敛所用轮数

## 讨论题

1. DV 为什么在互联网核心被 LS/BGP 体系替代？在小企业内网为何仍可能见到 RIP？
2. 链路故障时 DV 与 LS 的更新消息量有何差异直觉？

## 验证

```bash
pytest modules/04-luyou/tests -q
```

证据标签：`host-verified`。
