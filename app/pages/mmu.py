import tkinter as tk
from tkinter import ttk

class MMUPage(ttk.Frame):
    def __init__(self,parent):
        super().__init__(parent,padding=14)
        ttk.Label(self,text="MMU / TLB / PTW",style="Title.TLabel").pack(anchor="w")
        self.summary=ttk.Label(self,text="",style="Mono.TLabel",wraplength=1040); self.summary.pack(anchor='w',pady=(5,8))
        self.canvas=tk.Canvas(self,height=300,bg='#f7f7f7',highlightthickness=1,highlightbackground='#bbb')
        self.canvas.pack(fill='x')
        self.labels={}
        boxes=[('VA',90),('TLB',290),('PTW',490),('PTE',690),('PA',890)]
        for name,x in boxes:
            self.canvas.create_rectangle(x-65,95,x+65,165,fill='#e5e7eb',outline='#374151',width=2)
            self.canvas.create_text(x,115,text=name,font=('TkDefaultFont',11,'bold'))
            tid=self.canvas.create_text(x,145,text='-',font=('TkFixedFont',9)); self.labels[name]=tid
        for x1,x2 in [(155,225),(355,425),(555,625),(755,825)]: self.canvas.create_line(x1,130,x2,130,arrow='last',width=2)
        self.detail=ttk.Label(self,text="",wraplength=1040); self.detail.pack(anchor='w',pady=(10,0))
    def update_snapshot(self,s,backend):
        satp=s.csrs.get('satp',0)
        self.summary.config(text=f"SATP=0x{satp:016x}   I-MMU={s.immu_state or 'architectural'}   D-MMU={s.dmmu_state or 'architectural'}   I-TLB hit={int(s.itlb_hit)}   D-TLB hit={int(s.dtlb_hit)}")
        va=s.d_va or s.i_va or s.pc; pa=s.d_pa or s.i_pa
        self.canvas.itemconfig(self.labels['VA'],text=f"0x{va:x}")
        self.canvas.itemconfig(self.labels['TLB'],text=("I hit" if s.itlb_hit else "D hit" if s.dtlb_hit else "miss/unknown"))
        ptw=(s.dptw_state if s.dptw_state and s.dptw_state!='IDLE' else s.iptw_state) or '-'
        level=s.dptw_level if s.dptw_state and s.dptw_state!='IDLE' else s.iptw_level
        self.canvas.itemconfig(self.labels['PTW'],text=f"{ptw} L{level}")
        paddr=s.dpte_addr or s.ipte_addr; pte=s.dpte or s.ipte
        self.canvas.itemconfig(self.labels['PTE'],text=f"@{paddr:#x}\n{pte:#x}" if paddr else '-')
        self.canvas.itemconfig(self.labels['PA'],text=f"0x{pa:x}" if pa else '-')
        if s.dmem_pf or s.imem_pf or s.csrs.get('scause') in (12,13,15):
            self.detail.config(text=f"PAGE FAULT observable: scause={s.csrs.get('scause',0)}  stval=0x{s.csrs.get('stval',0):x}.  Compare TLB-hit vs PTW path before concluding the fault reason.")
        else:
            self.detail.config(text="RTL Replay shows measured TLB/PTW state cycle-by-cycle. Architecture mode is instruction-atomic, so microarchitectural PTW timing is intentionally not invented.")
