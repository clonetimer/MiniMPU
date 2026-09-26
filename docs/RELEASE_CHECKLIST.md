# V0.1.1 Release Checklist

## Frozen V0.1 baseline
- [x] Six pages unchanged.
- [x] Five simulator controls unchanged.
- [x] Six focus lessons unchanged.
- [x] Two backend adapters unchanged in scope.
- [x] C++ architecture bridge reaches EduOS exit 0.
- [x] RTL trace loads 63,409 cycles / 25 events.
- [x] Backend switching works without restart.

## First-use patch
- [x] Course is visibly marked `从这里开始`.
- [x] Five-step onboarding is visible at launch.
- [x] Only the next step is enabled.
- [x] Step 1 teaches Reset / PC / privilege.
- [x] Step 2 reaches first M→S transition.
- [x] Step 3 reaches U-mode ECALL and shows scause/sepc.
- [x] Step 4 reaches page fault and shows scause/stval.
- [x] Step 5 executes one real RTL cycle.
- [x] Final message directs learner to formal lessons.
- [x] Onboarding Xvfb smoke passes.
- [x] 1280×800 screenshots captured for visual QA.

## Still deliberately out of scope
- [ ] Native installers.
- [ ] Live Verilator GUI backend.
- [ ] Embedded waveform renderer.
- [ ] Persistent user/course progress.
