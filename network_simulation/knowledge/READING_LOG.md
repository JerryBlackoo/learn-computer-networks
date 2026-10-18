# 四资料阅读进度日志

## 目标第 1 条：章节阅读与知识点总结

| 资料 | 五层章节 | 页级 OCR / 文本 | 知识点总结证据 |
|------|----------|-----------------|----------------|
| 谢希仁教材 | 第2–6章 PDF≈p53–344 | **292/292 页** 已入 `ocr_raw/xie_*.txt` | 分章 NOTES、`NOTES_SECTIONS_FROM_OCR.md`、`OCR_KEYWORD_INDEX.md`、`NOTES_OCR_EXCERPTS_CH5_CH6.md` |
| 谢希仁答案 | 第1–6章 | `ans_ch1..6.txt` 全文可提取 | `FAQ_KNOWLEDGE_POINTS.md`（174问）、`FAQ_PIVOTAL_ANSWERS.md`、`ANSWER_EXERCISE_INDEX.md`、`ANSWER_EXERCISE_DIGEST.md` |
| 期末试卷 | 全卷 | 已抽取 | `NOTES_EXAM.md` |
| 王道 | 第2–6章（书签页） | **章内页 OCR 齐全**（ch2–6 页校验 0 miss） | `WANGDAO_CHAPTER_INDEX.md`、`NOTES_WANGDAO_LAYER_OUTLINE.md`、`WANGDAO_OCR_KEYWORDS.md`、`KNOWLEDGE_POINT_MAP.md`（叶子↔笔记/FAQ/仿真） |

关联总览：`CROSS_BOOK_LINKS.md`、`FIVE_LAYER_CORE.md`。

## 目标第 2 条：五层 C/C++ 仿真

目录 `network_simulation/cpp/`。`make test` 通过。覆盖：分层封装、CRC、校验和、滑动窗口与利用率、CSMA/CD、交换机自学习、ARP、LPM、DV/LS、TCP 连接与拥塞、DNS/HTTP、端到端。

## 完成审计注意事项

- OCR 有识别噪声，精读笔记为抽样+结构化索引；叶子知识点以王道书签为权威清单并已映射仿真。  
- 答案**全部**习题逐步演算未 100% 人工誊入笔记（原文在 `ans_ch*.txt`，摘要在 DIGEST）。  
- 若要求「笔记中逐题手写全解」则该项仍算未完；若接受「原文归档+FAQ 全量+摘要蒸馏+仿真映射」则阅读主线可视为达标。
