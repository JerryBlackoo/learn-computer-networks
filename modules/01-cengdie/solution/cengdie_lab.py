"""模块 01 solution — 分层封装教学实现。"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import List

from network_simulation.xinxi import FengzhuangKuai, validate_non_empty


@dataclass
class DuanDaoDuanLunzheng:
    yuansu: List[str] = field(default_factory=list)

    def dengji(self, yuansu_ming: str) -> None:
        validate_non_empty([yuansu_ming])
        if yuansu_ming not in self.yuansu:
            self.yuansu.append(yuansu_ming)

    def shifou_zai_duandian(self, yuansu_ming: str) -> bool:
        return yuansu_ming in self.yuansu


@dataclass
class FengzhuangZhan:
    cengxu: List[str]

    def fengzhuang(self, zaihe: bytes) -> List[FengzhuangKuai]:
        validate_non_empty(self.cengxu)
        current = zaihe
        kuai_lie: List[FengzhuangKuai] = []
        for ceng in self.cengxu:
            header = f"{ceng}|".encode()
            current = header + current
            kuai_lie.append(FengzhuangKuai(cengji=ceng, zaihe=current))
        return kuai_lie

    def jiefengzhuang(self, kuai_lie: List[FengzhuangKuai]) -> bytes:
        if len(kuai_lie) != len(self.cengxu):
            raise ValueError("层数不匹配")
        data = kuai_lie[-1].zaihe
        for ceng in reversed(self.cengxu):
            prefix = f"{ceng}|".encode()
            if not data.startswith(prefix):
                raise ValueError(f"层 {ceng} 头不匹配")
            data = data[len(prefix) :]
        return data
