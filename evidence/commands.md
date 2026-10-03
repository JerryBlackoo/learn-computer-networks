# Commands

Record only commands actually executed for the final candidate run.

| # | Command | Exit code | Raw log |
|---|---------|----------:|---------|
| 1 | `bash evidence/scripts/run_captures.sh` | 0 | stdout in agent run 2026-10-03 |
| 2 | `bash evidence/scripts/run_fault_netns.sh` | 0 | netem skipped; `symptom_tcp_reset.pcap` written |
| 3 | `make verify` | 0 | pytest + verify_repo OK |

## 复现（本 VM）

需 `sudo`（`cap_net_admin` / `cap_net_raw`）。

```bash
sudo apt-get update
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y tcpdump tshark iproute2 iptables
pip3 install -r requirements-dev.txt
bash evidence/scripts/run_captures.sh
bash evidence/scripts/run_fault_netns.sh
```

校验 pcap：

```bash
tshark -r evidence/raw/dns_query_example_com.pcap -q -z io,phs
tshark -r evidence/raw/symptom_tcp_reset.pcap -Y 'tcp.flags.reset==1'
tshark -r evidence/raw/http11_example_cleartext.pcap -Y http.request
```
