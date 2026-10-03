# 第二轮真实协议探针

post-DNS阶段共用绝对deadline，避免每个recv重置超时后慢速响应无限拖长；解析器仍是阻塞系统resolver，DNS阶段不在该deadline内。参数NaN/Inf及非法bytes边界拒绝。Content-Length短响应单独标body_complete=false，不把连接结束当完整成功；chunked仅保存wire bytes，不解码其完整性。Host使用scheme对应默认端口判断。

完整make verify通过：42项pytest与5项wire probe unittest（真实本机HTTP、校验证书的TLS及提前断开连接）。它们不是抓包、HTTP/2/QUIC或公网协议行为证据；recv边界不能当packet边界，时延不能独自归因。
