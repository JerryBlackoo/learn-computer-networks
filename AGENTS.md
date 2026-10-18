# 仓库协作指南

本文件适用于整个仓库，帮助维护者与编码代理理解项目、定位资料、实施修改和选择验证方式。下文路径均相对于仓库根目录；具体协议行为以当前源码与测试为准，学习范围以覆盖说明与教学缺口为准，不将历史报告中的结果当作本次执行结果。

## 1. 优先遵守的约定

- 默认使用简体中文沟通，讲义、说明和新增教学注释优先使用中文；保留既有 API、文件名、协议字段和命令的原始拼写。
- 明确区分 Python 教学模块、C++ 内存仿真、真实 socket 实验和真实抓包证据，不能把其中一种的通过结果推广到另一种。
- 修改前检查 `git status --short`，保留用户已有的未提交修改；只处理当前任务涉及的文件，不顺手重构、重排或更新无关材料。
- `starter/` 中的未实现部分通常是教学设计，不是待统一修复的缺陷；除非任务明确要求，不用参考答案覆盖练习骨架。
- 不伪造测试结果、抓包、时间线或毕业状态，不为了通过检查而删除失败记录、跳过关键用例或放宽门禁。

<!-- CODEGRAPH_START -->
## CodeGraph

In repositories indexed by CodeGraph (a `.codegraph/` directory exists at the repo root), reach for it BEFORE grep/find or reading files when you need to understand or locate code:

- **MCP tool** (when available): `codegraph_explore` answers most code questions in one call — the relevant symbols' verbatim source plus the call paths between them, including dynamic-dispatch hops grep can't follow. Name a file or symbol in the query to read its current line-numbered source. If it's listed but deferred, load it by name via tool search.
- **Shell** (always works): `codegraph explore "<symbol names or question>"` prints the same output.

If there is no `.codegraph/` directory, skip CodeGraph entirely — indexing is the user's decision.
<!-- CODEGRAPH_END -->

没有 CodeGraph 索引时，优先使用 `rg --files` 定位文件、`rg -n` 检索符号，再按需阅读源码；不要为普通维护任务自行创建索引。搜索时排除构建产物、缓存和非目标大型资料，避免把 PDF 或整份 OCR 原文当作代码入口。

## 2. 项目定位与技术栈

- 项目名称：计算机网络学习路径；Python 项目元数据中的名称为 `wangluo-xuejing`，配置位于 `pyproject.toml`。
- 面向已具备基础编程能力的学习者，以中文讲义、原创作业 API、`starter/solution` 对照和自动测试走通本科计算机网络主线。
- 学习主线为 13 个递进模块，另有教材知识库、C++17 五层仿真、真实网络探针和毕业证据包。
- Python 要求 **3.11 或更新版本**；运行代码主要使用标准库，开发依赖为 `pytest>=8.0`，见 `requirements-dev.txt`。
- C++ 工程使用 **C++17、CMake 3.16+ 和 CTest**，命名空间为 `netsim`，不依赖第三方 C++ 测试框架。它是独立工程，不是 Python 模块的原生扩展。
- 根目录使用 GNU Make 聚合 Python 测试与仓库检查；当前没有单独配置的 lint、格式化或类型检查门禁，不应声称这些检查已运行。
- 本仓库不是 CS144 或其他课程实验的镜像，也不是生产级网络栈、内核网络实验或完整互联网模拟器。项目代码许可见 `LICENSE`；教材、答案和试卷不因此自动获得再分发授权。

## 3. 目录导航与权威入口

| 路径 | 内容与使用时机 |
|------|----------------|
| `README.md` | 项目总览、快速开始、九个顶层业务目录的使用方法和学习者注意事项。 |
| `docs/` | 学习路径、覆盖范围、教学缺口、资源政策、诊断路线及补充知识文档。 |
| `modules/` | 01–13 教学模块；通常含讲义、元数据、练习骨架、参考实现与测试，部分另含 fixture 或综合题。 |
| `network_simulation/*.py` | Python 模块共用的内存信道、成帧、校验工具与动态加载器。 |
| `network_simulation/cpp/` | C++17 五层网络仿真、交换与路由算法、轻量 SDN、端到端测试和演示。 |
| `network_simulation/knowledge/` | 四类参考资料的分章笔记、FAQ、题解摘要、知识点映射、阅读记录与 OCR 归档。 |
| `real_network_practice/wire_probe/` | 使用系统真实 socket 的 HTTP/HTTPS 计时探针及 loopback 自动测试。 |
| `evidence/` | 毕业证据包：环境、实际执行命令、闸门状态、失败记录、最终报告、原始 pcap 与复现脚本。 |
| `sources/catalog.toml` | 官方规范、RFC、工具文档和参考课程的集中外链目录。 |
| `books/` | 本地参考教材、答案、试卷和王道复习资料 PDF；通过 Git LFS 管理。 |
| `tests/` | 模块参考实现加载辅助工具与证据标签全局测试。 |
| `tools/` | `verify_repo.py` 仓库完整性检查与 `verify_graduation_evidence.py` 毕业证据结构检查。 |
| `GRADUATION.md` | 冻结的真实协议毕业范围、G0–G7 闸门和证据交付要求。 |

