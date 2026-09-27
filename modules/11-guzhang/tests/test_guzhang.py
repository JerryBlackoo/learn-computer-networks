from pathlib import Path

from tests.solution_loader import load_solution


def _load():
    return load_solution(Path(__file__).resolve().parents[1], "guzhang_lab.py")


def test_failure_modes():
    mod = _load()
    m = mod.GuzhangBowuguan()
    assert m.fenlei_diubao() == "diubao"
    assert m.fenlei_fenqu() == "fenqu"
    assert m.fenlei_chongpai() == "chongpai"
    assert m.fenlei_chongfu() == "chongfu"
    assert "diubao" in m.liebiao_anli()


def test_zhengti_baogao():
    mod = _load()
    rep = mod.GuzhangBowuguan().zhengti_baogao()
    assert rep["diubao"] == "diubao"
    assert rep["fenqu"] == "fenqu"
