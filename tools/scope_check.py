#!/usr/bin/env python3
from pathlib import Path
R=Path(__file__).resolve().parents[1]
main=(R/'app/main.py').read_text()
required_pages=['Course','CPU','Privilege / CSR','MMU / TLB / PTW','Memory','Timeline']
for p in required_pages:
    assert repr(p) in main or f"'{p}'" in main
for token in ['Reset','Step Instruction','Step Cycle','Run Until Event']:
    assert token in main
course=(R/'app/course_data.py').read_text()
for ch in ['04','07','08','16','17','18']:
    assert f'"chapter":"{ch}"' in course
for key in ['start','privilege','ecall','fault','step']:
    assert f'"key": "{key}"' in course
assert 'Course  ← 从这里开始' in main
assert (R/'DESIGN_BASELINE.md').exists()
assert (R/'ONBOARDING_PATCH.md').exists()
assert (R/'FUTURE.md').exists()
print('[PASS] frozen V0.1 scope + bounded V0.1.1 onboarding patch present')
