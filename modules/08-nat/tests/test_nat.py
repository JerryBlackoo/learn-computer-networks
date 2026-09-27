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


def test_stable_mapping():
    mod = _load()
    nat = mod.ZhongjianHe("203.0.113.1")
    first = nat.chu_kou("10.0.0.2", 5000)
    second = nat.chu_kou("10.0.0.2", 5000)
    assert first == second


def test_two_hosts_distinct_ports():
    mod = _load()
    nat = mod.ZhongjianHe("203.0.113.1")
    a = nat.chu_kou("10.0.0.2", 5000)
    b = nat.chu_kou("10.0.0.3", 5000)
    assert a[1] != b[1]
    assert nat.ru_kou(a[1]) == ("10.0.0.2", 5000)
    assert nat.ru_kou(b[1]) == ("10.0.0.3", 5000)
