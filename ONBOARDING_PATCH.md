# MiniMPU Studio V0.1.1 — First-Use Onboarding Patch

## 1. Problem
Student feedback showed that V0.1 exposed the correct simulator functions but did not tell a first-time learner **what to click first**. A learner could see six pages, two backends and five controls before having any operational mental model.

This is treated as a **V0.1 usability defect**, not a request to broaden the product.

## 2. Change budget
V0.1.1 may change only the first-use experience inside the existing **Course** page and supporting documentation/tests.

It must **not** add:
- a seventh page;
- a sixth simulator control;
- another backend;
- new CPU/MMU/RTL functionality;
- compilation, IDE, Linux or waveform-renderer features.

## 3. Fixed five-step path
The Course page presents exactly one enabled next action at a time:

1. **Start** — force RTL Replay and Reset. Learn only `PC` and `M/S/U`.
2. **Privilege** — use existing `Run Until Event` logic to stop at the first privilege change. Observe M→S.
3. **ECALL** — stop at `ECALL_FROM_U`. Observe `scause=8` and `sepc`.
4. **Page Fault** — stop at the first page fault. Observe `scause` and `stval`.
5. **Step Cycle** — Reset and execute one real RTL cycle. Connect the guided action to the existing toolbar control.

Only after these five steps does the page direct the learner to the six focused lessons.

## 4. Interaction rules
- On launch, **Course ← 从这里开始** is the selected page.
- Only step 1 is enabled initially.
- Completing a step checks it and unlocks exactly the next step.
- Each step reports a short measured result in the same page; no terminology wall or modal tutorial is introduced.
- The path always uses RTL Replay, so a learner cannot accidentally enter instruction-atomic Architecture Model while learning cycle stepping.
- The original toolbar remains unchanged.

## 5. Acceptance criteria
V0.1.1 is accepted only if:
1. the original V0.1 six pages / five controls / two backends remain unchanged;
2. the five beginner steps are visible on launch;
3. the steps unlock sequentially;
4. each step reaches the intended real V3.3 RTL state;
5. completion clearly tells the learner what to do next;
6. the onboarding smoke test passes under Xvfb;
7. all original backend and GUI smoke tests still pass;
8. a visual QA screenshot shows the first-use page at 1280×800 without hiding the guided controls.

## 6. Explicitly deferred
Persistent completion history, user accounts, adaptive tutorials, animated coaches, tooltips on every field and course progress storage are **not** part of V0.1.1.
