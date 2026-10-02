from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, List, Mapping, MutableMapping, Sequence

# 计划第 11 节：症状 → 候选协议层（诊断假设，非最终裁决）
ZHENGZAI_DAO_CENG: Dict[str, List[str]] = {
    "shangchuan_bianman": ["chuanshu", "lianlu", "yingyong"],
    "liushi_kadun": ["yingyong", "chuanshu", "lianjie"],
    "lianjie_chongzhi": ["lianjie", "chuanshu", "zhongjianhe"],
}

ZHENGJU_BIXU = ("baowen", "taojie", "jincheng", "qingqiu")

# 教学模块与 trace 字段的对照提示（不要求与内核 TCP 逐字段一致）
JIAOXUE_DUIZHAO_TISHI = {
    "模块02_重传": "trace 中 retrans 为真的事件对应可靠层重传直觉",
    "模块05_RTO": "rtt_guji 与 chongchuan_shu 可对照 RTO 与 AIMD 讨论",
    "模块10_测量": "SYN/SYN-ACK 时间差用于 RTT 估计（教学简化）",
}


@dataclass(frozen=True)
class QingqiuLianJieduan:
    """双服务之间一次 HTTPS 请求链的分段占位（由学习者在 not-run 实验中填时间）。"""

    dns_ms: float | None
    tcp_ms: float | None
    tls_ms: float | None
    http_shoubao_ms: float | None

    def wanzheng(self) -> bool:
        return all(
            v is not None and v >= 0
            for v in (self.dns_ms, self.tcp_ms, self.tls_ms, self.http_shoubao_ms)
        )


@dataclass
class ZhenDuanBaogao:
    anli_id: str
    zhengzai: str
    zhengju: Dict[str, str]
    trace_tezheng: Dict[str, Any]
    yixuan_pingju_ceng: str
    weizhi_neirong: List[str]
    jiaoxue_duizhao: Dict[str, str]
    quekou: List[str]


class ZhenDuanGongju:
    """真实协议诊断工作流（CI 内用合成案例；抓包仅在学习者本机 not-run 完成）。"""

    def zhengzai_houxuan_ceng(self, zhengzai: str) -> List[str]:
        raise NotImplementedError

    def zhengjulian_quekou(self, zhengju: Mapping[str, str]) -> List[str]:
        raise NotImplementedError

    def duizhao_jiaoxue_trace(
        self, shijian_xian: Sequence[Mapping[str, Any]]
    ) -> Dict[str, Any]:
        raise NotImplementedError

    def cong_anli_json(self, path: Path) -> ZhenDuanBaogao:
        raise NotImplementedError

    def moban_biyelun(self) -> Dict[str, Any]:
        raise NotImplementedError

    @staticmethod
    def jiaoxue_tishi() -> Dict[str, str]:
        return dict(JIAOXUE_DUIZHAO_TISHI)
