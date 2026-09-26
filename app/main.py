from __future__ import annotations
import argparse, csv, os, subprocess, sys, tkinter as tk
from pathlib import Path
from tkinter import messagebox, ttk

ROOT=Path(__file__).resolve().parents[1]
if str(ROOT) not in sys.path: sys.path.insert(0,str(ROOT))

from engine.python import ArchitectureBackend, Disassembly, RTLTraceBackend
from app.pages import CoursePage, CPUPage, PrivilegePage, MMUPage, MemoryPage, TimelinePage

TRACE=ROOT/'backend/minimpu/traces/edu_v3_3_microtrace.csv'
EVENTS=ROOT/'backend/minimpu/traces/edu_v3_3_events.csv'
FW=ROOT/'backend/minimpu/firmware/edu_os'
BRIDGE=ROOT/'bin/linux-x86_64/minimpu_studio_bridge'

class MiniMPUStudio(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title('MiniMPU Studio V0.1.1')
        self.geometry('1280x800')
        self.minsize(1080,700)
        self.protocol('WM_DELETE_WINDOW',self._close)
        self.running=False
        self.backends={}
        self.current_backend=None
        self.disasm=Disassembly(FW/'kernel.disasm.txt',FW/'user.disasm.txt')
        with EVENTS.open(newline='') as f: self.events=list(csv.DictReader(f))
        self._style()
        self._build_ui()
        self._switch_backend('RTL Replay')

    def _style(self):
        style=ttk.Style(self)
        try: style.theme_use('clam')
        except tk.TclError: pass
        style.configure('Title.TLabel',font=('TkDefaultFont',18,'bold'))
        style.configure('Mono.TLabel',font=('TkFixedFont',10))
        style.configure('Toolbar.TFrame',padding=6)

    def _build_ui(self):
        toolbar=ttk.Frame(self,style='Toolbar.TFrame'); toolbar.pack(fill='x')
        ttk.Label(toolbar,text='Backend:').pack(side='left')
        self.backend_var=tk.StringVar(value='RTL Replay')
        cb=ttk.Combobox(toolbar,textvariable=self.backend_var,values=['RTL Replay','Architecture Model'],state='readonly',width=20)
        cb.pack(side='left',padx=(4,14)); cb.bind('<<ComboboxSelected>>',lambda e:self._switch_backend(self.backend_var.get()))
        ttk.Button(toolbar,text='Reset',command=self.reset).pack(side='left',padx=2)
        ttk.Button(toolbar,text='Step Instruction',command=self.step_instruction).pack(side='left',padx=2)
        ttk.Button(toolbar,text='Step Cycle',command=self.step_cycle).pack(side='left',padx=2)
        self.run_btn=ttk.Button(toolbar,text='Run',command=self.toggle_run); self.run_btn.pack(side='left',padx=2)
        self.event_var=tk.StringVar(value='ECALL_FROM_U')
        events=['Privilege change','ECALL_FROM_U','Page fault','TLB miss','SIM_EXIT']
        ttk.Combobox(toolbar,textvariable=self.event_var,values=events,state='readonly',width=18).pack(side='left',padx=(14,3))
        ttk.Button(toolbar,text='Run Until Event',command=self.run_until).pack(side='left',padx=2)

        body=ttk.Panedwindow(self,orient='horizontal'); body.pack(fill='both',expand=True)
        nav=ttk.Frame(body,padding=6); content=ttk.Frame(body); body.add(nav,weight=0); body.add(content,weight=1)
        ttk.Label(nav,text='Pages',font=('TkDefaultFont',11,'bold')).pack(anchor='w',pady=(0,5))
        self.nav=tk.Listbox(nav,width=23,height=15,exportselection=False)
        self.page_names=['Course','CPU','Privilege / CSR','MMU / TLB / PTW','Memory','Timeline']
        self.page_labels=['Course  ← 从这里开始','CPU','Privilege / CSR','MMU / TLB / PTW','Memory','Timeline']
        for n in self.page_labels:self.nav.insert('end',n)
        self.nav.pack(fill='y',expand=False); self.nav.selection_set(0); self.nav.bind('<<ListboxSelect>>',self._nav_changed)
        ttk.Separator(nav).pack(fill='x',pady=10)
        ttk.Label(nav,text='V0.1 baseline',font=('TkDefaultFont',10,'bold')).pack(anchor='w')
        ttk.Label(nav,text='6 pages\n5 controls\n6 focus labs\n2 backends',justify='left').pack(anchor='w',pady=(3,0))

        self.container=content
        self.pages={}
        self.pages['Course']=CoursePage(content,self.show_page,self._beginner_action)
        self.pages['CPU']=CPUPage(content)
        self.pages['Privilege / CSR']=PrivilegePage(content)
        self.pages['MMU / TLB / PTW']=MMUPage(content)
        self.pages['Memory']=MemoryPage(content)
        self.pages['Timeline']=TimelinePage(content,self.events)
        for p in self.pages.values(): p.grid(row=0,column=0,sticky='nsew')
        content.rowconfigure(0,weight=1); content.columnconfigure(0,weight=1)
        self.pages['Course'].tkraise()
        self.status=tk.StringVar(value='Ready')
        ttk.Label(self,textvariable=self.status,relief='sunken',anchor='w').pack(fill='x',side='bottom')

    def _build_bridge_if_needed(self):
        if BRIDGE.exists() and os.access(BRIDGE,os.X_OK): return
        p=subprocess.run([sys.executable,str(ROOT/'tools/build_bridge.py')],cwd=ROOT,capture_output=True,text=True)
        if p.returncode: raise RuntimeError('Failed to build C++ bridge:\n'+p.stdout+'\n'+p.stderr)

    def _get_backend(self,name):
        if name in self.backends:return self.backends[name]
        if name=='RTL Replay':
            b=RTLTraceBackend(TRACE,EVENTS,self.disasm)
        else:
            self._build_bridge_if_needed()
            b=ArchitectureBackend(BRIDGE,FW/'kernel.elf',FW/'user.bin',self.disasm)
        self.backends[name]=b; return b

    def _switch_backend(self,name):
        try:
            self.running=False; self.run_btn.config(text='Run')
            self.current_backend=self._get_backend(name)
            self.backend_var.set(name)
            self._refresh(self.current_backend.reset())
            extra=f"; trace rows={self.current_backend.row_count}" if hasattr(self.current_backend,'row_count') else ''
            self.status.set(f"Backend: {name}; resolution={self.current_backend.resolution}{extra}")
        except Exception as e:
            messagebox.showerror('Backend error',str(e)); self.status.set(str(e))


    def _beginner_action(self, key):
        """Run the fixed first-use path using only V0.1 simulator actions."""
        if self.current_backend is None or self.current_backend.name != 'RTL Replay':
            self._switch_backend('RTL Replay')
        if key == 'start':
            s = self.current_backend.reset()
            self._refresh(s)
            return f"① 已复位：现在是 {s.privilege}-mode，PC=0x{s.pc:016x}。先记住：PC 是当前执行位置。"
        if key == 'privilege':
            self.current_backend.reset()
            s = self.current_backend.run_until('Privilege change')
            self._refresh(s)
            return f"② 找到第一次权限切换：cycle {s.cycle}，现在是 {s.privilege}-mode，PC=0x{s.pc:016x}。M-mode 已经把控制权交给 S-mode。"
        if key == 'ecall':
            self.current_backend.reset()
            s = self.current_backend.run_until('ECALL_FROM_U')
            self._refresh(s)
            return f"③ 找到 U-mode ECALL：cycle {s.cycle}，现在进入 {s.privilege}-mode；scause={s.csrs.get('scause',0)}，sepc=0x{s.csrs.get('sepc',0):016x}。"
        if key == 'fault':
            self.current_backend.reset()
            s = self.current_backend.run_until('Page fault')
            self._refresh(s)
            return f"④ 找到 Page Fault：cycle {s.cycle}，scause={s.csrs.get('scause',0)}，stval=0x{s.csrs.get('stval',0):016x}。scause 说明错误类型，stval 是出错地址。"
        if key == 'step':
            s = self.current_backend.reset()
            before = s.cycle
            s = self.current_backend.step_cycle()
            self._refresh(s)
            return f"⑤ 你刚完成一次 Step Cycle：cycle {before} → {s.cycle}。以后顶部的 Step Cycle 就是这样逐拍推进 RTL。入门完成。"
        return ''

    def show_page(self,name):
        self.pages[name].tkraise()
        idx=self.page_names.index(name); self.nav.selection_clear(0,'end'); self.nav.selection_set(idx); self.nav.see(idx)
        if self.current_backend:self._refresh(self.current_backend.snapshot())
    def _nav_changed(self,event=None):
        sel=self.nav.curselection()
        if sel:self.show_page(self.page_names[sel[0]])

    def _refresh(self,s):
        for p in self.pages.values(): p.update_snapshot(s,self.current_backend)
        self.status.set(f"{s.backend} | {s.resolution} | cycle {s.cycle} | instret {s.instret} | {s.privilege}-mode | PC 0x{s.pc:016x}" + (f" | {s.event}" if s.event else ''))

    def reset(self):
        if self.current_backend:self._refresh(self.current_backend.reset())
    def step_instruction(self):
        if self.current_backend:self._refresh(self.current_backend.step_instruction())
    def step_cycle(self):
        if self.current_backend:self._refresh(self.current_backend.step_cycle())
    def run_until(self):
        if not self.current_backend:return
        self.running=False; self.run_btn.config(text='Run')
        condition=self.event_var.get()
        if condition=='TLB miss' and self.current_backend.resolution!='cycle':
            self.status.set('TLB miss is a microarchitectural event. Switch to RTL Replay; Architecture Model intentionally does not invent PTW timing.')
            return
        s=self.current_backend.run_until(condition)
        self._refresh(s)
    def toggle_run(self):
        self.running=not self.running; self.run_btn.config(text='Pause' if self.running else 'Run')
        if self.running:self.after(1,self._run_tick)
    def _run_tick(self):
        if not self.running or not self.current_backend:return
        s=self.current_backend.run_steps(250 if self.current_backend.resolution=='cycle' else 50)
        self._refresh(s)
        if s.stopped:self.running=False; self.run_btn.config(text='Run'); return
        self.after(15,self._run_tick)

    def _close(self):
        self.running=False
        for b in self.backends.values():
            if hasattr(b,'close'):
                try:b.close()
                except Exception:pass
        self.destroy()


def main():
    ap=argparse.ArgumentParser(description='MiniMPU Studio V0.1.1')
    ap.add_argument('--backend',choices=['rtl','arch'],default='rtl')
    ap.add_argument('--autoclose-ms',type=int,default=0,help='test helper: close automatically')
    args=ap.parse_args()
    app=MiniMPUStudio()
    if args.backend=='arch': app._switch_backend('Architecture Model')
    if args.autoclose_ms: app.after(args.autoclose_ms,app._close)
    app.mainloop()
if __name__=='__main__':main()
