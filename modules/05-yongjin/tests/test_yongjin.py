from pathlib import Path

from tests.solution_loader import load_solution


def _load():
    return load_solution(Path(__file__).resolve().parents[1], "yongjin_lab.py")


def test_slow_start_then_loss():
    mod = _load()
    m = mod.YongjinMoxing(ssthresh=4.0)
    for _ in range(5):
        m.shijian_bu(1, loss=False)
    assert m.cwnd >= 4.0
    m.shijian_bu(0, loss=True)
    assert m.cwnd == 1.0
