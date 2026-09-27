# 模块 01：分层与端到端论证

## 学习目标

- 理解**封装/解封装**如何在相邻层之间传递 PDU。
- 用「端到端论证」解释：哪些功能必须在通信端点实现，中间节点应保持简单。

## 作业 API（原创）

在 `starter/cengdie_lab.py` 中完成：

- `DuanDaoDuanLunzheng`：记录并查询「应在端点实现的语义」列表。
- `FengzhuangZhan`：按层名自底向上封装，再自顶向下解封装。

## 验证

```bash
pytest modules/01-cengdie/tests -q
```

证据标签：`host-verified`（见 `module.meta.toml`）。
