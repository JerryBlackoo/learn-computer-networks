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


@dataclass
class ChuanShuJiaoxue:
    """教学用发送端直觉：滑动窗口、RTO、重复 ACK 触发快重传（非 RFC 完整 TCP）。"""

    chuangkou: int = 4
    rtt_guji: float = 0.1
    rto: float = 1.0
    zaiwang: int = 0
    xia_yi_xuhao: int = 0
    yiqueren: int = -1
    dup_ack_lianxu: int = 0
    kuai_chongchuan: bool = False

    def keyi_fasong(self) -> bool:
        return self.zaiwang < self.chuangkou

    def fasong_bao(self) -> int:
        if not self.keyi_fasong():
            raise RuntimeError("窗口已满")
        seq = self.xia_yi_xuhao
        self.xia_yi_xuhao += 1
        self.zaiwang += 1
        return seq

    def jieshou_ack(self, ack_num: int) -> None:
        if ack_num <= self.yiqueren:
            self.dup_ack_lianxu += 1
            if self.dup_ack_lianxu >= 3:
                self.kuai_chongchuan = True
            return
        self.dup_ack_lianxu = 0
        self.kuai_chongchuan = False
        released = max(0, ack_num - self.yiqueren)
        self.yiqueren = ack_num
        self.zaiwang = max(0, self.zaiwang - released)

    def gengxin_rtt(self, yangben: float) -> None:
        """极简 RTO：RTO ≈ 2×RTT 估计（不实现 Karn/ Jacobson 全细节）。"""
        self.rtt_guji = max(0.001, yangben)
        self.rto = max(0.2, 2.0 * self.rtt_guji)

    def chaoshi(self, jingguo: float) -> bool:
        return jingguo >= self.rto


def _kexin_tongdao_cls():
    from pathlib import Path

    from wangluo_xuejing.lab_loader import load_lab

    mod = load_lab(
        Path(__file__).resolve().parents[2] / "02-huachuang",
        "huachuang_lab.py",
    )
    return mod.HuachuangKexinTongdao


@dataclass
class BaoJiKexinYanlian:
    """包级联动：模块 02 可靠成帧 + 本模块窗口直觉（主机仿真，非 RFC TCP）。"""

    chuangkou: int = 3
    duan_max: int = 8

    def chuanshu_bing_jiankong(self, shuju: bytes, diubao_gailv: float = 0.0, seed: int = 0) -> dict:
        from wangluo_xuejing.kexin_lianlu import LianluCanshu
        from wangluo_xuejing.shuangxiang_lianlu import ShuangxiangLianlu

        link = ShuangxiangLianlu.fenli_guzhang(LianluCanshu(diubao_gailv=diubao_gailv, seed=seed))
        kx = _kexin_tongdao_cls()(link, chuangkou=self.chuangkou, duan_max=self.duan_max, rto=0.15)
        kx.chuanshu_kexin(shuju)
        out = kx.jieshou_wanzheng_kexin()
        return {
            "shuju_wanzheng": out == shuju,
            "chongchuan_cishu": kx.chongchuan_cishu,
            "zuihou_zaiwang": kx.zaiwang,
            "kuai_chongchuan_ceng": kx.kuai_chongchuan,
        }
