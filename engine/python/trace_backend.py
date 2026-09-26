from __future__ import annotations
import csv
from pathlib import Path
from .snapshot import Snapshot
from .disasm import Disassembly

CORE = {"0":"RESET","1":"FETCH","2":"WAIT_I","3":"EXEC","4":"WAIT_D"}
MMU = {"0":"IDLE","1":"TLB","2":"PTW_REQ","3":"PTW_WAIT","4":"RESP"}
PTW = {"0":"IDLE","1":"REQ","2":"WAIT","3":"CHECK","4":"FINISH"}

def _ival(v: str) -> int:
    v = (v or "0").strip()
    return int(v, 0)

class RTLTraceBackend:
    name = "RTL Replay"
    resolution = "cycle"

    def __init__(self, trace_path: Path, events_path: Path, disasm: Disassembly):
        self.trace_path = Path(trace_path)
        self.events_path = Path(events_path)
        self.disasm = disasm
        with self.trace_path.open(newline="") as f:
            self.rows = list(csv.DictReader(f))
        if not self.rows:
            raise RuntimeError("RTL trace is empty")
        with self.events_path.open(newline="") as f:
            self.events = list(csv.DictReader(f))
        self.event_by_cycle: dict[int, list[str]] = {}
        for e in self.events:
            self.event_by_cycle.setdefault(_ival(e["cycle"]), []).append(e["event"])
        self.index = 0

    @property
    def row_count(self) -> int:
        return len(self.rows)

    def reset(self) -> Snapshot:
        self.index = 0
        return self.snapshot()

    def snapshot(self) -> Snapshot:
        r = self.rows[self.index]
        cycle = _ival(r["cycle"])
        event = ", ".join(self.event_by_cycle.get(cycle, []))
        pc = _ival(r["pc"])
        return Snapshot(
            backend=self.name,
            resolution=self.resolution,
            cycle=cycle,
            instret=_ival(r["instret"]),
            privilege=r["priv"],
            pc=pc,
            instruction=self.disasm.lookup(pc),
            regs=[0]*32,
            csrs={
                "satp": _ival(r["satp"]),
                "scause": _ival(r["scause"]),
                "sepc": _ival(r["sepc"]),
                "stval": _ival(r["stval"]),
            },
            stopped=self.index >= len(self.rows)-1,
            exit_code=0,
            core_state=CORE.get(r["core_state"], r["core_state"]),
            immu_state=MMU.get(r["immu_state"], r["immu_state"]),
            dmmu_state=MMU.get(r["dmmu_state"], r["dmmu_state"]),
            itlb_hit=bool(_ival(r["itlb_hit"])),
            dtlb_hit=bool(_ival(r["dtlb_hit"])),
            iptw_state=PTW.get(r["iptw_state"], r["iptw_state"]),
            dptw_state=PTW.get(r["dptw_state"], r["dptw_state"]),
            iptw_level=_ival(r["iptw_level"]),
            dptw_level=_ival(r["dptw_level"]),
            ipte_addr=_ival(r["ipte_addr"]),
            dpte_addr=_ival(r["dpte_addr"]),
            ipte=_ival(r["ipte"]),
            dpte=_ival(r["dpte"]),
            i_va=pc,
            i_pa=_ival(r["i_pa"]),
            d_va=_ival(r["d_va"]),
            d_pa=_ival(r["d_pa"]),
            dmem_pf=bool(_ival(r["dmem_pf"])),
            imem_pf=bool(_ival(r["imem_pf"])),
            event=event,
        )

    def step_cycle(self) -> Snapshot:
        if self.index < len(self.rows)-1:
            self.index += 1
        return self.snapshot()

    def step_instruction(self) -> Snapshot:
        start = _ival(self.rows[self.index]["instret"])
        while self.index < len(self.rows)-1:
            self.index += 1
            if _ival(self.rows[self.index]["instret"]) != start:
                break
        return self.snapshot()

    def run_steps(self, count: int = 100) -> Snapshot:
        self.index = min(len(self.rows)-1, self.index + max(1, count))
        return self.snapshot()

    def run_until(self, condition: str, max_steps: int = 200000) -> Snapshot:
        start_priv = self.snapshot().privilege
        for _ in range(max_steps):
            s = self.step_cycle()
            if condition == "Privilege change" and s.privilege != start_priv:
                return s
            if condition == "ECALL_FROM_U" and "ECALL_FROM_U" in s.event:
                return s
            if condition == "Page fault" and (s.csrs.get("scause") in (12,13,15)):
                return s
            if condition == "TLB miss" and (not s.itlb_hit or (s.d_va and not s.dtlb_hit)) and (s.iptw_state != "IDLE" or s.dptw_state != "IDLE"):
                return s
            if condition == "SIM_EXIT" and ("SIM_EXIT" in s.event or s.stopped):
                return s
            if s.stopped:
                return s
        return self.snapshot()

    def nearby_rows(self, radius: int = 6):
        lo = max(0, self.index-radius)
        hi = min(len(self.rows), self.index+radius+1)
        return [(i, self.rows[i]) for i in range(lo, hi)]
