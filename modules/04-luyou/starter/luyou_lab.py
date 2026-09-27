from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict, List, Tuple


@dataclass
class JuliShiliang:
    jiedian: str
    linju_daijuli: Dict[str, int] = field(default_factory=dict)
    juli_biao: Dict[str, int] = field(default_factory=dict)

    def chushihua(self, quan_jiedian: List[str]) -> None:
        raise NotImplementedError

    def jiaohuan_yilun(self, linju_biao: Dict[str, Dict[str, int]]) -> None:
        raise NotImplementedError


@dataclass
class LianluZhuangtai:
    @staticmethod
    def zuiduan_lujing(
        yuan: str, tuopu: Dict[str, List[Tuple[str, int]]]
    ) -> Dict[str, int]:
        raise NotImplementedError
