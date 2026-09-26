# MiniMPU Studio V0.1.1

A bounded desktop teaching front-end for MiniMPU.

V0.1.1 is a **first-use onboarding patch** over the frozen V0.1 product baseline. It adds no simulator capability. The Course page now provides a sequential five-step path so a student can learn how to operate the software before starting MPU theory.

## Frozen product surface
- 6 pages
- 5 simulator controls
- 6 focus lessons
- 2 backends

## First-use path
`Start → M→S → ECALL → Page Fault → Step Cycle`

Only the next step is enabled. All steps use the same existing V0.1 backend operations.

## Repository structure

| Directory / File | Purpose |
|------------------|---------|
| `app/` | GUI front-end (pages: course, cpu, memory, mmu, privilege, timeline) |
| `backend/` | Simulator backends — C++ `minimpu` reference model + firmware (`edu_os`) |
| `engine/` | Simulation engine / RTL replay loader |
| `tools/` | Build bridge, screenshot, scope check, verification helpers |
| `tests/` | Backend, GUI smoke, onboarding smoke tests |
| `verification/` | Latest run logs, `RESULTS.md`, onboarding screenshots |
| `docs/` | `FIRST_5_MINUTES.md` and other guides |
| `assets/` | Static assets |
| `bin/` | Prebuilt binaries (if any) |
| `requirements.txt` | Python dependencies (none required at runtime; Tkinter + optional Pillow) |
| `run_studio.py` / `run_studio.sh` | Launchers |
| `VERSION` | `0.1.1` |
| `SOURCE_MANIFEST.sha256` | Source integrity manifest |

## Requirements
- Python 3 with **Tkinter** (usually bundled with standard installs).
- **Pillow** is used only by the optional screenshot helper.
- No pip packages are required by the runtime — see `requirements.txt`.

## Quick start
In a verified Linux environment:

```bash
./run_studio.sh
# or
python run_studio.py
```

The app opens on the left **`Course ← 从这里开始`** page. Complete the five buttons in order
(~5 minutes), then pick a formal lab from Chapter 04 / 07 / 08 / 16 / 17 / 18.

## Two backends
- **RTL Replay** (default): loads real Verilator V3.3 traces — 63,409 cycles and 25 measured
  events; best for cycle-accurate MMU/TLB/PTW inspection.
- **Architecture Model**: runs the C++ reference model live; best for registers and CSRs
  (instruction-atomic, does not fake pipeline cycles).

## Documentation
- `README_START_HERE.md` — operation guide (first-open walkthrough).
- `DESIGN_BASELINE.md` — frozen V0.1 product scope.
- `ONBOARDING_PATCH.md` — why V0.1.1 is a usability fix, not a feature expansion.
- `USER_GUIDE.md` — user guide.
- `FUTURE.md` — ideas for later versions.
- `docs/FIRST_5_MINUTES.md` — step-by-step expected results.

## Scope
`DESIGN_BASELINE.md` remains the frozen V0.1 product boundary. `ONBOARDING_PATCH.md` explains why
V0.1.1 belongs to a usability fix rather than a feature expansion. `FUTURE.md` holds only ideas for
later versions.
