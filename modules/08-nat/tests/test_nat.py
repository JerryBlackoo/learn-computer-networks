from pathlib import Path

from tests.solution_loader import load_solution


def _load():
    return load_solution(Path(__file__).resolve().parents[1], "nat_lab.py")


def test_napt():
    mod = _load()
    nat = mod.ZhongjianHe("203.0.113.1")
    pub = nat.chu_kou("10.0.0.2", 5000)
    assert pub[0] == "203.0.113.1"
    assert nat.ru_kou(pub[1]) == ("10.0.0.2", 5000)
