from pathlib import Path

from tests.solution_loader import load_solution


def _load():
    return load_solution(Path(__file__).resolve().parents[1], "tls_lab.py")


def test_state_machine_order():
    mod = _load()
    sm = mod.TlsZhuangtaiJi()
    sm.kehu_kaishi()
    sm.fuwu_xiangying()
    sm.jinru_yingyong()
    sm.wancheng()
    assert sm.zhuangtai == mod.WoshouZhuangtai.WANCHENG


def test_ssl_context_loads():
    mod = _load()
    ctx = mod.chuangjian_shangxiawen()
    assert ctx is not None
