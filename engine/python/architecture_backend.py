from __future__ import annotations
import json
import subprocess
from pathlib import Path
from .snapshot import Snapshot
from .disasm import Disassembly

class ArchitectureBackend:
    name = "Architecture Model"
    resolution = "instruction"

    def __init__(self, bridge: Path, kernel: Path, user: Path, disasm: Disassembly):
        self.bridge = Path(bridge)
        self.kernel = Path(kernel)
        self.user = Path(user)
        self.disasm = disasm
        self.proc: subprocess.Popen[str] | None = None
        self._last: Snapshot | None = None
        self._start()

    def _start(self):
        if self.proc:
            try:
                self.proc.terminate()
            except Exception:
                pass
        self.proc = subprocess.Popen(
            [str(self.bridge), str(self.kernel), str(self.user)],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            text=True, bufsize=1,
        )
        self._last = self._command("STATE")

    def close(self):
        if self.proc and self.proc.poll() is None:
            try:
                self.proc.stdin.write("QUIT\n")
                self.proc.stdin.flush()
            except Exception:
                pass
            self.proc.terminate()

    def _command(self, cmd: str) -> Snapshot:
        assert self.proc and self.proc.stdin and self.proc.stdout
        if self.proc.poll() is not None:
            err = self.proc.stderr.read() if self.proc.stderr else ""
            raise RuntimeError(f"architecture bridge exited: {err}")
        self.proc.stdin.write(cmd + "\n")
        self.proc.stdin.flush()
        line = self.proc.stdout.readline()
        if not line:
            err = self.proc.stderr.read() if self.proc.stderr else ""
            raise RuntimeError(f"no bridge response: {err}")
        d = json.loads(line)
        regs = [int(x, 0) if isinstance(x, str) else int(x) for x in d.get("regs", [0]*32)]
        csrs = {k:(int(v,0) if isinstance(v,str) else int(v)) for k,v in d.get("csrs",{}).items()}
        pc = int(d["pc"], 0) if isinstance(d["pc"], str) else int(d["pc"])
        s = Snapshot(
            backend=self.name, resolution=self.resolution,
            cycle=int(d.get("cycle", 0)), instret=int(d.get("instret", 0)),
            privilege=d.get("privilege", "M"), pc=pc,
            instruction=self.disasm.lookup(pc), regs=regs, csrs=csrs,
            stopped=bool(d.get("stopped", False)), exit_code=int(d.get("exit_code", 0)),
            uart=d.get("uart", ""), i_va=pc,
        )
        self._last = s
        return s

    def snapshot(self):
        return self._last or self._command("STATE")

    def reset(self):
        return self._command("RESET")

    def step_instruction(self):
        return self._command("STEP 1")

    def step_cycle(self):
        # Architectural model is atomic at instruction granularity by design.
        return self.step_instruction()

    def run_steps(self, count: int = 100):
        return self._command(f"STEP {max(1, count)}")

    def run_until(self, condition: str, max_steps: int = 100000):
        start = self.snapshot()
        prev_priv = start.privilege
        prev_scause = start.csrs.get("scause", 0)
        for _ in range(max_steps):
            s = self.step_instruction()
            if condition == "Privilege change" and s.privilege != prev_priv:
                return s
            if condition == "ECALL_FROM_U" and s.privilege == "S" and s.csrs.get("scause") == 8:
                return s
            if condition == "Page fault" and s.csrs.get("scause") in (12,13,15) and s.csrs.get("scause") != prev_scause:
                return s
            if condition == "SIM_EXIT" and s.stopped:
                return s
            if condition == "TLB miss":
                # Not observable in the instruction-atomic reference model.
                return s
            prev_priv = s.privilege
            prev_scause = s.csrs.get("scause", 0)
            if s.stopped:
                return s
        return self.snapshot()

    def nearby_rows(self, radius: int = 6):
        return []
