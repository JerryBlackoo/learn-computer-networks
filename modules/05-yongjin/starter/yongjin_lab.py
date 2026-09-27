from __future__ import annotations

from dataclasses import dataclass
from enum import Enum, auto


class Jieduan(Enum):
    MANQI = auto()
    BISE = auto()


@dataclass
class YongjinMoxing:
    cwnd: float = 1.0
    ssthresh: float = 8.0
    jieduan: Jieduan = Jieduan.MANQI

    def shijian_bu(self, ack_count: int, loss: bool = False) -> None:
        raise NotImplementedError
