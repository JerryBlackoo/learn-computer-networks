from pathlib import Path

from tests.solution_loader import load_solution


def _load():
    return load_solution(Path(__file__).resolve().parents[1], "guzhang_lab.py")


def test_failure_modes():
    mod = _load()
    m = mod.GuzhangBowuguan()
    assert m.fenlei_diubao() == "diubao"
    assert m.fenlei_fenqu() == "fenqu"
    assert "diubao" in m.liebiao_anli()
