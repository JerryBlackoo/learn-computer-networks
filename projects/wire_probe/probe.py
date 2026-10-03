"""Actual DNS/TCP/TLS/HTTP timings. recv events are application observations, not packet captures."""
import argparse
import hashlib
import json
import socket
import ssl
import time
from urllib.parse import urlsplit

def probe(url,ca_file=None,timeout=3,max_bytes=65536):
    parsed=urlsplit(url)
    if parsed.scheme not in ('http','https') or not parsed.hostname or parsed.username or parsed.password:raise ValueError('HTTP(S) URL without credentials required')
    path=parsed.path or '/'
    if parsed.query:path+='?'+parsed.query
    if any(c in path+parsed.hostname for c in '\r\n'):raise ValueError('invalid HTTP target')
    if timeout<=0 or max_bytes<1024:raise ValueError('timeout/bound')
    port=parsed.port or (443 if parsed.scheme=='https' else 80)
    start=time.perf_counter();addresses=socket.getaddrinfo(parsed.hostname,port,type=socket.SOCK_STREAM);dns_end=time.perf_counter()
    sock=None;error=None
    for family,kind,proto,_,address in addresses:
        candidate=socket.socket(family,kind,proto);candidate.settimeout(timeout)
        try:candidate.connect(address);sock=candidate;break
        except OSError as e:candidate.close();error=e
    if sock is None:raise error or OSError('no address')
    tcp_end=time.perf_counter();tls_ms=None
    try:
        if parsed.scheme=='https':
            context=ssl.create_default_context(cafile=ca_file)
            sock=context.wrap_socket(sock,server_hostname=parsed.hostname)
            tls_ms=1000*(time.perf_counter()-tcp_end)
        host=f'[{parsed.hostname}]' if ':' in parsed.hostname else parsed.hostname
        if port not in (80,443):host+=':'+str(port)
        request=f'GET {path} HTTP/1.1\r\nHost: {host}\r\nConnection: close\r\n\r\n'.encode('ascii')
        sent=time.perf_counter();sock.sendall(request);data=bytearray();events=[];first=None;truncated=False
        while len(data)<max_bytes:
            block=sock.recv(min(4096,max_bytes-len(data)))
            now=time.perf_counter()
            if not block:break
            if first is None:first=now
            data.extend(block);events.append({'since_request_ms':1000*(now-sent),'bytes':len(block)})
        else:truncated=True
        header,separator,body=bytes(data).partition(b'\r\n\r\n')
        if not separator or not header.startswith(b'HTTP/'):raise ValueError('incomplete HTTP headers')
        status=int(header.split(b' ',2)[1]);headers=header.decode('iso-8859-1')
        return {'evidence':'actual-host-network','scheme':parsed.scheme,'status':status,'resolution_ms':1000*(dns_end-start),'tcp_connect_ms':1000*(tcp_end-dns_end),'tls_handshake_ms':tls_ms,'first_response_byte_ms':None if first is None else 1000*(first-sent),'recv_events':events,'body_wire_bytes':len(body),'body_sha256':hashlib.sha256(body).hexdigest(),'truncated':truncated,'chunked':'transfer-encoding: chunked' in headers.lower(),'scope':'recv calls may coalesce packets; timing alone cannot attribute root cause. DNS timing may be resolver-cache work.'}
    finally:sock.close()

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('url');p.add_argument('--ca-file');args=p.parse_args();print(json.dumps(probe(args.url,args.ca_file),indent=2))
