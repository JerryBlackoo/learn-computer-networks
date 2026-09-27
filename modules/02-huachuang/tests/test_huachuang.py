from pathlib import Path

from tests.solution_loader import load_solution
from wangluo_xuejing.kexin_lianlu import KexinLianlu


def _load():
    return load_solution(Path(__file__).resolve().parents[1], "huachuang_lab.py")


def test_segment_reassembly():
    mod = _load()
    link = KexinLianlu()
    hc = mod.HuachuangTongdao(link, duan_max=8)
    payload = b"wangluo-xuejing-" * 3
    hc.chuanshu(payload)
    out = hc.jieshou_wanzheng()
    assert out == payload
