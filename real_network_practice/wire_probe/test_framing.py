import http.server
import threading
import unittest
from probe import probe

class Framing(unittest.TestCase):
    def run_case(self,headers):
        class Handler(http.server.BaseHTTPRequestHandler):
            def do_GET(self):
                self.send_response(200)
                for key,value in headers:self.send_header(key,value)
                self.end_headers();self.wfile.write(b'1\r\nx\r\n0\r\n\r\n')
            def log_message(self,*args):pass
        server=http.server.ThreadingHTTPServer(('127.0.0.1',0),Handler)
        worker=threading.Thread(target=server.serve_forever,daemon=True);worker.start()
        try:return probe(f'http://127.0.0.1:{server.server_port}/')
        finally:server.shutdown();server.server_close();worker.join()
    def test_transfer_and_length_conflict_rejected(self):
        with self.assertRaises(ValueError):self.run_case([('Transfer-Encoding','chunked'),('Content-Length','11')])
    def test_coding_list_does_not_claim_decoded_completeness(self):
        report=self.run_case([('Transfer-Encoding','gzip, chunked')])
        self.assertTrue(report['chunked']);self.assertIsNone(report['body_complete'])
    def test_chunked_must_be_final(self):
        with self.assertRaises(ValueError):self.run_case([('Transfer-Encoding','chunked, gzip')])
