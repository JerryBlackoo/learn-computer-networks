# network_simulation

本目录包含两套教学资产：

1. **Python 共享库**（既有）：`kexin_lianlu.py` 等，供 `modules/` 作业使用。
2. **C/C++ 五层仿真**（`cpp/`）：按谢希仁五层模型实现封装/解封装与核心算法，见下方。

## 知识点梳理

| 文档 | 内容 |
|------|------|
| [knowledge/FIVE_LAYER_CORE.md](knowledge/FIVE_LAYER_CORE.md) | 五层核心知识点、跨层联系、试卷映射、算法—数据结构对照 |
| [knowledge/WANGDAO_CHAPTER_INDEX.md](knowledge/WANGDAO_CHAPTER_INDEX.md) | 王道书签导出的全量章节知识点索引 |

资料 PDF 位于仓库 `books/`（谢希仁教材/答案、期末试卷、王道）。

## C/C++ 五层仿真

```bash
cd network_simulation/cpp
make test
```

实现覆盖：物理信道、以太网+CRC、ARP、GBN 滑动窗口、IPv4+校验和、LPM、UDP/TCP（握手与拥塞控制）、DNS/HTTP，以及端到端 HTTP 测试。详情见 [cpp/README.md](cpp/README.md)。
