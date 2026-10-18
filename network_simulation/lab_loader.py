import importlib.util
import sys
from pathlib import Path
from types import ModuleType


def load_lab(module_dir: Path, filename: str) -> ModuleType:
    """从模块目录加载 solution/*.py（注册 sys.modules 以兼容 dataclass）。"""
    path = module_dir / "solution" / filename
    name = f"xuejing_{module_dir.name}_{filename.replace('.py', '')}"
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise ImportError(path)
    mod = importlib.util.module_from_spec(spec)
    sys.modules[name] = mod
    spec.loader.exec_module(mod)
    return mod
