import socket
import threading
import time
from pathlib import Path

from tests.solution_loader import load_solution


def _load():
    return load_solution(Path(__file__).resolve().parents[1], "taojie_lab.py")


def test_echo_loopback():
    mod = _load()
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind(("127.0.0.1", 0))
        port = s.getsockname()[1]
    lab = mod.TaoJieLab()
    ready = threading.Event()

    def serve():
        ready.set()
        lab.echo_fuwu(port, b"pong")

    t = threading.Thread(target=serve)
    t.start()
    assert ready.wait(timeout=2)
    time.sleep(0.05)
    out = lab.echo_kehu(port, b"ping")
    t.join(timeout=5)
    assert out == b"pong"