按任务选择文档，不必每次通读知识库：

- 学习顺序与先修关系：[docs/LEARNING_PATH.md](docs/LEARNING_PATH.md)。
- 已覆盖主题、实现形态与验证范围：[docs/COVERAGE.md](docs/COVERAGE.md)。
- 教学简化和未实现部分：[docs/TEACHING_GAPS.md](docs/TEACHING_GAPS.md)。
- 原创、引用、外链与安全政策：[docs/RESOURCE_POLICY.md](docs/RESOURCE_POLICY.md)。
- 真实协议诊断、请求链和三症状项目：[docs/ZHENSHI_XIEYI_ZHENDUAN.md](docs/ZHENSHI_XIEYI_ZHENDUAN.md)。
- 五层理论总览：[docs/NETWORK_KNOWLEDGE_BASE.md](docs/NETWORK_KNOWLEDGE_BASE.md) 与 [network_simulation/knowledge/FIVE_LAYER_CORE.md](network_simulation/knowledge/FIVE_LAYER_CORE.md)。
- C++ 架构与 SDN API：[network_simulation/cpp/README.md](network_simulation/cpp/README.md)；真实探针边界见 [wire_probe/README.md](real_network_practice/wire_probe/README.md) 与 [FRAMING_LAB.md](real_network_practice/wire_probe/FRAMING_LAB.md)。

## 4. 十三个模块索引

| 模块目录 | 主题 | 关键 API / 交付 |
|----------|------|-----------------|
| `01-cengdie` | 分层、封装与端到端论证 | `DuanDaoDuanLunzheng`、`FengzhuangZhan`。 |
| `02-huachuang` | 成帧、滑动窗口、ACK 与重传 | 单向 `HuachuangTongdao` 与双向可靠 `HuachuangKexinTongdao`。 |
| `03-ip-zhuanfa` | IP 寻址与最长前缀匹配 | `MoNiZhuanfa`；`exercises/` 中有逐跳转发与 ARP 两道纸面综合题。 |
| `04-luyou` | 小拓扑距离向量与链路状态 | `JuliShiliang`、`LianluZhuangtai`、`juli_shouling`。 |
| `05-yongjin` | 拥塞、窗口与包级可靠传输教学模型 | `YongjinMoxing`、`ChuanShuJiaoxue`、`BaoJiKexinYanlian`。 |
| `06-yingyong` | 内存 DNS 与极简 HTTP | `BenDiYuming`、`JianYiHttp`，不经真实 socket。 |
| `07-taojie` | 本机 TCP echo 与套接字顺序 | `TaoJieLab`，仅使用回环地址。 |
| `08-nat` | NAT / NAPT 映射直觉 | `ZhongjianHe`，出向建表、入向反查。 |
| `09-tls` | TLS 高层握手阶段与证书 fixture | `TlsZhuangtaiJi`，标准库 `ssl`，不自研密码学。 |
| `10-celiang` | 合成 trace 测量与调试 | `CeLiangJiao`，RTT、载荷与显式重传标记。 |
| `11-guzhang` | 丢包、乱序、重复与分区 | `GuzhangBowuguan`，固定 seed 的内存故障实验。 |
| `12-capstone` | 可靠通道上的类 HTTP 请求与响应 | `XinyunTongdao`、`XinxiYingyong`，`GET /xinxi` 返回 JSON。 |
| `13-zhenduan` | 真实协议诊断方法与三案例报告结构 | `ZhenDuanGongju`、`ZhenDuanBaogao`、`QingqiuLianJieduan`；自动测试使用合成 JSON。 |

