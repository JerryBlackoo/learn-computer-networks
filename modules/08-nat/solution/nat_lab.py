from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict, Tuple


@dataclass
class ZhongjianHe:
    """NAPT 教学表：内网 (ip,port) <-> 公网端口。"""

    gongwang_ip: str
    xiayige_duankou: int = 40000
    chu_biao: Dict[Tuple[str, int], Tuple[str, int]] = field(default_factory=dict)
    ru_biao: Dict[int, Tuple[str, int]] = field(default_factory=dict)

    def chu_kou(self, nei_ip: str, nei_port: int) -> Tuple[str, int]:
        key = (nei_ip, nei_port)
        if key not in self.chu_biao:
            self.chu_biao[key] = (self.gongwang_ip, self.xiayige_duankou)
            self.ru_biao[self.xiayige_duankou] = key
            self.xiayige_duankou += 1
        return self.chu_biao[key]

    def ru_kou(self, gong_port: int) -> Tuple[str, int] | None:
        return self.ru_biao.get(gong_port)
