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


def test_window_and_ack():
    mod = _load()
    t = mod.ChuanShuJiaoxue(chuangkou=2)
    assert t.keyi_fasong()
    t.fasong_bao()
    t.fasong_bao()
    assert not t.keyi_fasong()
    t.jieshou_ack(0)
    assert t.keyi_fasong()
    assert t.zaiwang == 1


def test_dup_ack_fast_retransmit():
    mod = _load()
    t = mod.ChuanShuJiaoxue()
    t.jieshou_ack(0)
    for _ in range(3):
        t.jieshou_ack(0)
    assert t.kuai_chongchuan


def test_rto_from_rtt():
    mod = _load()
    t = mod.ChuanShuJiaoxue()
    t.gengxin_rtt(0.05)
    assert t.rto == 0.2
    assert t.chaoshi(0.19) is False
    assert t.chaoshi(0.21) is True
