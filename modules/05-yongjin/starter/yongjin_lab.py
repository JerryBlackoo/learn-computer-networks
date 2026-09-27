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


@dataclass
class ChuanShuJiaoxue:
    chuangkou: int = 4
    rtt_guji: float = 0.1
    rto: float = 1.0
    zaiwang: int = 0
    xia_yi_xuhao: int = 0
    yiqueren: int = -1
    dup_ack_lianxu: int = 0
    kuai_chongchuan: bool = False

    def keyi_fasong(self) -> bool:
        raise NotImplementedError

    def fasong_bao(self) -> int:
        raise NotImplementedError

    def jieshou_ack(self, ack_num: int) -> None:
        raise NotImplementedError

    def gengxin_rtt(self, yangben: float) -> None:
        raise NotImplementedError

    def chaoshi(self, jingguo: float) -> bool:
        raise NotImplementedError
