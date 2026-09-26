from __future__ import annotations
import re
from pathlib import Path

LINE_RE = re.compile(r"^\s*([0-9a-fA-F]+):\s+(?:[0-9a-fA-F]{8}\s+)?(.+?)\s*$")

class Disassembly:
    def __init__(self, *paths: Path):
        self.by_pc: dict[int, str] = {}
        for path in paths:
            if path.exists():
                self._load(path)

    def _load(self, path: Path) -> None:
        for line in path.read_text(errors="replace").splitlines():
            m = LINE_RE.match(line)
            if not m:
                continue
            try:
                pc = int(m.group(1), 16)
            except ValueError:
                continue
            text = m.group(2).strip()
            if text:
                self.by_pc[pc] = text

    def lookup(self, pc: int) -> str:
        return self.by_pc.get(pc, "")
