from pathlib import Path
import http.server
import ssl
import subprocess
import tempfile
import threading
import unittest
import sys
sys.path.insert(0,str(Path(__file__).resolve().parent))
from probe import probe

class Handler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        body=b'first\nsecond\n';self.send_response(200);self.send_header('Content-Length',str(len(body)));self.end_headers();self.wfile.write(body);self.wfile.flush()
    def log_message(self,*args):pass

class WireProbe(unittest.TestCase):
    def run_probe(self,context=None,ca_file=None):
        server=http.server.ThreadingHTTPServer(('127.0.0.1',0),Handler)
        if context:server.socket=context.wrap_socket(server.socket,server_side=True)
        worker=threading.Thread(target=server.serve_forever,daemon=True);worker.start()
        try:return probe(f"{'https' if context else 'http'}://localhost:{server.server_port}/stream",ca_file=ca_file)
        finally:server.shutdown();server.server_close();worker.join()
    def test_actual_http(self):
        report=self.run_probe();self.assertEqual(report['status'],200);self.assertEqual(report['body_wire_bytes'],13);self.assertIsNone(report['tls_handshake_ms']);self.assertTrue(report['recv_events'])
    def test_actual_verified_tls(self):
        with tempfile.TemporaryDirectory() as tmp:
            cert,key=tmp+'/cert.pem',tmp+'/key.pem'
            subprocess.run(['openssl','req','-x509','-newkey','rsa:2048','-nodes','-keyout',key,'-out',cert,'-days','1','-subj','/CN=localhost','-addext','subjectAltName=DNS:localhost'],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
            context=ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER);context.load_cert_chain(cert,key)
            report=self.run_probe(context,cert);self.assertEqual(report['status'],200);self.assertGreater(report['tls_handshake_ms'],0)
    def test_reject_bad_target(self):
        with self.assertRaises(ValueError):probe('http://user:secret@localhost/')
if __name__=='__main__':unittest.main()

class BoundaryProbe(unittest.TestCase):
    def test_disconnect_reports_incomplete_body(self):
        class Short(Handler):
            def do_GET(self):
                self.send_response(200);self.send_header('Content-Length','100');self.end_headers();self.wfile.write(b'short')
        server=http.server.ThreadingHTTPServer(('127.0.0.1',0),Short)
        worker=threading.Thread(target=server.serve_forever,daemon=True);worker.start()
        try:
            report=probe(f'http://127.0.0.1:{server.server_port}/')
            self.assertFalse(report['body_complete']);self.assertFalse(report['truncated'])
        finally:server.shutdown();server.server_close();worker.join()

    def test_invalid_budget_is_rejected_before_resolution(self):
        for timeout in (float('nan'),float('inf'),True):
            with self.assertRaises(ValueError):probe('http://localhost/',timeout=timeout)
        with self.assertRaises(ValueError):probe('http://localhost/',max_bytes=1024.5)
