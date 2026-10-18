from __future__ import annotations

import json
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional

from network_simulation.lab_loader import load_lab
from network_simulation.shuangxiang_lianlu import ShuangxiangLianlu


def _kexin_cls():
    mod = load_lab(
        Path(__file__).resolve().parents[2] / "02-huachuang",
        "huachuang_lab.py",
    )
    return mod.HuachuangKexinTongdao


@dataclass
class XinyunTongdao:
    lianlu: ShuangxiangLianlu
    chuangkou: int = 4
    duan_max: int = 32
    _kexin: object = field(default=None, init=False, repr=False)

    def fasong(self, shuju: bytes) -> None:
        self._kexin = _kexin_cls()(self.lianlu, self.chuangkou, self.duan_max)
        self._kexin.chuanshu_kexin(shuju)

    def jieshou(self) -> Optional[bytes]:
        if self._kexin is None:
            return None
        return self._kexin.jieshou_wanzheng_kexin()


@dataclass
class XinxiYingyong:
    tongdao: XinyunTongdao

    def chuli_qingqiu(self, qingqiu: bytes) -> bytes:
        line = qingqiu.decode("utf-8", errors="replace").split("\r\n", 1)[0]
        if "GET /xinxi" in line:
            body = json.dumps({"fuwu": "xinyun", "zhuangtai": "ok"}, ensure_ascii=False)
            return f"HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{body}".encode()
        return b"HTTP/1.1 404 Not Found\r\n\r\n"

    def yici_jiaohu(self, qingqiu: bytes) -> bytes:
        self.tongdao.fasong(qingqiu)
        raw = self.tongdao.jieshou()
        if raw is None:
            raise RuntimeError("未收到请求")
        resp = self.chuli_qingqiu(raw)
        self.tongdao.lianlu.qingkong()
        self.tongdao.fasong(resp)
        out = self.tongdao.jieshou()
        return out or b""
