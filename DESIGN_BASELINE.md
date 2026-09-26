# MiniMPU Studio V0.1 — Frozen Design Baseline

Status: **FROZEN for V0.1 implementation**

> **V0.1.1 maintenance note:** the first-use onboarding described in `ONBOARDING_PATCH.md` is a usability repair inside the existing Course page. It does not add a page, simulator control, backend, lesson or hardware feature, so the frozen product scope below remains unchanged.

## 1. Product goal
MiniMPU Studio V0.1 is a desktop teaching application for learning the mechanisms that distinguish an MPU-class RISC-V system from a basic MCU: privilege levels, CSR/traps, Sv39 MMU, TLB/PTW, virtual/physical memory, syscall/page-fault flow, and the complete EduOS M→S→U path.

It is not an IDE, EDA suite, Linux development environment, or general-purpose debugger.

## 2. V0.1 user promise
A learner can open one application, select either a live architectural reference model or a recorded real-RTL execution, and use the same controls/state views to study six focused lessons without first reading the complete RTL source tree.

## 3. Frozen feature scope
### 3.1 Six pages
1. **Course** — six focused lessons and learning objectives.
2. **CPU** — PC, current instruction, privilege, cycle/instret, x0–x31.
3. **Privilege / CSR** — M/S/U state, trap-related CSR values.
4. **MMU / TLB / PTW** — VA→PA path, SATP, TLB/PTW state for RTL replay.
5. **Memory** — fixed SoC map plus current instruction/data VA/PA observations.
6. **Timeline** — measured RTL events and a local cycle table.

### 3.2 Five controls
- Reset
- Step Instruction
- Step Cycle
- Run / Pause
- Run Until Event

### 3.3 Two backends
**Architecture backend**
- Live MiniMPU C++ reference model.
- Runs the existing EduOS kernel/user ELF files.
- Instruction-resolution state.
- Step Cycle intentionally aliases one architectural instruction because the reference model is not cycle-accurate.

**RTL Replay backend**
- Loads the measured V3.3 Verilator microtrace/event CSV files.
- True cycle stepping.
- Instruction stepping by advancing to the next `instret` change.
- Run-until support for privilege changes, U-mode ECALL, page faults, TLB miss and exit.

### 3.4 Six focused lessons
- Chapter 04 — M/S/U Privilege
- Chapter 07 — Sv39 MMU
- Chapter 08 — TLB / SFENCE.VMA
- Chapter 16 — Syscall / Trap
- Chapter 17 — Page Fault / Isolation
- Chapter 18 — Full EduOS

## 4. Technical architecture
```
MiniMPU Studio (Tkinter)
        |
  Unified Snapshot API
     /            \
C++ Architecture   RTL Trace Replay
    backend          backend
     |                 |
MiniMPU C++       V3.3 measured
reference model   microtrace/events
```

The UI must never depend directly on backend-specific fields. Backends normalize state into one `Snapshot` structure.

## 5. Toolkit decision
V0.1 uses **Python 3 + Tkinter**.

Reason: it is available in the execution environment, has no additional runtime package dependency, and allows the interaction design to be verified now. The simulator remains C++ where appropriate. A future Qt/PySide port is permitted only after V0.1 interaction semantics stabilize.

## 6. Deliberate non-goals
The following are explicitly excluded from V0.1:
- source-code editor or IDE features;
- compile/assemble buttons inside the GUI;
- live Verilator compilation or live RTL simulation launched from the GUI;
- embedded VCD waveform renderer;
- OpenSBI/Linux teaching pages;
- multicore support;
- plugin framework;
- project/workspace management;
- cloud/network service;
- Windows/macOS native installers;
- PySide/Qt migration;
- changing the MiniMPU ISA/RTL functionality.

These may be considered later but cannot block V0.1.

## 7. Acceptance criteria
V0.1 is complete only when all are true:
1. Application launches under Linux with Python standard library only.
2. Six pages and five controls are present.
3. Live C++ architecture bridge builds and reaches EduOS exit code 0.
4. RTL replay loads all 63,409 measured cycle rows and 25 measured events.
5. Architecture mode supports reset, instruction step, run and run-until trap/exit.
6. RTL mode supports cycle step, instruction step and all defined run-until events.
7. GUI can switch backend without restart.
8. The six focused lessons are selectable from Course page.
9. Headless GUI smoke test passes under Xvfb.
10. Backend/unit tests pass and a release verification report is generated.

## 8. Change-control rule
During V0.1 implementation, a new idea may enter the release only if it is required to satisfy an acceptance criterion or replaces an already-scoped feature without increasing scope. Everything else goes to `FUTURE.md`.
