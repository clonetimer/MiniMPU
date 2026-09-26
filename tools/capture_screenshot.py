#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
from app.main import MiniMPUStudio


def grab(app, out):
    from PIL import ImageGrab
    app.update(); app.update_idletasks()
    x,y=app.winfo_rootx(),app.winfo_rooty(); w,h=app.winfo_width(),app.winfo_height()
    ImageGrab.grab(bbox=(x,y,x+w,y+h)).save(out)


def main():
    try:
        from PIL import ImageGrab  # noqa: F401
    except Exception as e:
        raise SystemExit('Pillow is only required for this optional screenshot helper: '+str(e))
    outdir=ROOT/'verification'; outdir.mkdir(exist_ok=True)
    app=MiniMPUStudio(); app.geometry('1280x800+0+0')
    app.show_page('Course')
    grab(app,outdir/'studio_v0_1_1_onboarding.png')
    course=app.pages['Course']
    for key in ['start','privilege','ecall','fault','step']:
        course._run_beginner(key)
    app.show_page('Course')
    grab(app,outdir/'studio_v0_1_1_onboarding_complete.png')
    app._close()
    print(outdir/'studio_v0_1_1_onboarding.png')
    print(outdir/'studio_v0_1_1_onboarding_complete.png')
if __name__=='__main__': main()
