# HTTP 帧边界也是诊断输入

wire probe 现在按 Transfer-Encoding 编码列表识别最后的 chunked，拒绝 Content-Length 与 Transfer-Encoding 同时出现，拒绝 chunked 非最终编码或重复出现。此前仅搜索固定字符串，gzip, chunked 会漏判，可能报告没有依据的 body_complete。

此探针保存原始 wire bytes 的长度和哈希，不执行 chunk 解码或 gzip 解压；任何 Transfer-Encoding 下 body_complete 都保持 unknown/None。recv 事件仍是应用读取，不是抓包，也不能仅用阶段耗时归因根因。

新增三个真实 loopback HTTP 反例：TE/CL 冲突、编码列表、非法顺序；既有 HTTP、验证过的 TLS、提前断开和时间预算继续覆盖。DNS 的阻塞 resolver 不在 post-DNS deadline 内；公共网络、HTTP/2/QUIC 与真实丢包实验本轮 not-run。
