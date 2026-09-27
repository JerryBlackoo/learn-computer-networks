from pathlib import Path

from tests.solution_loader import load_solution


def _load():
    return load_solution(Path(__file__).resolve().parents[1], "cengdie_lab.py")


def test_e2e_registry():
    mod = _load()
    e2e = mod.DuanDaoDuanLunzheng()
    e2e.dengji("可靠交付")
    e2e.dengji("可靠交付")
    assert e2e.shifou_zai_duandian("可靠交付")
    assert not e2e.shifou_zai_duandian("仅缓存")


def test_encapsulation_roundtrip():
    mod = _load()
    zhan = mod.FengzhuangZhan(["L2", "L3", "L4"])
    kuai = zhan.fengzhuang(b"hello")
    assert kuai[0].cengji == "L2"
    assert zhan.jiefengzhuang(kuai) == b"hello"