以上目录均位于 `modules/`。模块 13 所称“计划第 11 节”是外部知识覆盖计划中的编号，**不是**仓库的 `11-guzhang` 模块，不要据此合并或重编号。

每个模块维护 `README.md`、`module.meta.toml`、`starter/*.py`、`solution/*.py` 和 `tests/test_*.py`。改动作业接口时同时检查骨架、参考实现、测试和讲义；不要只改其中一处。

## 5. 代码架构与复用关系

### Python 教学仿真

- `kexin_lianlu.py`：`LianluCanshu` 控制丢包、乱序、重复、分区与随机种子；`KexinLianlu` 虽名含“可信”，实际上是**单向不可靠内存信道**。
- `shuangxiang_lianlu.py`：`ShuangxiangLianlu` 将前向数据与回传 ACK 分开，可独立注入故障。
- `huachuang_zhen.py`：数据帧、ACK 帧编解码；使用单字节序号与教学 XOR 校验，不是以太网 FCS 或 TCP 首部。
- `xinxi.py`：共享载荷结构、十六进制辅助与 `xor_checksum`；不可把 XOR 校验描述为 CRC-32。
- `lab_loader.py`：通过文件路径加载模块的 `solution/*.py`，并注册 `sys.modules` 以兼容 dataclass。
- 模块 05 的包级演练和模块 12 的可靠通道复用模块 02 的参考实现。修改 02 或共享帧格式时，至少回归 02、05、11、12；必要时运行完整根目录验证，不复制一套平行可靠层。

Python 教学路径的 ACK、RTO、窗口和拥塞模型都是简化模型，不等同于 Linux TCP。模块 03 的 ARP/逐跳题也不意味着 Python 路径增加了真实 ARP 发包。

### C++ 五层仿真与轻量 SDN

公共头文件在 `include/netsim/`，实现放在 `src/`，测试放在 `tests/`；下表文件名相对于该工程，头文件通常位于 `include/netsim/`。

| 部分 | 主要定位入口 |
|------|--------------|
| 基础类型与计算 | `types.hpp`、`checksum.hpp`、`crc.hpp`、`capacity.hpp`、`channel_util.hpp`。 |
| 物理链路 | `phy.hpp` / `src/phy.cpp`，`PhyLink` 与故障参数。 |
| 链路与介质访问 | `ethernet.hpp`、`csma_cd.hpp`、`csma_ca.hpp`、`token_ring.hpp`、`mac_switch.hpp`、`sliding_window.hpp` 及对应实现。 |
| 网络与路由 | `ipv4.hpp`、`arp.hpp`、`route.hpp`、`routing_algo.hpp` 及对应实现。 |
| 传输与应用 | `udp.hpp`、`tcp.hpp`、`dns.hpp`、`http.hpp` 及对应实现。 |
| 跨层主机 | `host.hpp` / `src/host.cpp`，`Host` 与 `Iface`；端到端路径为 HTTP → TCP → IPv4 → Ethernet → `PhyLink`。 |
| SDN | `flow_table.hpp`、`sdn_switch.hpp`、`sdn_controller.hpp`，对应 `FlowTable`、`SdnSwitch`、`SdnController`。 |

`CMakeLists.txt` 显式列出库源码、可执行程序和测试。新增 `.cpp` 或测试后必须检查构建注册，不能认为文件放入目录就会自动编译。

轻量 SDN 使用同步 Packet-In / Flow-Mod、优先级流表及转发/改写/丢弃动作。它不实现 OpenFlow 线协议、不连接真实网卡，也不提供多级流表或生产控制器能力；维护时保留首包重查、校验和重算和无控制器时的丢弃语义。

C++ 中存在 ARP/TCP 等报文模型，不意味着 Python 模块或真实网络实验已获得同等实现；各路径的覆盖范围应分别说明。

### 教材知识库

