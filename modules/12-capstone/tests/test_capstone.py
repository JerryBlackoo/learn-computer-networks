import json
from pathlib import Path

from tests.solution_loader import load_solution
from wangluo_xuejing.kexin_lianlu import KexinLianlu


def _load():
    return load_solution(Path(__file__).resolve().parents[1], "capstone_lab.py")


def test_xinxi_roundtrip():
    mod = _load()
    link = KexinLianlu()
    app = mod.XinxiYingyong(mod.XinyunTongdao(link))
    req = b"GET /xinxi HTTP/1.1\r\nHost: local\r\n\r\n"
    resp = app.yici_jiaohu(req)
    assert b"200 OK" in resp
    body = resp.split(b"\r\n\r\n", 1)[1]
    data = json.loads(body.decode())
    assert data["zhuangtai"] == "ok"
