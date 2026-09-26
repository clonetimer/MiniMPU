#!/usr/bin/env python3
from pathlib import Path
import subprocess, sys, shutil
R=Path(__file__).resolve().parents[1]
V=R/'verification/latest'
if V.exists(): shutil.rmtree(V)
V.mkdir(parents=True)
steps=[
 ('01_scope',[sys.executable,'tools/scope_check.py']),
 ('02_pycompile',[sys.executable,'-m','compileall','-q','app','engine/python','tests','tools']),
 ('03_bridge_build',[sys.executable,'tools/build_bridge.py']),
 ('04_backends',[sys.executable,'tests/test_backends.py']),
 ('05_gui_smoke',['xvfb-run','-a',sys.executable,'tests/gui_smoke.py']),
 ('06_onboarding_smoke',['xvfb-run','-a',sys.executable,'tests/onboarding_smoke.py']),
]
failed=False
for name,cmd in steps:
    p=subprocess.run(cmd,cwd=R,text=True,capture_output=True)
    (V/(name+'.log')).write_text('$ '+' '.join(cmd)+'\n'+p.stdout+p.stderr)
    (V/(name+'.exit')).write_text(str(p.returncode)+'\n')
    print(f'{name}: {p.returncode}')
    failed |= p.returncode!=0
raise SystemExit(1 if failed else 0)