- 概念总览：`FIVE_LAYER_CORE.md`、`CROSS_BOOK_LINKS.md`。
- 考点到笔记、习题和代码的映射：`KNOWLEDGE_POINT_MAP.md`、`WANGDAO_CHAPTER_INDEX.md`。
- 疑难与题解：`FAQ_KNOWLEDGE_POINTS.md`、`FAQ_PIVOTAL_ANSWERS.md`、`ANSWER_EXERCISE_INDEX.md`、`ANSWER_EXERCISE_DIGEST.md` 与分章 `ANSWER_EXERCISES_CH*.md`。
- 分章阅读：`NOTES_*`；进度和目标审计：`READING_LOG.md`、`OBJECTIVE_AUDIT.md`。
- `ocr_raw/` 是有识别噪声的资料归档，不是可直接采信的协议规范。修订笔记应核对上下文，区分“材料已归档”“已作摘要”和“逐题人工全解”。
- 阅读覆盖审计和 C++ 测试通过不能替代真实网络毕业验收。

## 6. 环境准备与常用命令

### Python 主线（在仓库根目录执行）

建议使用虚拟环境，不向系统 Python 强行安装依赖：

```bash
python3 -m venv .venv
. .venv/bin/activate
make install-dev PIP="python3 -m pip"
make verify
```

已有合适环境时无需重复创建或安装。根目录 Makefile 支持 `PYTHON` 与 `PIP` 覆盖；`make install-dev` 只安装开发依赖，不是项目发布或系统工具安装命令。

```bash
# 根目录完整门禁：必要文档、Python 模块、真实 loopback 实验、仓库结构
make verify

# 只运行 pytest 部分，默认不包含 wire_probe 的 unittest
PYTHONPATH=. python3 -m pytest modules tests -q

# 单模块示例，仍然测试 solution 而不是 starter
PYTHONPATH=. python3 -m pytest modules/02-huachuang/tests -q

# 真实本机 HTTP/TLS 探针的全部 unittest
python3 -m unittest discover -s real_network_practice/wire_probe -p 'test_*.py'

# 轻量文档与结构检查
make docs-check
python3 tools/verify_repo.py

# 毕业证据结构检查，不自动执行抓包或故障注入
python3 tools/verify_graduation_evidence.py
```

`wire_probe` 的 TLS 测试会调用 **openssl 命令行工具**生成临时证书，需要可用的 `openssl`、Python `ssl` 和本机回环 socket 权限。普通 `make verify` 不要求访问公网，也不需要 tcpdump、tshark 或 sudo。受限环境不能绑定端口或缺失工具时，应记录环境限制，不能直接当作协议实现错误或测试已通过。

### C++ 独立工程（以下命令也从仓库根目录执行）

```bash
cmake -S network_simulation/cpp -B network_simulation/cpp/build
cmake --build network_simulation/cpp/build
ctest --test-dir network_simulation/cpp/build --output-on-failure

# 仅运行 SDN 相关测试
ctest --test-dir network_simulation/cpp/build -R sdn --output-on-failure

# 可选演示
./network_simulation/cpp/build/netsim_demo
./network_simulation/cpp/build/netsim_sdn_demo
```

也可用 `make -C network_simulation/cpp test` 完成配置、构建与 CTest。当前注册的测试为 `netsim_tests`、`netsim_e2e`、`netsim_sdn_tests`、`netsim_sdn_e2e`；具体以 `CMakeLists.txt` 为准。`netsim_demo` 是可选演示，不属于根目录验证。

**根目录 `make verify` 不构建、不测试 C++ 工程；C++ 的 `make test` 也不运行 Python 门禁。**

## 7. 测试机制与门禁的真实含义

- 根目录 `make verify` 依次执行 `docs-check`、`test` 和 `tools/verify_repo.py`。
- `make test` 包含 `PYTHONPATH=. python3 -m pytest modules tests -q`，再执行 `wire_probe` 的 unittest discovery；只运行 pytest 不等价于完整验证。
- 模块测试通过 `tests/solution_loader.py` → `network_simulation/lab_loader.py` 加载 `solution/`。默认 pytest 和 `make verify` **不会验收学习者的 `starter/`**。需要验证练习时，须明确设计加载方式；仓库目前没有内置的 starter 切换开关，不要宣称存在相应环境变量或命令选项。
- `docs-check` 只检查指定文档是否存在，不验证内容正确性或所有内部链接。
- `verify_repo.py` 检查模块骨架目录与 `.py` 文件、证据标签、模块 README 至少 **600 UTF-8 字节**、`docs/*.md` 与模块 README 中不得出现外部 URL，以及来源目录存在。它不是完整的协议正确性或引用合规审计。
- `tests/test_evidence_tags.py` 固定要求 **13 个**模块元数据，且每个均含 `host-verified`。新增/删除模块要协调学习路线、覆盖表、元数据与数量断言，不能只增加目录。
- `module.meta.toml` 的 `host-verified` 表示已纳入主机自动验证的范围，`not-run` 表示人工环境或未纳入自动验证的事项。模块 13 同时包含两种标签，不得将后者一并宣传为自动通过。

