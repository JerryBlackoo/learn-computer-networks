from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def test_all_modules_host_verified():
    metas = list((ROOT / "modules").glob("*/module.meta.toml"))
    assert len(metas) == 13
    for meta in metas:
        text = meta.read_text(encoding="utf-8")
        assert "host-verified" in text
