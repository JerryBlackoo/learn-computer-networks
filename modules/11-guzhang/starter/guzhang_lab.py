from __future__ import annotations

from dataclasses import dataclass
from typing import List


@dataclass
class GuzhangBowuguan:
    def fenlei_diubao(self, seed: int = 1) -> str:
        raise NotImplementedError

    def fenlei_chongpai(self, seed: int = 7) -> str:
        raise NotImplementedError

    def fenlei_chongfu(self, seed: int = 3) -> str:
        raise NotImplementedError

    def fenlei_fenqu(self) -> str:
        raise NotImplementedError

    def zhengti_baogao(self) -> dict[str, str]:
        raise NotImplementedError

    def liebiao_anli(self) -> List[str]:
        raise NotImplementedError
