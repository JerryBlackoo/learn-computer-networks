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
