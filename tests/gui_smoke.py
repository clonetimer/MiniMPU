#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]; sys.path.insert(0,str(ROOT))
from app.main import MiniMPUStudio

def main():
    app=MiniMPUStudio(); app.update_idletasks()
    assert len(app.pages)==6
    assert app.current_backend.name=='RTL Replay'
    app.step_cycle(); app.step_instruction(); app.run_until()
    for name in app.page_names:
        app.show_page(name); app.update_idletasks()
    app._switch_backend('Architecture Model'); app.step_instruction(); app.show_page('Privilege / CSR'); app.update_idletasks()
    app._switch_backend('RTL Replay'); app.show_page('MMU / TLB / PTW'); app.update_idletasks()
    app._close()
    print('[PASS] GUI smoke')
if __name__=='__main__': main()
