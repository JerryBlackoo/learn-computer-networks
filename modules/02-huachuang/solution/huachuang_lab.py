"""模块 02 solution — 分片成帧 + 序号重组（链路无 ACK，见模块 11 扩展可靠性）。"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict, List, Optional, Tuple

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

    _parts: Dict[int, bytes] = field(default_factory=dict)
    _next: int = 0
    _done: Optional[bytes] = field(default=None)

    def chuanshu(self, shuju: bytes) -> None:
        seq = 0
        for i in range(0, len(shuju), self.duan_max):
            chunk = shuju[i : i + self.duan_max]
            fin = i + self.duan_max >= len(shuju)
            self.lianlu.fasong(_frame(seq, chunk, fin))
            seq += 1

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
            if seq == self._next:
                self._parts[seq] = chunk
                self._next += 1
                if fin:
                    self._done = b"".join(self._parts[i] for i in range(self._next))
                    return self._done
        return None
