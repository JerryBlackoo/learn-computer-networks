"""模块 02 starter."""

from __future__ import annotations

from dataclasses import dataclass

from wangluo_xuejing.kexin_lianlu import KexinLianlu
from wangluo_xuejing.shuangxiang_lianlu import ShuangxiangLianlu


@dataclass
class HuachuangTongdao:
    lianlu: KexinLianlu
    chuangkou: int = 4
    duan_max: int = 32

    def chuanshu(self, shuju: bytes) -> None:
        raise NotImplementedError

    def jieshou_wanzheng(self) -> bytes | None:
        raise NotImplementedError


@dataclass
class HuachuangKexinTongdao:
    lianlu: ShuangxiangLianlu
    chuangkou: int = 4
    duan_max: int = 32
    rto: float = 0.5

    chongchuan_cishu: int = 0

    def chuanshu_kexin(self, shuju: bytes) -> None:
        raise NotImplementedError

    def jieshou_wanzheng_kexin(self) -> bytes | None:
        raise NotImplementedError
