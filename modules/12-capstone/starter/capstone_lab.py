from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

from wangluo_xuejing.kexin_lianlu import KexinLianlu


@dataclass
class XinyunTongdao:
    lianlu: KexinLianlu

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
