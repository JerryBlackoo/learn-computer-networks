from pathlib import Path

from tests.solution_loader import load_solution
from wangluo_xuejing.kexin_lianlu import KexinLianlu, LianluCanshu
from wangluo_xuejing.shuangxiang_lianlu import ShuangxiangLianlu


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


def test_kexin_lossless_roundtrip():
    mod = _load()
    link = ShuangxiangLianlu.wuwuha()
    kx = mod.HuachuangKexinTongdao(link, duan_max=8, chuangkou=3)
    payload = b"kexin-chuan-" * 5
    kx.chuanshu_kexin(payload)
    assert kx.jieshou_wanzheng_kexin() == payload
    assert kx.chongchuan_cishu == 0


def test_kexin_recovers_from_loss():
    mod = _load()
    link = ShuangxiangLianlu.fenli_guzhang(LianluCanshu(diubao_gailv=0.35, seed=21))
    kx = mod.HuachuangKexinTongdao(link, duan_max=6, chuangkou=2, rto=0.2)
    payload = b"NETLAB-" * 12
    kx.chuanshu_kexin(payload)
    assert kx.jieshou_wanzheng_kexin() == payload
    assert kx.chongchuan_cishu >= 1


def test_kexin_sliding_window_caps_inflight():
    mod = _load()
    link = ShuangxiangLianlu.wuwuha()
    kx = mod.HuachuangKexinTongdao(link, duan_max=4, chuangkou=2)
    payload = b"abcdefgh" * 2
    kx._zhongzhi_huihua()
    for i in range(0, len(payload), kx.duan_max):
        chunk = payload[i : i + kx.duan_max]
        fin = i + kx.duan_max >= len(payload)
        kx._duan_liebiao.append((chunk, fin))
    max_zaiwang = 0
    for _ in range(200):
        kx._tuijin_yibu()
        max_zaiwang = max(max_zaiwang, kx.zaiwang)
        if kx.jieshou_wanzheng_kexin() == payload:
            break
    assert max_zaiwang <= 2
