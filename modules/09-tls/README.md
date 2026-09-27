# 模块 09：TLS 高层（握手状态机 + 标准库）

## 学习目标

- `TlsZhuangtaiJi`：手写**握手阶段日志**（ClientHello → ServerHello → 应用数据），理解顺序约束。
- 使用 Python `ssl` 加载 `fixtures/selfsigned.pem`（**不自研密码学**、非生产 PKI）。

## 核心概念

- **状态机**：非法顺序（如未 ClientHello 就 ServerHello）必须 `RuntimeError`。
- **Fixture**：自签证书仅用于本地 `SSLContext` 加载演练；`zhengshu_fixture_cunzai` 检查文件可读。

## 不变量

- 不得将 fixture 私钥用于公网服务或冒充第三方站点。
- 日志 `rizhi` 仅记录阶段名，不记录主密钥材料。

## 常见失败模式

- 跳过证书链验证讨论，误以为自签等于「可信」。
- 将本模块与漏洞利用、中间人攻击工具混为一谈（**禁止**未授权目标）。

## 明确非目标

- 无自定义密码套件、无 TLS 1.3 密钥导出细节、无 HSM、无证书透明度日志。

## 作业

完成 `starter/tls_lab.py` 状态机与辅助函数。

## 讨论题

1. 浏览器如何信任 CA？与本仓库自签有何本质区别？
2. 前向保密（PFS）在握手消息中对应哪类密钥交换直觉？

## 验证

```bash
pytest modules/09-tls/tests -q
```

证据标签：`host-verified`。
