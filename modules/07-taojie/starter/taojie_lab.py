from __future__ import annotations

from dataclasses import dataclass


@dataclass
class TaoJieLab:
    def echo_fuwu(self, duankou: int, yici_huifu: bytes) -> None:
        raise NotImplementedError

    def echo_kehu(self, duankou: int, xiaoxi: bytes) -> bytes:
        raise NotImplementedError
