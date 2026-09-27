from pathlib import Path

from tests.solution_loader import load_solution


def _load():
    return load_solution(Path(__file__).resolve().parents[1], "luyou_lab.py")


def test_dijkstra():
    mod = _load()
    topo = {
        "A": [("B", 1), ("C", 4)],
        "B": [("C", 2), ("D", 5)],
        "C": [("D", 1)],
        "D": [],
    }
    d = mod.LianluZhuangtai.zuiduan_lujing("A", topo)
    assert d["D"] == 4


def test_distance_vector_round():
    mod = _load()
    a = mod.JuliShiliang("A", {"B": 1})
    a.chushihua(["A", "B", "C"])
    b = mod.JuliShiliang("B", {"A": 1, "C": 1})
    b.chushihua(["A", "B", "C"])
    c = mod.JuliShiliang("C", {"B": 1})
    c.chushihua(["A", "B", "C"])
    biao = {"A": a.juli_biao, "B": b.juli_biao, "C": c.juli_biao}
    a.jiaohuan_yilun(biao)
    assert a.juli_biao["C"] <= 2


def test_dv_convergence_triangle():
    mod = _load()
    nodes = ["A", "B", "C", "D"]
    a = mod.JuliShiliang("A", {"B": 1})
    b = mod.JuliShiliang("B", {"A": 1, "C": 1})
    c = mod.JuliShiliang("C", {"B": 1, "D": 1})
    d = mod.JuliShiliang("D", {"C": 1})
    for j in (a, b, c, d):
        j.chushihua(nodes)
    lun = mod.juli_shouling([a, b, c, d])
    assert lun >= 1
    assert a.juli_biao["D"] == 3
    assert d.juli_biao["A"] == 3


def test_ls_negative_cycle_not_in_graph():
    mod = _load()
    topo = {"A": [("B", 1)], "B": []}
    d = mod.LianluZhuangtai.zuiduan_lujing("A", topo)
    assert d["B"] == 1
    assert "C" not in d
