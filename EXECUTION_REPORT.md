# MiniMPU Studio V0.1.1 — Execution Report

V0.1.1 was executed against two constraints:

1. the frozen V0.1 product surface in `DESIGN_BASELINE.md` must remain unchanged;
2. the first-use defect must be fixed exactly as specified in `ONBOARDING_PATCH.md`.

## Frozen surface preserved

| Frozen item | V0.1.1 result |
|---|---|
| 6 pages | unchanged |
| 5 simulator controls | unchanged |
| 6 focus lessons | unchanged |
| 2 backends | unchanged |
| Unified Snapshot API | unchanged |
| CPU/MMU/RTL behavior | unchanged |

## Onboarding patch implemented

| Patch criterion | Implementation / evidence |
|---|---|
| Start on Course | `Course ← 从这里开始` selected on launch |
| Five fixed steps | `app/course_data.py::BEGINNER_STEPS` |
| One next action at a time | later step buttons disabled until predecessor completes |
| Uses existing simulator operations | `MiniMPUStudio._beginner_action()` calls Reset / existing run-until / Step Cycle paths |
| Measured results visible | Course page reports cycle, privilege, PC, scause/sepc/stval |
| Completion points to lessons | final message names Chapter 04/07/16/17 and lesson cards remain below |
| Automated first-use test | `tests/onboarding_smoke.py` |
| Visual QA | `verification/studio_v0_1_1_onboarding.png` |

## Measured guided states
The guided path uses the real V3.3 RTL replay. During verification it reaches:

- reset in M-mode;
- first M→S privilege change;
- `ECALL_FROM_U` with `scause=8`;
- first page fault with non-zero fault context;
- one exact RTL cycle step.

## Regression status
The original GUI smoke and both original backend tests still pass. The Architecture Model still reaches EduOS `SYS_EXIT / exit 0`; the RTL Replay still loads the measured 63,409-cycle / 25-event evidence set.

No item from `FUTURE.md` was promoted into V0.1.1.
