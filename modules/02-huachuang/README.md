# 模块 02：不可靠链路上的可靠交付

在 `KexinLianlu` 上实现原创滑动窗口协议 `HuachuangTongdao`：序号、累积 ACK、固定窗口。

## API

- `HuachuangTongdao.chuanshu(shuju: bytes) -> None` 发送端驱动
- `HuachuangTongdao.jieshou_wanzheng() -> bytes | None` 接收端取完整报文

完成 `starter/huachuang_lab.py`。solution 通过 `make verify`。
