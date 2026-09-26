#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))

from app.course_data import BEGINNER_STEPS
from app.main import MiniMPUStudio


def main():
    app=MiniMPUStudio()
    app.update_idletasks()
    course=app.pages['Course']
    keys=[s['key'] for s in BEGINNER_STEPS]
    assert len(keys)==5
    assert course.step_widgets[keys[0]][1].instate(['!disabled'])
    for key in keys[1:]:
        assert course.step_widgets[key][1].instate(['disabled'])

    expected=[
        ('start','M-mode'),
        ('privilege','权限切换'),
        ('ecall','scause=8'),
        ('fault','stval='),
        ('step','Step Cycle'),
    ]
    for i,(key,token) in enumerate(expected, start=1):
        course._run_beginner(key)
        app.update_idletasks()
        assert key in course.completed, key
        assert token in course.beginner_result.get(), (key,course.beginner_result.get())
        assert len(course.completed)==i
        assert course.step_widgets[key][0].get()=='✓'
        if i < len(keys):
            assert course.step_widgets[keys[i]][1].instate(['!disabled'])

    assert course.progress_var.get()=='入门进度：5 / 5'
    assert '入门完成' in course.beginner_result.get()
    assert app.current_backend.name=='RTL Replay'
    app._close()
    print('[PASS] onboarding 5-step smoke')

if __name__=='__main__':
    main()
