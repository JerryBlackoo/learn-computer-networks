import json
from pathlib import Path

from tests.solution_loader import load_solution
from network_simulation.kexin_lianlu import LianluCanshu
from network_simulation.shuangxiang_lianlu import ShuangxiangLianlu


def _load():
    return load_solution(Path(__file__).resolve().parents[1], "capstone_lab.py")


def test_xinxi_roundtrip():
    mod = _load()
    link = ShuangxiangLianlu.wuwuha()
    app = mod.XinxiYingyong(mod.XinyunTongdao(link))
    req = b"GET /xinxi HTTP/1.1\r\nHost: local\r\n\r\n"
    resp = app.yici_jiaohu(req)
    assert b"200 OK" in resp
    body = resp.split(b"\r\n\r\n", 1)[1]
    data = json.loads(body.decode())
    assert data["zhuangtai"] == "ok"


def test_roundtrip_with_forward_loss():
    mod = _load()
    link = ShuangxiangLianlu.fenli_guzhang(LianluCanshu(diubao_gailv=0.25, seed=42))
    app = mod.XinxiYingyong(mod.XinyunTongdao(link, duan_max=16, chuangkou=2))
    req = b"GET /xinxi HTTP/1.1\r\nHost: local\r\n\r\n"
    resp = app.yici_jiaohu(req)
    assert b"200 OK" in resp


def test_unknown_path_404():
    mod = _load()
    link = ShuangxiangLianlu.wuwuha()
    app = mod.XinxiYingyong(mod.XinyunTongdao(link))
    req = b"GET /weizhi HTTP/1.1\r\nHost: local\r\n\r\n"
    resp = app.yici_jiaohu(req)
    assert b"404" in resp
