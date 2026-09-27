"""模块 02 solution — 分片成帧、校验、固定窗口内乱序重组。"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict, Optional, Tuple

from wangluo_xuejing.kexin_lianlu import KexinLianlu
from wangluo_xuejing.xinxi import xor_checksum


def _frame(seq: int, chunk: bytes, fin: bool) -> bytes:
    flag = b"F" if fin else b"C"
    body = bytes([seq & 0xFF]) + flag + chunk
    return body + bytes([xor_checksum(body)])


def _parse(frame: bytes) -> Optional[Tuple[int, bool, bytes]]:
    if len(frame) < 3:
        return None
    body, cs = frame[:-1], frame[-1]
    if xor_checksum(body) != cs:
        return None
    seq = body[0]
    fin = body[1:2] == b"F"
    return seq, fin, body[2:]


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
