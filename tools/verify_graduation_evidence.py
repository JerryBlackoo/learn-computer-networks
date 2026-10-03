#!/usr/bin/env python3
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]; E=ROOT/"evidence"
required=[ROOT/"GRADUATION.md",E/"environment.json",E/"commands.md",E/"results.json",E/"failures.md",E/"final-report.md",E/"raw"/"README.md"]
missing=[str(p.relative_to(ROOT)) for p in required if not p.exists()]
if missing: raise SystemExit("missing graduation evidence files: "+", ".join(missing))
env=json.loads((E/"environment.json").read_text(encoding="utf-8")); res=json.loads((E/"results.json").read_text(encoding="utf-8"))
bad=[g for g in [f"G{i}" for i in range(8)] if res.get("gates",{}).get(g,{}).get("status")!="pass"]
if res.get("graduation_status")=="PASS":
    if bad: raise SystemExit("PASS forbidden: non-pass gates: "+", ".join(bad))
    if not env.get("commit_sha"): raise SystemExit("PASS forbidden: environment.json commit_sha is empty")
    if not [p for p in (E/"raw").iterdir() if p.name!="README.md"]: raise SystemExit("PASS forbidden: no primary raw evidence")
    if "graduation_status: **PASS**" not in (E/"final-report.md").read_text(encoding="utf-8"): raise SystemExit("PASS forbidden: final report mismatch")
print("graduation evidence structure: OK")
if bad: print("repository remains NOT_CLOSED; pending:",", ".join(bad))
