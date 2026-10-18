from pathlib import Path
from types import ModuleType

from network_simulation.lab_loader import load_lab


def load_solution(module_dir: Path, filename: str) -> ModuleType:
    return load_lab(module_dir, filename)
