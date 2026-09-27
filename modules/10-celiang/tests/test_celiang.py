from pathlib import Path

from tests.solution_loader import load_solution


def _load():
    return load_solution(Path(__file__).resolve().parents[1], "celiang_lab.py")


def test_trace_metrics():
    mod = _load()
    trace = mod.CeLiangJiao.cong_json(
        Path(__file__).resolve().parents[1] / "fixtures" / "synthetic_trace.json"
    )
    assert abs(trace.rtt_guji() - 0.01) < 1e-6
    assert trace.shuju_bao_shu() == 2
    assert trace.chongchuan_bao_shu() == 1
    assert trace.pingjun_shuju_daxiao() == 100.0
