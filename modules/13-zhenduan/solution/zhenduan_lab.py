from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, List, Mapping, MutableMapping, Sequence

ZHENGZAI_DAO_CENG: Dict[str, List[str]] = {
    "shangchuan_bianman": ["chuanshu", "lianlu", "yingyong"],
    "liushi_kadun": ["yingyong", "chuanshu", "lianjie"],
    "lianjie_chongzhi": ["lianjie", "chuanshu", "zhongjianhe"],
}

ZHENGJU_BIXU = ("baowen", "taojie", "jincheng", "qingqiu")

JIAOXUE_DUIZHAO_TISHI = {
    "模块02_重传": "trace 中 retrans 为真的事件对应可靠层重传直觉",
    "模块05_RTO": "rtt_guji 与 chongchuan_shu 可对照 RTO 与 AIMD 讨论",
    "模块10_测量": "SYN/SYN-ACK 时间差用于 RTT 估计（教学简化）",
}


@dataclass(frozen=True)
class QingqiuLianJieduan:
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
    def zhengzai_houxuan_ceng(self, zhengzai: str) -> List[str]:
        return list(ZHENGZAI_DAO_CENG.get(zhengzai, []))

    def zhengjulian_quekou(self, zhengju: Mapping[str, str]) -> List[str]:
        quekou: List[str] = []
        for key in ZHENGJU_BIXU:
            val = zhengju.get(key, "")
            if not str(val).strip():
                quekou.append(key)
        return quekou

    def _rtt_cong_syn(self, shijian_xian: Sequence[Mapping[str, Any]]) -> float | None:
        syn = next(
            (
                e
                for e in shijian_xian
                if "SYN" in e.get("flags", []) and e.get("dir") == "c2s"
            ),
            None,
        )
        synack = next(
            (
                e
                for e in shijian_xian
                if "SYN" in e.get("flags", [])
                and "ACK" in e.get("flags", [])
                and e.get("dir") == "s2c"
            ),
            None,
        )
        if not syn or not synack:
            return None
        return float(synack["t"] - syn["t"])

    def duizhao_jiaoxue_trace(
        self, shijian_xian: Sequence[Mapping[str, Any]]
    ) -> Dict[str, Any]:
        rtt = self._rtt_cong_syn(shijian_xian)
        chongchuan = sum(1 for e in shijian_xian if e.get("retrans"))
        shuju = sum(
            1
            for e in shijian_xian
            if e.get("len", 0) > 0
            and e.get("dir") == "c2s"
            and not e.get("retrans")
            and "SYN" not in e.get("flags", [])
        )
        yixuan: List[str] = []
        weizhi: List[str] = []
        if chongchuan >= 3:
            yixuan.append("chuanshu")
        elif chongchuan > 0:
            yixuan.append("chuanshu")
            weizhi.append("重传次数较少，需结合 RTT 与窗口判断是否为偶发")
        if rtt is not None and rtt > 0.05:
            yixuan.append("lianlu")
        if not yixuan:
            weizhi.append("仅凭合成 trace 无法排除应用层背压")
        return {
            "rtt_guji": rtt,
            "chongchuan_shu": chongchuan,
            "shuju_bao_shu": shuju,
            "jiaoxue_duizhao": dict(JIAOXUE_DUIZHAO_TISHI),
            "yixuan_ceng": yixuan,
            "weizhi_neirong": weizhi,
        }

    def cong_anli_json(self, path: Path) -> ZhenDuanBaogao:
        raw = json.loads(path.read_text(encoding="utf-8"))
        zhengju = dict(raw["zhengju"])
        quekou = self.zhengjulian_quekou(zhengju)
        return ZhenDuanBaogao(
            anli_id=raw["anli_id"],
            zhengzai=raw["zhengzai"],
            zhengju=zhengju,
            trace_tezheng=dict(raw["trace_tezheng"]),
            yixuan_pingju_ceng=raw["yixuan_pingju_ceng"],
            weizhi_neirong=list(raw["weizhi_neirong"]),
            jiaoxue_duizhao=dict(JIAOXUE_DUIZHAO_TISHI),
            quekou=quekou,
        )

    def moban_biyelun(self) -> Dict[str, Any]:
        return {
            "biaoti": "网络诊断报告（毕业门）",
            "huanjing": "固定 OS、工具版本、两端服务版本与监听地址",
            "qingqiu_lian": {
                "dns_ms": None,
                "tcp_ms": None,
                "tls_ms": None,
                "http_shoubao_ms": None,
            },
            "anli": [
                "shangchuan_bianman",
                "liushi_kadun",
                "lianjie_chongzhi",
            ],
            "zhengjulian": {k: "" for k in ZHENGJU_BIXU},
            "yixuan_pingju_ceng": "",
            "weizhi_neirong": [],
            "jiaoxue_duizhao": dict(JIAOXUE_DUIZHAO_TISHI),
        }

    @staticmethod
    def jiaoxue_tishi() -> Dict[str, str]:
        return dict(JIAOXUE_DUIZHAO_TISHI)
