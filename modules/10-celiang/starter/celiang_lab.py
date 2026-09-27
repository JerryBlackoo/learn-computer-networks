from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Any, List


@dataclass
class CeLiangJiao:
    shijian_xian: List[dict[str, Any]]

    @classmethod
    def cong_json(cls, path: Path) -> "CeLiangJiao":
        raise NotImplementedError

    def rtt_guji(self) -> float | None:
        raise NotImplementedError

    def shuju_bao_shu(self) -> int:
        raise NotImplementedError
