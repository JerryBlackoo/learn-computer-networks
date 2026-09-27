from __future__ import annotations

import socket
from contextlib import closing
from dataclasses import dataclass


@dataclass
class TaoJieLab:
    """本机回环上的极简 echo 服务（教学 API）。"""

    def echo_fuwu(self, duankou: int, yici_huifu: bytes) -> None:
        with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as srv:
            srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            srv.bind(("127.0.0.1", duankou))
            srv.listen(1)
            conn, _ = srv.accept()
            with closing(conn):
                data = conn.recv(4096)
                conn.sendall(yici_huifu if yici_huifu else data)

    def echo_kehu(self, duankou: int, xiaoxi: bytes) -> bytes:
        with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as cli:
            cli.connect(("127.0.0.1", duankou))
            cli.sendall(xiaoxi)
            return cli.recv(4096)
