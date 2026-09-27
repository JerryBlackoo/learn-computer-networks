from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any, List


@dataclass
class CeLiangJiao:
    shijian_xian: List[dict[str, Any]]

    @classmethod
    def cong_json(cls, path: Path) -> "CeLiangJiao":
        data = json.loads(path.read_text(encoding="utf-8"))
        return cls(shijian_xian=data)

    def rtt_guji(self) -> float | None:
        raise NotImplementedError

    def shuju_bao_shu(self) -> int:
        raise NotImplementedError

    def chongchuan_bao_shu(self) -> int:
        raise NotImplementedError

    def pingjun_shuju_daxiao(self) -> float:
        raise NotImplementedError