按修改范围选择验证：

| 改动 | 最低相关验证 |
|------|--------------|
| 讲义、导航、AGENTS 或资源目录 | 核对内容、路径、外链政策；运行 `make docs-check` 与 `python3 tools/verify_repo.py`。 |
| Python 模块、共享库或加载器 | 先运行相关模块测试，再运行 `make verify`；修改共享能力时检查依赖模块。 |
| `wire_probe` | 运行完整 unittest discovery，再运行 `make verify`，保留真实 HTTP/TLS 和 framing 边界测试。 |
| C++ 源码、头文件或 CMake | 重新配置/构建并运行 CTest；跨层修改覆盖端到端，SDN 修改覆盖 SDN 测试。 |
| 毕业报告、元数据或证据 | 运行证据结构检查，人工逐项核对原始证据和 G0–G7；结构通过不等于毕业通过。 |
| 教材 PDF 或知识笔记 | 核对来源授权、Git LFS 和概念准确性；更新必要的索引，不把归档数量当理解程度。 |

交付时区分“本次实际运行”“仅阅读现有记录”“未运行及原因”，不要把历史测试计数或旧报告中的“全部通过”复制成新结论。

## 8. 代码与教学材料维护规范

- 保留既有拼音 API 命名、中文术语和模块编号，避免无必要的批量重命名。Python 沿用现有四空格缩进、类型注解和 dataclass 风格；C++ 沿用对应文件的布局、命名空间与头/源分工，不对无关代码统一格式化。
- Python 优先使用标准库；新增依赖前说明必要性，并同步维护相关依赖声明。不要为文档修改引入包管理、构建系统或第三方测试框架。
- 公共信道、帧格式、加载器属于跨模块契约；修改签名、序号/ACK 语义、异常或返回值时，检查依赖方和测试，而不是只改调用点使测试静默。
- 二进制协议解析应覆盖短报文、非法长度/标志、校验失败和边界值；改写地址或端口时核对相应校验和。保持教学模型与真实协议字段的区别可见。
- 测试应确定、可重复，随机行为固定 seed；新增本机服务测试优先使用系统分配端口，显式关闭 socket、线程和临时文件，避免引入公网可用性依赖。
- C++ 新用例沿用现有显式检查/失败返回机制，不能只依赖可能在 Release 构建被禁用的 `assert` 来证明行为。
- 扩展能力时同步更新相关模块讲义、测试、元数据、`docs/COVERAGE.md` 和 `docs/TEACHING_GAPS.md`。只撤销确实补齐且经过相应验证的缺口，不因某个演示运行成功宣称完整协议兼容。
- 模块 README 应保留中文学习目标、核心概念、API、不变量、失败模式、讨论和验证说明；达到字节门槛只是底线，不能用无意义文字填充。
- 纸面综合题应交代题设、歧义和假设；例如模块 03 的转发表查表、ARP 对象与逐跳 MAC 变化，不应混成同一种算法或自动验证能力。

## 9. 真实网络实验与证据安全

- 实验仅用于防御性、教育性分析，默认限定为回环或自己控制且获授权的服务；禁止未授权扫描、第三方流量采集、漏洞利用或攻击性 PoC。
- 探针通过 `ssl.create_default_context` 校验证书链与主机名，自有 CA 使用 `--ca-file`。不要为了使测试成功关闭证书验证，`modules/09-tls/fixtures/` 中的测试私钥不得用于真实服务。
- `wire_probe` 的 `recv_events` 是应用读取事件，不是 packet 边界；DNS 计时可能来自缓存，阻塞 resolver 不受 post-DNS deadline 约束。阶段耗时和 recv 间隔不能单独证明拥塞或某层根因。
- 探针保存 wire body 长度与哈希，不负责 chunk 解码或解压；存在 Transfer-Encoding 时完整性保持未知，达到预算须标记截断。维护时保留 TE/CL 冲突、编码顺序、提前断连、超时和参数边界检查。
- `evidence/scripts/run_captures.sh` 会发起外部请求、调用 sudo/tcpdump 并写入固定证据文件；默认接口为 `enp0s3`，可通过 `CAP_IFACE` 指定。它不是无副作用的检查命令，不要在普通测试或文档维护中顺手执行。
- `evidence/scripts/run_fault_netns.sh` 会创建并清理固定名称的 `grad-fault-ns`、`veth-gf0` / `veth-gf1`，修改 namespace 内 iptables，并尝试在宿主 `lo` 上添加/删除测试 qdisc。运行前必须确认任务授权、隔离条件、权限与名称冲突，不得影响用户既有网络配置。
- 原始 pcap、命令输出和日志放在 `evidence/raw/`；派生汇总写入 `results.json` 与报告。保留失败尝试，不以合成 JSON、内存 TCP 轨迹或手工构造 pcap 替代真实毕业证据。
- 抓包可能含地址、请求内容或其他敏感信息；新增或重抓证据前确认保存位置，公开提交前核查隐私和许可，不覆盖现有原始证据来掩盖失败。

