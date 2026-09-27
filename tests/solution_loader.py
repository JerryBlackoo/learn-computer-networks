from pathlib import Path
from types import ModuleType

from wangluo_xuejing.lab_loader import load_lab


def load_solution(module_dir: Path, filename: str) -> ModuleType:
    return load_lab(module_dir, filename)
