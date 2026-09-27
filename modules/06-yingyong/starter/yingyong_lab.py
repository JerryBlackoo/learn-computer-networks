from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict, Optional


@dataclass
class BenDiYuming:
    jilu: Dict[str, str] = field(default_factory=dict)

    def tianjia(self, zhuji: str, ip: str) -> None:
        raise NotImplementedError

    def chaxun_a(self, zhuji: str) -> Optional[str]:
        raise NotImplementedError


@dataclass
class JianYiHttp:
    def goujian_qingqiu(self, fangfa: str, lujing: str, zhuji: str) -> bytes:
        raise NotImplementedError

    def jiexi_zhuangtai(self, xiangying: bytes) -> int:
        raise NotImplementedError
