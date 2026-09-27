"""模块 01 starter — 请补全 TODO。"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import List

from wangluo_xuejing.xinxi import FengzhuangKuai


@dataclass
class DuanDaoDuanLunzheng:
    """端到端论证：登记必须在端点保证的语义。"""

    yuansu: List[str] = field(default_factory=list)

    def dengji(self, yuansu_ming: str) -> None:
        # TODO: 去重登记
        raise NotImplementedError

    def shifou_zai_duandian(self, yuansu_ming: str) -> bool:
        # TODO
        raise NotImplementedError


@dataclass
class FengzhuangZhan:
    """分层封装站：cengxu 给出从下到上的层名顺序。"""

    cengxu: List[str]

    def fengzhuang(self, zaihe: bytes) -> List[FengzhuangKuai]:
        # TODO: 每层包裹上一层字节串
        raise NotImplementedError

    def jiefengzhuang(self, kuai_lie: List[FengzhuangKuai]) -> bytes:
        # TODO: 校验层序并还原最内层载荷
        raise NotImplementedError
