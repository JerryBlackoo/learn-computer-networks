"""成帧与 ACK 包格式（教学简化，非以太网 FCS / 非 RFC TCP 头）。"""

from __future__ import annotations

from typing import Literal, Optional, Tuple, Union

from wangluo_xuejing.xinxi import xor_checksum

AckBao = Tuple[Literal["ack"], int]
ShujuBao = Tuple[Literal["data"], int, bool, bytes]
JieXiJieGuo = Union[AckBao, ShujuBao]


def bianma_shuju(seq: int, chunk: bytes, fin: bool) -> bytes:
    flag = b"F" if fin else b"C"
    body = bytes([seq & 0xFF]) + flag + chunk
    return body + bytes([xor_checksum(body)])


def bianma_ack(yiqueren_xuhao: int) -> bytes:
    """累积 ACK：序号 ≤ yiqueren_xuhao 的数据均已按序收到。"""
    body = b"A" + bytes([yiqueren_xuhao & 0xFF])
    return body + bytes([xor_checksum(body)])


def jiexi_bao(frame: bytes) -> Optional[JieXiJieGuo]:
    if len(frame) < 3:
        return None
    body, cs = frame[:-1], frame[-1]
    if xor_checksum(body) != cs:
        return None
    if body[:1] == b"A":
        if len(body) != 2:
            return None
        return ("ack", body[1])
    if len(body) < 2:
        return None
    seq = body[0]
    fin = body[1:2] == b"F"
    return ("data", seq, fin, body[2:])
