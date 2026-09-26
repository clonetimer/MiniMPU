from __future__ import annotations
from dataclasses import dataclass, field
from typing import Dict, List

@dataclass
class Snapshot:
    backend: str = ""
    resolution: str = ""
    cycle: int = 0
    instret: int = 0
    privilege: str = "M"
    pc: int = 0
    instruction: str = ""
    regs: List[int] = field(default_factory=lambda: [0] * 32)
    csrs: Dict[str, int] = field(default_factory=dict)
    stopped: bool = False
    exit_code: int = 0
    uart: str = ""
    core_state: str = ""
    immu_state: str = ""
    dmmu_state: str = ""
    itlb_hit: bool = False
    dtlb_hit: bool = False
    iptw_state: str = ""
    dptw_state: str = ""
    iptw_level: int = 0
    dptw_level: int = 0
    ipte_addr: int = 0
    dpte_addr: int = 0
    ipte: int = 0
    dpte: int = 0
    i_va: int = 0
    i_pa: int = 0
    d_va: int = 0
    d_pa: int = 0
    dmem_pf: bool = False
    imem_pf: bool = False
    event: str = ""

    def hex(self, value: int, width: int = 16) -> str:
        return f"0x{value:0{width}x}"
