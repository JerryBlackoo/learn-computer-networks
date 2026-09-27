from pathlib import Path

from tests.solution_loader import load_solution


def _load():
    return load_solution(Path(__file__).resolve().parents[1], "yingyong_lab.py")


def test_dns_http():
    mod = _load()
    dns = mod.BenDiYuming()
    dns.tianjia("xuejing.local", "127.0.0.1")
    assert dns.chaxun_a("xuejing.local") == "127.0.0.1"
    http = mod.JianYiHttp()
    req = http.goujian_qingqiu("get", "/xinxi", "xuejing.local")
    assert b"GET /xinxi" in req
    code = http.jiexi_zhuangtai(b"HTTP/1.1 200 OK\r\n\r\n")
    assert code == 200
