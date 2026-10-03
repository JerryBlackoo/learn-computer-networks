# 从仿真到真实 socket

运行 `python3 projects/wire_probe/probe.py http://127.0.0.1:8000/`。另一个终端在临时目录运行 `python3 -m http.server 8000 --bind 127.0.0.1`。HTTPS 实验使用 `--ca-file` 指定自己的 CA，客户端始终校验证书与 hostname；不要用关闭校验代替配置 CA。

`make verify` 启动真实 HTTP 服务和带临时证书的 TLS 服务，断言状态、实际 body 长度和握手结果。证书每次生成，测试结束服务关闭。报告记录解析、连接、TLS、首字节和每次 recv 的时间。DNS 项可能命中缓存；recv 可以合并多个报文；chunked body 是线上编码字节，尚未解码。默认上限 64 KiB，达到上限标记 truncated，不能把截断数据当完整响应。连接失败、证书失败、读超时会显式报错。

练习：固定服务和输入，分别改变应用生成速度、socket 消费速度、代理缓冲策略，再收集两端日志与自己流量的抓包。仅凭首字节/recv 间隔不能区分服务器等待、代理等待、拥塞或客户端停顿。记录假设和能够排除它的下一项观测。TLS 内容不可直接从 pcap 读取。

已验证范围：本机 HTTP、验证证书的 TLS 和无凭据 URL 边界。尚未执行：tcpdump 报文字段对照、netem、上传故障、HTTP/2/QUIC。不要将本实验通过写成网络毕业验收全部通过。
