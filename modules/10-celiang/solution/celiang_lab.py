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
        syn = next(
            (e for e in self.shijian_xian if "SYN" in e.get("flags", []) and e["dir"] == "c2s"),
            None,
        )
        synack = next(
            (
                e
                for e in self.shijian_xian
                if "SYN" in e.get("flags", []) and "ACK" in e.get("flags", []) and e["dir"] == "s2c"
            ),
            None,
        )
        if not syn or not synack:
            return None
        return float(synack["t"] - syn["t"])

    def shuju_bao_shu(self) -> int:
        return sum(
            1
            for e in self.shijian_xian
            if e.get("len", 0) > 0
            and e["dir"] == "c2s"
            and not e.get("retrans")
            and "SYN" not in e.get("flags", [])
        )

    def chongchuan_bao_shu(self) -> int:
        return sum(1 for e in self.shijian_xian if e.get("retrans"))

    def pingjun_shuju_daxiao(self) -> float:
        lens = [
            e.get("len", 0)
            for e in self.shijian_xian
            if e.get("len", 0) > 0
            and e["dir"] == "c2s"
            and not e.get("retrans")
            and "SYN" not in e.get("flags", [])
        ]
        if not lens:
            return 0.0
        return sum(lens) / len(lens)
