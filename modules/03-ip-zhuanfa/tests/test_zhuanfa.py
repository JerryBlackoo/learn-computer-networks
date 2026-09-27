from pathlib import Path

from tests.solution_loader import load_solution


def _load():
    return load_solution(Path(__file__).resolve().parents[1], "zhuanfa_lab.py")


def test_lpm():
    mod = _load()
    tbl = mod.MoNiZhuanfa(
        [
            mod.ZhuanfaXiang("10.0.0.0/8", 8, "r1"),
            mod.ZhuanfaXiang("10.1.0.0/16", 16, "r2"),
            mod.ZhuanfaXiang("192.168.1.0/24", 24, "lan"),
        ]
    )
    assert tbl.chazhao("10.1.2.3") == "r2"
    assert tbl.chazhao("10.9.0.1") == "r1"
    assert tbl.chazhao("192.168.1.50") == "lan"


def test_default_route():
    mod = _load()
    tbl = mod.MoNiZhuanfa(
        [
            mod.ZhuanfaXiang("10.0.0.0/8", 8, "inside"),
            mod.ZhuanfaXiang("0.0.0.0/0", 0, "default-gw"),
        ]
    )
    assert tbl.chazhao("8.8.8.8") == "default-gw"
    assert tbl.chazhao("10.2.3.4") == "inside"


def test_host_route_beats_shorter_aggregate():
    mod = _load()
    tbl = mod.MoNiZhuanfa(
        [
            mod.ZhuanfaXiang("192.168.0.0/16", 16, "vlan"),
            mod.ZhuanfaXiang("192.168.1.5/32", 32, "host-gw"),
        ]
    )
    assert tbl.chazhao("192.168.1.5") == "host-gw"
    assert tbl.chazhao("192.168.1.6") == "vlan"


def test_no_match():
    mod = _load()
    tbl = mod.MoNiZhuanfa([mod.ZhuanfaXiang("10.0.0.0/8", 8, "r1")])
    assert tbl.chazhao("172.16.0.1") is None
