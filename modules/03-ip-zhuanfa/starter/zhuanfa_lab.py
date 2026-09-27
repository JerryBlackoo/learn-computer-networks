from __future__ import annotations

from dataclasses import dataclass
from typing import List, Optional


@dataclass(frozen=True)
class ZhuanfaXiang:
    mudi_wang: str
    changdu: int
    xiaoyi_tiaojie: str


@dataclass
class MoNiZhuanfa:
    biao: List[ZhuanfaXiang]

    def chazhao(self, mudi_ip: str) -> Optional[str]:
        raise NotImplementedError
