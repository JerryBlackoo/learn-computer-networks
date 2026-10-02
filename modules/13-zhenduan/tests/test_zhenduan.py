from pathlib import Path

from tests.solution_loader import load_solution


def _load():
    return load_solution(Path(__file__).resolve().parents[1], "zhenduan_lab.py")


def test_zhengzai_houxuan_ceng():
    gongju = _load().ZhenDuanGongju()
    assert gongju.zhengzai_houxuan_ceng("lianjie_chongzhi") == [
        "lianjie",
        "chuanshu",
        "zhongjianhe",
    ]


def test_zhengjulian_quekou():
    gongju = _load().ZhenDuanGongju()
    assert gongju.zhengjulian_quekou({"baowen": "x", "taojie": "", "jincheng": "y", "qingqiu": "z"}) == [
        "taojie",
    ]


def test_duizhao_jiaoxue_trace_reuses_module10_shape():
    mod = _load()
    trace_path = Path(__file__).resolve().parents[2] / "10-celiang" / "fixtures" / "synthetic_trace.json"
    import json

    shijian = json.loads(trace_path.read_text(encoding="utf-8"))
    duizhao = mod.ZhenDuanGongju().duizhao_jiaoxue_trace(shijian)
    assert abs(duizhao["rtt_guji"] - 0.01) < 1e-6
    assert duizhao["chongchuan_shu"] == 1
    assert duizhao["shuju_bao_shu"] == 2
    assert "chuanshu" in duizhao["yixuan_ceng"] or duizhao["chongchuan_shu"] > 0


def test_anli_fixtures_load():
    base = Path(__file__).resolve().parents[1] / "fixtures"
    gongju = _load().ZhenDuanGongju()
    for name in (
        "anli_shangchuan_bianman.json",
        "anli_liushi_kadun.json",
        "anli_lianjie_chongzhi.json",
    ):
        baogao = gongju.cong_anli_json(base / name)
        assert baogao.quekou == []
        assert baogao.yixuan_pingju_ceng
        assert baogao.weizhi_neirong


def test_moban_biyelun_has_three_cases():
    moban = _load().ZhenDuanGongju().moban_biyelun()
    assert len(moban["anli"]) == 3
    assert set(moban["zhengjulian"]) == {"baowen", "taojie", "jincheng", "qingqiu"}
