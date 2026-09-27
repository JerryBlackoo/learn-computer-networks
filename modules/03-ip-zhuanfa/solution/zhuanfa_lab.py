from __future__ import annotations

from dataclasses import dataclass
from ipaddress import ip_network
from typing import List, Optional, Tuple


@dataclass(frozen=True)
class ZhuanfaXiang:
    mudi_wang: str
    changdu: int
    xiaoyi_tiaojie: str


@dataclass
class MoNiZhuanfa:
    """最长前缀匹配转发表。"""

    biao: List[ZhuanfaXiang]

    def chazhao(self, mudi_ip: str) -> Optional[str]:
        best: Optional[Tuple[int, str]] = None
        addr = ip_network(f"{mudi_ip}/32", strict=False).network_address
        for row in self.biao:
            net = ip_network(row.mudi_wang, strict=False)
            if addr in net:
                if best is None or row.changdu > best[0]:
                    best = (row.changdu, row.xiaoyi_tiaojie)
        return best[1] if best else None
