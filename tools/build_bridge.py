#!/usr/bin/env python3
from pathlib import Path
import os, shutil, subprocess, sys

ROOT=Path(__file__).resolve().parents[1]
SRC=ROOT/'engine/cpp/minimpu_studio_bridge.cpp'
INC=ROOT/'backend/minimpu/core'
OUT=ROOT/'bin/linux-x86_64/minimpu_studio_bridge'

def main():
    cxx=os.environ.get('CXX') or shutil.which('g++') or shutil.which('clang++')
    if not cxx:
        print('No C++17 compiler found (g++/clang++)', file=sys.stderr); return 2
    OUT.parent.mkdir(parents=True,exist_ok=True)
    cmd=[cxx,'-std=c++17','-O2','-Wall','-Wextra','-I',str(INC),str(SRC),'-o',str(OUT)]
    print('+',' '.join(cmd))
    subprocess.run(cmd,check=True)
    print(OUT)
    return 0
if __name__=='__main__': raise SystemExit(main())
