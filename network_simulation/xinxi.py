"""共享数据结构与校验工具。"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable


@dataclass(frozen=True)
class FengzhuangKuai:
    """分层教学用的载荷块。"""

    cengji: str
    zaihe: bytes


def hex_dump(data: bytes, max_len: int = 64) -> str:
    snippet = data[:max_len]
    return snippet.hex()


def xor_checksum(data: bytes) -> int:
    """教学用简单校验和（非 CRC32/FCS）。"""
    acc = 0
    for b in data:
        acc ^= b
    return acc & 0xFF


def validate_non_empty(labels: Iterable[str]) -> None:
    for label in labels:
        if not label or not label.strip():
            raise ValueError("标签不能为空")
