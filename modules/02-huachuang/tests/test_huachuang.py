from pathlib import Path

from tests.solution_loader import load_solution
from wangluo_xuejing.kexin_lianlu import KexinLianlu, LianluCanshu


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


def test_reorder_within_window():
    mod = _load()
    link = KexinLianlu(LianluCanshu(chongpai_gailv=1.0, seed=11))
    hc = mod.HuachuangTongdao(link, duan_max=4, chuangkou=4)
    payload = b"abcd" * 4
    hc.chuanshu(payload)
    out = hc.jieshou_wanzheng()
    assert out == payload


def test_bad_checksum_dropped():
    mod = _load()
    link = KexinLianlu()
    hc = mod.HuachuangTongdao(link, duan_max=16)
    link.fasong(b"\x00Cok\xff")
    assert hc.jieshou_wanzheng() is None