## 10. 毕业闸门与已知缺口

状态以 `GRADUATION.md`、`evidence/results.json`、`evidence/final-report.md`、`evidence/failures.md` 和 `evidence/environment.json` 的一致记录为准。维护本文件时，已有归档证据轮次为 **2026-10-03**，仓库记录为 `NOT_CLOSED`，`graduation_status` 为 `NOT_PASS`；这些是历史证据包的结论，不是对当前机器能力的新检测。

- G0–G4 记录为 pass：环境、DNS、TCP、TLS 与 HTTP/1.1 基线。
- G5 为 not-run：有 HTTP/2 pcap，但该轮环境不支持 QUIC/HTTP3。
- G6 为 not-run：已有 netns + TCP reset 证据，netem 的 delay/loss/reorder/带宽注入未完成。
- G7 为 not-run：连接重置仅有部分传输层证据，慢上传与流式卡顿缺少完整真实证据链。

三个症状都需建立“报文 → 套接字 → 进程 → 应用请求”的证据链，并说明候选瓶颈与未知项。只有 RST 或一张 Wireshark 截图不能证明完整诊断；不能根据缺失的 pcap 编造吞吐、时延或根因。

`python3 tools/verify_graduation_evidence.py` 主要校验文件存在和有限的一致性条件。即使输出 `graduation evidence structure: OK`，也可能同时提示 `NOT_CLOSED`。

当前存在一项需显式说明的规则差异：`GRADUATION.md` 的文字允许合规 not-run，而结构脚本在 `graduation_status == "PASS"` 时要求 **G0–G7 全部为 `pass`**，并检查 commit SHA、raw 文件和报告标记。涉及最终判定时应先明确并协调这一差异，不应自行放宽脚本或提前改写 PASS；本文件不替代正式验收规则。

此外，不要跨范围宣传已有能力：Python 教学路径不覆盖生产 TCP 全状态与拥塞耦合、真实 NIC 或 ARP on-wire；C++ 仿真不等于内核协议栈；当前路径不实现完整 HTTP/2/QUIC、BGP 策略或运营商级拓扑。具体缺口仍查阅 `docs/TEACHING_GAPS.md`。

## 11. 引用、大文件与交付检查

- 官方规范、工具文档和参考课程的外部链接统一登记到 `sources/catalog.toml`；讲义与注释使用 RFC 编号、中文释义和仓库相对链接，不把外链散落到其他文档。
- 遵守原创与学术诚信政策：不复制课程实验 PDF、评分规则、官方答案或第三方仓库实现。概念可以对齐，接口、场景和教学表达应独立。
- `.gitattributes` 将 `books/*.pdf` 配置为 Git LFS 文件。需要真实 PDF 内容时先确认已安装 Git LFS 且文件不是 pointer；不要把 LFS 指针误判为损坏 PDF，也不要移除 LFS 配置或直接提交大型二进制副本。
- 不新增未经授权的教材、答案、试卷、长篇 OCR 转载或第三方抓包；维护参考资料时遵守 `books/README.md` 与资源政策。
- `.venv/`、Python 缓存、`build/`、对象文件和静态库已配置忽略，不纳入源码提交；清理构建目录前确认精确路径，不对仓库根目录或已有证据做递归删除。
- 完成后检查变更范围、格式与路径有效性，说明本次改了什么、实际执行了哪些验证，以及仍未验证或待补齐的事项；不要为当前任务自动创建提交、重抓证据或更新历史验收结论。
