"""可注入故障的不可靠链路（模块 02/11 共用）。"""

from __future__ import annotations

import random
from collections import deque
from dataclasses import dataclass, field
from typing import Deque, List, Optional


@dataclass
class LianluCanshu:
    diubao_gailv: float = 0.0
    chongpai_gailv: float = 0.0
    chongfu_gailv: float = 0.0
    fenqu: bool = False
    seed: int = 42


@dataclass
class KexinLianlu:
    """单向不可靠信道：发送端 push，接收端 pop。"""

    canshu: LianluCanshu = field(default_factory=LianluCanshu)
    _rx_queue: Deque[bytes] = field(default_factory=deque)
    _rng: random.Random = field(init=False)

    def __post_init__(self) -> None:
        self._rng = random.Random(self.canshu.seed)

    def fasong(self, zhen: bytes) -> None:
        if self.canshu.fenqu:
            return
        if self._rng.random() < self.canshu.diubao_gailv:
            return
        self._rx_queue.append(zhen)
        if self._rng.random() < self.canshu.chongfu_gailv:
            self._rx_queue.append(zhen)
        if self._rng.random() < self.canshu.chongpai_gailv:
            if len(self._rx_queue) >= 2:
                a = self._rx_queue.pop()
                b = self._rx_queue.pop()
                self._rx_queue.append(a)
                self._rx_queue.append(b)

    def jieshou(self) -> Optional[bytes]:
        if not self._rx_queue:
            return None
        return self._rx_queue.popleft()

    def qingkong(self) -> None:
        self._rx_queue.clear()
