from __future__ import annotations

from dataclasses import dataclass
from typing import List

from network_simulation.kexin_lianlu import KexinLianlu, LianluCanshu


@dataclass
class GuzhangBowuguan:
    """对不可靠链路注入故障并分类现象。"""

    def fenlei_diubao(self, seed: int = 1) -> str:
        link = KexinLianlu(LianluCanshu(diubao_gailv=1.0, seed=seed))
        link.fasong(b"x")
        return "diubao" if link.jieshou() is None else "qita"

    def fenlei_chongpai(self, seed: int = 7) -> str:
        link = KexinLianlu(LianluCanshu(chongpai_gailv=1.0, seed=seed))
        link.fasong(b"a")
        link.fasong(b"b")
        first = link.jieshou()
        second = link.jieshou()
        if first == b"b" and second == b"a":
            return "chongpai"
        return "zhengxu"

    def fenlei_chongfu(self, seed: int = 3) -> str:
        link = KexinLianlu(LianluCanshu(chongfu_gailv=1.0, seed=seed))
        link.fasong(b"x")
        a = link.jieshou()
        b = link.jieshou()
        if a == b"x" and b == b"x":
            return "chongfu"
        return "qita"

    def fenlei_fenqu(self) -> str:
        link = KexinLianlu(LianluCanshu(fenqu=True))
        link.fasong(b"x")
        return "fenqu" if link.jieshou() is None else "lianjie"

    def zhengti_baogao(self) -> dict[str, str]:
        return {
            "diubao": self.fenlei_diubao(),
            "chongpai": self.fenlei_chongpai(),
            "chongfu": self.fenlei_chongfu(),
            "fenqu": self.fenlei_fenqu(),
        }

    def liebiao_anli(self) -> List[str]:
        return ["diubao", "chongpai", "chongfu", "fenqu"]
