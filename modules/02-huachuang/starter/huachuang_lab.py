"""模块 02 starter."""

from __future__ import annotations

from dataclasses import dataclass

from wangluo_xuejing.kexin_lianlu import KexinLianlu


@dataclass
class HuachuangTongdao:
    lianlu: KexinLianlu
    chuangkou: int = 4
    duan_max: int = 32

    def chuanshu(self, shuju: bytes) -> None:
        raise NotImplementedError

    def jieshou_wanzheng(self) -> bytes | None:
        raise NotImplementedError
