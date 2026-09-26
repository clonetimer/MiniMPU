#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]; sys.path.insert(0,str(ROOT))
from engine.python import Disassembly, RTLTraceBackend, ArchitectureBackend

FW=ROOT/'backend/minimpu/firmware/edu_os'
D=Disassembly(FW/'kernel.disasm.txt',FW/'user.disasm.txt')

def test_rtl():
    b=RTLTraceBackend(ROOT/'backend/minimpu/traces/edu_v3_3_microtrace.csv', ROOT/'backend/minimpu/traces/edu_v3_3_events.csv', D)
    assert b.row_count==63409, b.row_count
    assert len(b.events)==25, len(b.events)
    s=b.reset(); assert s.privilege=='M'
    s=b.run_until('Privilege change'); assert s.privilege=='S'
    b.reset(); s=b.run_until('ECALL_FROM_U'); assert s.csrs['scause']==8 and s.privilege=='S'
    b.reset(); s=b.run_until('Page fault'); assert s.csrs['scause'] in (12,13,15)
    b.reset(); s=b.run_until('SIM_EXIT'); assert s.stopped
    print('[PASS] RTL replay backend')

def test_arch():
    b=ArchitectureBackend(ROOT/'bin/linux-x86_64/minimpu_studio_bridge',FW/'kernel.elf',FW/'user.bin',D)
    try:
        s=b.reset(); assert s.privilege=='M' and s.pc==0x80000000
        s=b.run_until('Privilege change'); assert s.privilege=='S'
        b.reset(); s=b.run_until('ECALL_FROM_U'); assert s.privilege=='S' and s.csrs['scause']==8
        b.reset(); s=b.run_until('Page fault'); assert s.csrs['scause'] in (12,13,15)
        b.reset(); s=b.run_until('SIM_EXIT'); assert s.stopped and s.exit_code==0
        assert 'SYS_EXIT' in s.uart
        print('[PASS] Architecture backend')
    finally: b.close()

if __name__=='__main__':
    test_rtl(); test_arch()
