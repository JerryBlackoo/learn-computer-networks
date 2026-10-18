# 计算机网络学习路径

中文、原创作业 API 的本科计算机网络学习路径，与同家族的 OS / 算法仓库同级：**13 个模块 + starter/solution + `make verify`**。传输层在内存信道上提供双向 ACK、滑动窗口与重传的教学仿真（见 `docs/TEACHING_GAPS.md` 诚实边界）。**模块 13** 对齐知识覆盖计划第 11 节「真实协议诊断」，仅补证据链与报告结构，见 `docs/ZHENSHI_XIEYI_ZHENDUAN.md`。

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
| [docs/ZHENSHI_XIEYI_ZHENDUAN.md](docs/ZHENSHI_XIEYI_ZHENDUAN.md) | 真实协议诊断路线（计划第 11 节） |
| [docs/RESOURCE_POLICY.md](docs/RESOURCE_POLICY.md) | 引用与安全政策 |
| [sources/catalog.toml](sources/catalog.toml) | **唯一**官方外链目录 |

## 九个目录怎么用

路径均相对于仓库根目录。学习主线是先读 `docs/`，按顺序完成 `modules/01-cengdie` 至 `modules/13-zhenduan`，再结合真实网络实验与 `evidence/` 做诊断；其余目录在需要时查阅。

| 目录 | 具体使用方式 |
|------|--------------|
| [docs/](docs/) | 入门先读 [学习路线](docs/LEARNING_PATH.md)、[覆盖范围](docs/COVERAGE.md)和[教学缺口](docs/TEACHING_GAPS.md)；做模块 13 时读[真实协议诊断指南](docs/ZHENSHI_XIEYI_ZHENDUAN.md)。 |
| [modules/](modules/) | 按编号 01→13 学习。每个模块读讲义、完成 `starter/` 作业，再参考 `solution/`；模块 03 还可以完成 `exercises/` 中的推导题。具体步骤见下文。 |
| [network_simulation/](network_simulation/) | 给部分模块提供内存信道、教学帧和公共工具。遇到代码中的 `from network_simulation...`，再阅读对应文件（如 `kexin_lianlu.py`）；模块 03、04 的查表与路由算法不需要直接使用虚拟信道。这里不是完整的 IP 协议栈。 |
| [real_network_practice/](real_network_practice/) | 学完套接字和 TLS 的基本概念后，按 [wire_probe 的说明](real_network_practice/wire_probe/README.md)连接自己控制的 HTTP/HTTPS 服务，观察 DNS、TCP、TLS 与首字节耗时。它使用系统真实网络，不调用教学仿真信道。 |
| [evidence/](evidence/) | 做模块 13 和毕业验收时，先读 [GRADUATION.md](GRADUATION.md) 与 `evidence/final-report.md`、`evidence/failures.md`；按 `commands.md` 和 `scripts/` 在可控环境复现，把真实抓包保存到 `raw/`，核对 `results.json` 中的 G0–G7 状态。仿真数据不能代替抓包证据。 |
| [sources/](sources/) | 学到 IP、TCP、DNS、TLS 等协议而需要查 RFC 或工具文档时，到 [catalog.toml](sources/catalog.toml) 找官方入口；无需提前通读。 |
| [books/](books/) | 在本机存放个人有权使用的 PDF 作为补充阅读；文件被 `.gitignore` 排除，不随 Git 提交或推送。见 [本地资料说明](books/README.md)。 |
| [tests/](tests/) | 根目录的全局测试支撑：`solution_loader.py` 负责加载模块参考实现，`test_evidence_tags.py` 检查元数据标签。各模块自己的用例在 `modules/NN-*/tests/`，从仓库根目录执行 `make verify` 会运行两者。 |
| [tools/](tools/) | 平时在根目录运行 `make verify`，它会调用 `verify_repo.py` 检查目录、文档与元数据；整理毕业材料时可另运行 `python3 tools/verify_graduation_evidence.py` 检查证据包结构。结构检查通过不等于毕业状态为 PASS。 |

## 每个模块怎样做

1. 读该模块的 `README.md` 和 `module.meta.toml`，明确目标与验证范围；有 `exercises/` 时按题目提示练习。
2. 看该模块 `tests/` 中的用例，自己在 `starter/` 编写实现；遇到共享类型或信道再查 `network_simulation/`。
3. 自行验证写在 `starter/` 的代码，最后再用 `solution/` 对照思路。**当前模块测试通过 `tests/solution_loader.py` 加载的是 `solution/`，直接运行 pytest 或 `make verify` 不代表你的 `starter/` 已通过测试。**若要自动验收自己的实现，需另外让测试加载 `starter/`。
4. 阶段结束后在仓库根目录运行 `make verify`，检查仓库现有参考实现、真实网络小项目和目录结构是否正常。

真实网络实验可以在学完模块 07 的套接字与模块 09 的 TLS 后开始。最简单的本机 HTTP 练习需要两个终端。终端 A 先切换到一个空的临时目录，再启动本机服务：

```bash
python3 -m http.server 8000 --bind 127.0.0.1
```

终端 B 在仓库根目录运行探针：

```bash
python3 real_network_practice/wire_probe/probe.py http://127.0.0.1:8000/
```

## 许可

[MIT LICENSE](LICENSE)

## 用途声明

仅用于防御性、教育性的协议仿真与分析；请勿用于未授权扫描、攻击或恶意软件开发。
