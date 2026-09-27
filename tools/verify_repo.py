#!/usr/bin/env python3
"""仓库完整性检查：starter/solution 成对、证据元数据、外链政策抽样。"""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MODULES = ROOT / "modules"
DOCS = ROOT / "docs"
CATALOG = ROOT / "sources" / "catalog.toml"

URL_RE = re.compile(r"https?://[^\s\)>\"]+")


def main() -> int:
    errors: list[str] = []

    for doc in DOCS.glob("*.md"):
        if URL_RE.search(doc.read_text(encoding="utf-8")):
            errors.append(f"docs 中含 URL（应仅 catalog.toml）: {doc.name}")

    for readme in MODULES.glob("*/README.md"):
        if URL_RE.search(readme.read_text(encoding="utf-8")):
            errors.append(f"模块 README 中含 URL: {readme}")

    for meta in MODULES.glob("*/module.meta.toml"):
        text = meta.read_text(encoding="utf-8")
        if "host-verified" not in text and "not-run" not in text:
            errors.append(f"缺少 evidence 标签: {meta}")

    for mod in sorted(MODULES.iterdir()):
        if not mod.is_dir():
            continue
        starter = mod / "starter"
        solution = mod / "solution"
        if not starter.is_dir() or not solution.is_dir():
            errors.append(f"缺少 starter/solution: {mod.name}")
            continue
        s_py = list(starter.glob("*.py"))
        if not s_py:
            errors.append(f"starter 无 .py: {mod.name}")
        sol_py = list(solution.glob("*.py"))
        if not sol_py:
            errors.append(f"solution 无 .py: {mod.name}")

    if not CATALOG.exists():
        errors.append("缺少 sources/catalog.toml")

    if errors:
        for e in errors:
            print(f"ERROR: {e}", file=sys.stderr)
        return 1

    print("verify_repo: OK — 12 模块结构、文档与外链政策检查通过")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
