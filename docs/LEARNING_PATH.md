# 计算机网络学习路径（中文）

面向已学过一门编程课、希望**系统走通**本科计算机网络主线的学习者。风格与同仓库家族的 OS / 算法路径一致：**模块递进 + starter/solution + `make verify`**。

## 如何使用

1. 阅读本文件与 `docs/COVERAGE.md`，确认目标与缺口。
2. 按 `modules/01` → `13` 顺序学习；每模块先读中文 `README.md`，再在 `starter/` 完成作业，对照 `solution/` 自检。模块 13 为**真实协议诊断**（计划第 11 节），详见 `docs/ZHENSHI_XIEYI_ZHENDUAN.md`。
3. 在仓库根目录运行 `make verify`（或 `make install-dev && make verify`）确认主机验证通过。
4. 外链与 RFC 入口仅查 `sources/catalog.toml`。

## 建议节奏（按周示意，非硬性）

| 阶段 | 模块 | 核心能力 |
|------|------|----------|
| 第 1–2 周 | 01–02 | 分层思维、端到端论证、成帧与双向 ACK 可靠发送 |
| 第 3–4 周 | 03–04 | 寻址、转发、DV/LS 直觉 |
| 第 5–6 周 | 05–06 | 拥塞教学模型、DNS/HTTP 路径 |
| 第 7 周 | 07–08 | 套接字 API、NAT 表项 |
| 第 8 周 | 09–10 | TLS 状态机阅读、合成 trace 分析 |
| 第 9 周 | 11 | 故障注入与现象归类 |
| 第 10–11 周 | 12 | 大作业：可靠通道 + 简易应用 |
| 第 12–13 周 | 13 + 根目录毕业 | 真实协议诊断；毕业证据见 `GRADUATION.md` 与 `evidence/`（真实 pcap，`graduation_status` 未 PASS 前不得用仿真 TCP 宣告毕业） |

## 模块目录

```
modules/
  01-cengdie/          # 分层与端到端
  02-huachuang/        # 成帧 + HuachuangKexinTongdao（ACK/重传）
  03-ip-zhuanfa/       # IP 转发仿真
  04-luyou/            # 路由迷你仿真
  05-yongjin/          # TCP 拥塞教学模型
  06-yingyong/         # DNS/HTTP stub
  07-taojie/           # 套接字实验
  08-nat/              # NAT 直觉
  09-tls/              # TLS 高层
  10-celiang/          # 测量与调试
  11-guzhang/          # 故障案例馆
  12-capstone/         # 期末大作业
  13-zhenduan/         # 真实协议诊断（计划第 11 节）
```

## 大作业（模块 12）目标

在 `ShuangxiangLianlu` 与模块 02 可靠层之上实现 `XinyunTongdao`，承载极简「类 HTTP」请求 `GET /xinxi` 返回 JSON 状态；须在 `make verify` 中端到端通过（含适度丢包场景）。

## 学术诚信

- 作业提交应以 `starter/` 为起点独立完成；`solution/` 仅供课后对照。
- 不得将本仓库代码当作任何课程官方答案上传或抄袭。

## 证据与 CI

- 根目录 `make verify` 聚合各模块 `host-verified` 测试。
- 标注 `not-run` 的条目见各模块 `module.meta.toml`，本仓库当前**无**必选 not-run 项。
