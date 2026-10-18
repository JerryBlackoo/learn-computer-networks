"""双向内存信道：前向承载数据帧，回传承载 ACK（仿真，非真实 NIC）。"""

from __future__ import annotations

from dataclasses import dataclass, field

from network_simulation.kexin_lianlu import KexinLianlu, LianluCanshu


@dataclass
class ShuangxiangLianlu:
    """数据 `qianxiang` + ACK `huitui`，各自可独立注入故障参数。"""

    qianxiang: KexinLianlu = field(default_factory=KexinLianlu)
    huitui: KexinLianlu = field(default_factory=KexinLianlu)

    @classmethod
    def wuwuha(cls) -> ShuangxiangLianlu:
        return cls()

    @classmethod
    def fenli_guzhang(
        cls,
        qian: LianluCanshu,
        hui: LianluCanshu | None = None,
    ) -> ShuangxiangLianlu:
        hui = hui or LianluCanshu()
        return cls(KexinLianlu(qian), KexinLianlu(hui))

    def qingkong(self) -> None:
        self.qianxiang.qingkong()
        self.huitui.qingkong()
