"""模块 02 solution — 分片成帧、校验、固定窗口内乱序重组；双向 ACK 可靠发送。"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict, List, Optional, Tuple

from network_simulation.huachuang_zhen import bianma_ack, bianma_shuju, jiexi_bao
from network_simulation.kexin_lianlu import KexinLianlu
from network_simulation.shuangxiang_lianlu import ShuangxiangLianlu


def _frame(seq: int, chunk: bytes, fin: bool) -> bytes:
    return bianma_shuju(seq, chunk, fin)


def _parse(frame: bytes):
    parsed = jiexi_bao(frame)
    if not parsed or parsed[0] != "data":
        return None
    _, seq, fin, chunk = parsed
    return seq, fin, chunk


@dataclass
class HuachuangTongdao:
    lianlu: KexinLianlu
    chuangkou: int = 4
    duan_max: int = 32

    _huancong: Dict[int, Tuple[bytes, bool]] = field(default_factory=dict)
    _next: int = 0
    _done: Optional[bytes] = field(default=None)
    _leiji: list[bytes] = field(default_factory=list)

    def chuanshu(self, shuju: bytes) -> None:
        seq = 0
        for i in range(0, len(shuju), self.duan_max):
            chunk = shuju[i : i + self.duan_max]
            fin = i + self.duan_max >= len(shuju)
            self.lianlu.fasong(_frame(seq, chunk, fin))
            seq += 1

    def _xishou(self, seq: int, fin: bool, chunk: bytes) -> None:
        if seq < self._next:
            return
        if seq > self._next:
            if seq - self._next >= self.chuangkou:
                return
            self._huancong[seq] = (chunk, fin)
            return
        self._leiji.append(chunk)
        self._next += 1
        if fin:
            self._done = b"".join(self._leiji)
            return
        while self._next in self._huancong:
            c, f = self._huancong.pop(self._next)
            self._leiji.append(c)
            self._next += 1
            if f:
                self._done = b"".join(self._leiji)
                return

    def jieshou_wanzheng(self) -> bytes | None:
        if self._done is not None:
            return self._done
        while True:
            raw = self.lianlu.jieshou()
            if raw is None:
                break
            parsed = _parse(raw)
            if not parsed:
                continue
            seq, fin, chunk = parsed
            self._xishou(seq, fin, chunk)
            if self._done is not None:
                return self._done
        return None


@dataclass
class HuachuangKexinTongdao:
    """滑动窗口发送 + 累积 ACK + 超时/快重传；接收侧复用 `HuachuangTongdao` 重组逻辑。"""

    lianlu: ShuangxiangLianlu
    chuangkou: int = 4
    duan_max: int = 32
    rto: float = 0.5

    zaiwang: int = 0
    xia_yi_xuhao: int = 0
    yiqueren: int = -1
    dup_ack_lianxu: int = 0
    kuai_chongchuan: bool = False
    chongchuan_cishu: int = 0

    _duan_liebiao: List[Tuple[bytes, bool]] = field(default_factory=list)
    _dai_fa: int = 0
    _wei_ack: Dict[int, Tuple[bytes, float]] = field(default_factory=dict)
    _shijian: float = 0.0
    _jie_shou: HuachuangTongdao = field(init=False)
    _yingyong_wancheng: Optional[bytes] = field(default=None)

    def __post_init__(self) -> None:
        self._jie_shou = HuachuangTongdao(self.lianlu.qianxiang, self.chuangkou, self.duan_max)

    def _zhongzhi_huihua(self) -> None:
        self.lianlu.qingkong()
        self.zaiwang = 0
        self.xia_yi_xuhao = 0
        self.yiqueren = -1
        self.dup_ack_lianxu = 0
        self.kuai_chongchuan = False
        self._duan_liebiao = []
        self._dai_fa = 0
        self._wei_ack.clear()
        self._shijian = 0.0
        self._jie_shou = HuachuangTongdao(self.lianlu.qianxiang, self.chuangkou, self.duan_max)
        self._yingyong_wancheng = None

    def _keyi_fasong(self) -> bool:
        return self.zaiwang < self.chuangkou

    def _fasong_xin(self) -> None:
        while self._keyi_fasong() and self._dai_fa < len(self._duan_liebiao):
            chunk, fin = self._duan_liebiao[self._dai_fa]
            seq = self.xia_yi_xuhao
            frame = bianma_shuju(seq, chunk, fin)
            self.lianlu.qianxiang.fasong(frame)
            self._wei_ack[seq] = (frame, self._shijian)
            self.xia_yi_xuhao += 1
            self.zaiwang += 1
            self._dai_fa += 1

    def _jieshou_ack(self, ack_num: int) -> None:
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
        for seq in range(ack_num + 1):
            self._wei_ack.pop(seq, None)

    def _chongchuan_zuidi(self) -> None:
        if not self._wei_ack:
            return
        seq = min(self._wei_ack)
        frame, _ = self._wei_ack[seq]
        self.lianlu.qianxiang.fasong(frame)
        self._wei_ack[seq] = (frame, self._shijian)
        self.chongchuan_cishu += 1
        self.kuai_chongchuan = False

    def _chuli_qianxiang(self) -> None:
        while True:
            raw = self.lianlu.qianxiang.jieshou()
            if raw is None:
                break
            parsed = jiexi_bao(raw)
            if not parsed or parsed[0] != "data":
                continue
            _, seq, fin, chunk = parsed
            before = self._jie_shou._next
            self._jie_shou._xishou(seq, fin, chunk)
            after = self._jie_shou._next
            if after > before:
                self.lianlu.huitui.fasong(bianma_ack(after - 1))
            elif seq < after and after > 0:
                self.lianlu.huitui.fasong(bianma_ack(after - 1))
            if self._jie_shou._done is not None:
                self._yingyong_wancheng = self._jie_shou._done

    def _chuli_huitui(self) -> None:
        while True:
            raw = self.lianlu.huitui.jieshou()
            if raw is None:
                break
            parsed = jiexi_bao(raw)
            if not parsed or parsed[0] != "ack":
                continue
            _, ack_num = parsed
            self._jieshou_ack(ack_num)

    def _jiancha_chaoshi(self) -> None:
        if not self._wei_ack:
            return
        seq = min(self._wei_ack)
        _, sent_at = self._wei_ack[seq]
        if self._shijian - sent_at >= self.rto:
            self._chongchuan_zuidi()

    def _tuijin_yibu(self) -> bool:
        self._fasong_xin()
        self._chuli_qianxiang()
        self._chuli_huitui()
        if self.kuai_chongchuan:
            self._chongchuan_zuidi()
        self._jiancha_chaoshi()
        self._shijian += self.rto / 10.0
        if self._yingyong_wancheng is not None and not self._wei_ack:
            return False
        if self._dai_fa < len(self._duan_liebiao) or self._wei_ack:
            return True
        return self._yingyong_wancheng is None

    def chuanshu_kexin(self, shuju: bytes) -> None:
        self._zhongzhi_huihua()
        for i in range(0, len(shuju), self.duan_max):
            chunk = shuju[i : i + self.duan_max]
            fin = i + self.duan_max >= len(shuju)
            self._duan_liebiao.append((chunk, fin))
        guard = 0
        while self._tuijin_yibu():
            guard += 1
            if guard > 50000:
                raise RuntimeError("可靠传输仿真步数超限")

    def jieshou_wanzheng_kexin(self) -> bytes | None:
        return self._yingyong_wancheng
