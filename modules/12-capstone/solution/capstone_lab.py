from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

from wangluo_xuejing.kexin_lianlu import KexinLianlu
from wangluo_xuejing.lab_loader import load_lab


def _huachuang_cls():
    mod = load_lab(
        Path(__file__).resolve().parents[2] / "02-huachuang",
        "huachuang_lab.py",
    )
    return mod.HuachuangTongdao


def _chuanshu(lianlu: KexinLianlu, shuju: bytes) -> None:
    _huachuang_cls()(lianlu).chuanshu(shuju)


def _jieshou(lianlu: KexinLianlu) -> Optional[bytes]:
    return _huachuang_cls()(lianlu).jieshou_wanzheng()


@dataclass
class XinyunTongdao:
    lianlu: KexinLianlu

    def fasong(self, shuju: bytes) -> None:
        _chuanshu(self.lianlu, shuju)

    def jieshou(self) -> Optional[bytes]:
        return _jieshou(self.lianlu)


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
        self.tongdao.fasong(resp)
        out = self.tongdao.jieshou()
        return out or b""
