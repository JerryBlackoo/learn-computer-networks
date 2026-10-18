from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

from network_simulation.shuangxiang_lianlu import ShuangxiangLianlu


@dataclass
class XinyunTongdao:
    lianlu: ShuangxiangLianlu
    chuangkou: int = 4
    duan_max: int = 32

    def fasong(self, shuju: bytes) -> None:
        raise NotImplementedError

    def jieshou(self) -> Optional[bytes]:
        raise NotImplementedError


@dataclass
class XinxiYingyong:
    tongdao: XinyunTongdao

    def chuli_qingqiu(self, qingqiu: bytes) -> bytes:
        raise NotImplementedError

    def yici_jiaohu(self, qingqiu: bytes) -> bytes:
        raise NotImplementedError
