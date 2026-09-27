from __future__ import annotations

import ssl
from dataclasses import dataclass, field
from enum import Enum, auto
from pathlib import Path


class WoshouZhuangtai(Enum):
    KONGXIAN = auto()
    KEHU_HELLO = auto()
    FUWU_HELLO = auto()
    YINGYONG_SHUJU = auto()
    WANCHENG = auto()


@dataclass
class TlsZhuangtaiJi:
    zhuangtai: WoshouZhuangtai = WoshouZhuangtai.KONGXIAN
    rizhi: list[str] = field(default_factory=list)

    def kehu_kaishi(self) -> None:
        self.zhuangtai = WoshouZhuangtai.KEHU_HELLO
        self.rizhi.append("C:ClientHello")

    def fuwu_xiangying(self) -> None:
        if self.zhuangtai != WoshouZhuangtai.KEHU_HELLO:
            raise RuntimeError("顺序错误")
        self.zhuangtai = WoshouZhuangtai.FUWU_HELLO
        self.rizhi.append("S:ServerHello")

    def jinru_yingyong(self) -> None:
        self.zhuangtai = WoshouZhuangtai.YINGYONG_SHUJU
        self.rizhi.append("加密应用数据")

    def wancheng(self) -> None:
        self.zhuangtai = WoshouZhuangtai.WANCHENG


def jiazai_zhengshu_lujing() -> Path:
    return Path(__file__).resolve().parents[1] / "fixtures" / "selfsigned.pem"


def chuangjian_shangxiawen() -> ssl.SSLContext:
    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    pem = jiazai_zhengshu_lujing()
    ctx.load_cert_chain(certfile=str(pem))
    return ctx
