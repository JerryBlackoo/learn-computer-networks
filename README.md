# 计算机网络学习路径（wangluo-xuejing）

中文、原创作业 API 的本科计算机网络学习路径，与同家族的 OS / 算法仓库同级：**12 个模块 + starter/solution + `make verify`**。传输层在内存信道上提供双向 ACK、滑动窗口与重传的教学仿真（见 `docs/TEACHING_GAPS.md` 诚实边界）。

> 主题对齐常见「计算机网络」课程主线，但**不是**任何校园实验（含 CS144）的镜像；实验接口与代码均为独立设计。

## 快速开始

```bash
make install-dev
make verify
```

## 文档

| 文件 | 说明 |
|------|------|
| [docs/LEARNING_PATH.md](docs/LEARNING_PATH.md) | 学习顺序与模块索引 |
| [docs/COVERAGE.md](docs/COVERAGE.md) | 主题覆盖与自动验证范围 |
| [docs/TEACHING_GAPS.md](docs/TEACHING_GAPS.md) | 诚实教学缺口 |
| [docs/RESOURCE_POLICY.md](docs/RESOURCE_POLICY.md) | 引用与安全政策 |
| [sources/catalog.toml](sources/catalog.toml) | **唯一**官方外链目录 |

## 模块

`modules/01-cengdie` … `modules/12-capstone` — 每模块含中文 `README.md`、`starter/`、`solution/`、`tests/`、`module.meta.toml`（证据标签 `host-verified`）。

## 许可

[MIT LICENSE](LICENSE)

## 用途声明

仅用于防御性、教育性的协议仿真与分析；请勿用于未授权扫描、攻击或恶意软件开发。
