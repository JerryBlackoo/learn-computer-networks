from __future__ import annotations

from dataclasses import dataclass, field
from enum import Enum, auto


class Jieduan(Enum):
    MANQI = auto()
    BISE = auto()


@dataclass
class YongjinMoxing:
    """简化 AIMD + 慢启动（离散步）。"""

    cwnd: float = 1.0
    ssthresh: float = 8.0
    jieduan: Jieduan = Jieduan.MANQI
    _bu: int = 0

    def shijian_bu(self, ack_count: int, loss: bool = False) -> None:
        if loss:
            self.ssthresh = max(self.cwnd / 2, 1.0)
            self.cwnd = 1.0
            self.jieduan = Jieduan.MANQI
            return
        self._bu += 1
        if self.jieduan == Jieduan.MANQI:
            self.cwnd += 1.0
            if self.cwnd >= self.ssthresh:
                self.jieduan = Jieduan.BISE
        else:
            self.cwnd += 1.0 / max(int(self.cwnd), 1)
        self.cwnd += 0.01 * ack_count
