# GRADUATION — Computer Networks

Status: **NOT_CLOSED** until G0–G7 are evidenced.

## Frozen scope
Graduate evidence-driven diagnosis across DNS, TCP, TLS, HTTP/1.1, HTTP/2 and QUIC/HTTP3. A simulated TCP stack alone is insufficient.

## Final assessment
Capture real packet traces and use network namespaces/netem or equivalent to inject delay, loss, reordering, bandwidth restriction and reset. Diagnose three end-user symptoms from packet/transport/application evidence: slow upload, stalled streaming response and connection reset.

## Target-platform gate
G5 requires real sockets and packet capture for the protocols claimed. Protocols unavailable in the environment must remain not-run.

## PASS rule
Every diagnosis must identify the layer and cite the observable trace/event that supports it.
