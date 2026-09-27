from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict, Optional


@dataclass
class BenDiYuming:
    jilu: Dict[str, str] = field(default_factory=dict)

    def tianjia(self, zhuji: str, ip: str) -> None:
        self.jilu[zhuji.lower()] = ip

    def chaxun_a(self, zhuji: str) -> Optional[str]:
        return self.jilu.get(zhuji.lower())


@dataclass
class JianYiHttp:
    def goujian_qingqiu(self, fangfa: str, lujing: str, zhuji: str) -> bytes:
        return f"{fangfa.upper()} {lujing} HTTP/1.1\r\nHost: {zhuji}\r\n\r\n".encode()

    def jiexi_zhuangtai(self, xiangying: bytes) -> int:
        first = xiangying.split(b"\r\n", 1)[0]
        parts = first.decode("ascii", errors="replace").split()
        if len(parts) < 2:
            return 0
        return int(parts[1])
