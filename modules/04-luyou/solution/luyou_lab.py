from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict, List, Tuple


@dataclass
class JuliShiliang:
    jiedian: str
    linju_daijuli: Dict[str, int] = field(default_factory=dict)
    juli_biao: Dict[str, int] = field(default_factory=dict)

    def chushihua(self, quan_jiedian: List[str]) -> None:
        for n in quan_jiedian:
            self.juli_biao[n] = 0 if n == self.jiedian else 10_000
        for linju, cost in self.linju_daijuli.items():
            self.juli_biao[linju] = min(self.juli_biao.get(linju, 10_000), cost)

    def jiaohuan_yilun(self, linju_biao: Dict[str, Dict[str, int]]) -> None:
        for linju, remote in linju_biao.items():
            if linju not in self.linju_daijuli:
                continue
            direct = self.linju_daijuli[linju]
            for mudi, d in remote.items():
                cand = direct + d
                if cand < self.juli_biao.get(mudi, 10_000):
                    self.juli_biao[mudi] = cand


@dataclass
class LianluZhuangtai:
    @staticmethod
    def zuiduan_lujing(
        yuan: str, tuopu: Dict[str, List[Tuple[str, int]]]
    ) -> Dict[str, int]:
        dist: Dict[str, int] = {yuan: 0}
        pq = [(0, yuan)]
        visited = set()
        while pq:
            pq.sort(key=lambda x: x[0])
            d, u = pq.pop(0)
            if u in visited:
                continue
            visited.add(u)
            for v, w in tuopu.get(u, []):
                nd = d + w
                if nd < dist.get(v, 10_000):
                    dist[v] = nd
                    pq.append((nd, v))
        return dist


def juli_shouling(jiedian_lie: List[JuliShiliang], max_lun: int = 32) -> int:
    """多轮邻居表交换，直到全网距离向量不再变化（小图教学用）。"""
    for lun in range(1, max_lun + 1):
        kuai_zhao = {j.jiedian: dict(j.juli_biao) for j in jiedian_lie}
        for j in jiedian_lie:
            j.jiaohuan_yilun(kuai_zhao)
        xin_zhao = {j.jiedian: dict(j.juli_biao) for j in jiedian_lie}
        if xin_zhao == kuai_zhao:
            return lun
    return max_lun
