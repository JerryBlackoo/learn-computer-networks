from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict, Tuple


@dataclass
class ZhongjianHe:
    gongwang_ip: str
    xiayige_duankou: int = 40000
    chu_biao: Dict[Tuple[str, int], Tuple[str, int]] = field(default_factory=dict)
    ru_biao: Dict[int, Tuple[str, int]] = field(default_factory=dict)

    def chu_kou(self, nei_ip: str, nei_port: int) -> Tuple[str, int]:
        raise NotImplementedError

    def ru_kou(self, gong_port: int) -> Tuple[str, int] | None:
        raise NotImplementedError
