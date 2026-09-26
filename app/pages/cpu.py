from tkinter import ttk

class CPUPage(ttk.Frame):
    def __init__(self,parent):
        super().__init__(parent,padding=14)
        ttk.Label(self,text="CPU",style="Title.TLabel").pack(anchor="w")
        self.summary=ttk.Label(self,text="",style="Mono.TLabel"); self.summary.pack(anchor="w",pady=(5,10))
        self.insn=ttk.Label(self,text="",style="Mono.TLabel",wraplength=980); self.insn.pack(anchor="w",pady=(0,10))
        self.tree=ttk.Treeview(self,columns=("r1","v1","r2","v2","r3","v3","r4","v4"),show="headings",height=8)
        heads=["Reg","Value"]*4
        for c,h in zip(self.tree['columns'],heads): self.tree.heading(c,text=h); self.tree.column(c,width=105 if h=='Value' else 45,anchor='w')
        self.tree.pack(fill="x")
    def update_snapshot(self,s,backend):
        self.summary.config(text=f"backend={s.backend}   resolution={s.resolution}   cycle={s.cycle}   instret={s.instret}   priv={s.privilege}   PC=0x{s.pc:016x}")
        self.insn.config(text=f"Instruction: {s.instruction or '(disassembly not available at this PC)'}")
        self.tree.delete(*self.tree.get_children())
        regs=s.regs if s.regs else [0]*32
        rtl_unavailable = s.backend == 'RTL Replay'
        for row in range(8):
            vals=[]
            for bank in range(4):
                idx=row+bank*8
                vals.extend([f"x{idx}", '—' if rtl_unavailable else f"0x{regs[idx]:016x}"])
            self.tree.insert('', 'end', values=vals)
