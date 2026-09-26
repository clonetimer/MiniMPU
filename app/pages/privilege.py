import tkinter as tk
from tkinter import ttk

CSR_ORDER=["mstatus","medeleg","mideleg","mie","mtvec","mepc","mcause","mtval","stvec","sepc","scause","stval","satp"]

class PrivilegePage(ttk.Frame):
    def __init__(self,parent):
        super().__init__(parent,padding=14)
        ttk.Label(self,text="Privilege / CSR",style="Title.TLabel").pack(anchor="w")
        body=ttk.Frame(self); body.pack(fill="both",expand=True,pady=(8,0))
        self.canvas=tk.Canvas(body,width=360,height=430,bg="#f7f7f7",highlightthickness=1,highlightbackground="#bbb")
        self.canvas.pack(side="left",fill="y",padx=(0,12))
        self.nodes={}
        for mode,y,label in [('M',70,'M-mode\nMachine'),('S',210,'S-mode\nSupervisor'),('U',350,'U-mode\nUser')]:
            oval=self.canvas.create_oval(90,y-38,270,y+38,fill="#e5e7eb",outline="#374151",width=2)
            text=self.canvas.create_text(180,y,text=label,font=('TkDefaultFont',12,'bold'))
            self.nodes[mode]=(oval,text)
        self.canvas.create_line(180,108,180,172,arrow='both',width=2)
        self.canvas.create_text(245,140,text='MRET / trap')
        self.canvas.create_line(180,248,180,312,arrow='both',width=2)
        self.canvas.create_text(245,280,text='SRET / ECALL')
        right=ttk.Frame(body); right.pack(side='left',fill='both',expand=True)
        self.mode=ttk.Label(right,text="",style="Mono.TLabel"); self.mode.pack(anchor='w',pady=(0,8))
        self.tree=ttk.Treeview(right,columns=('csr','value'),show='headings',height=15)
        self.tree.heading('csr',text='CSR'); self.tree.heading('value',text='Value')
        self.tree.column('csr',width=120); self.tree.column('value',width=240)
        self.tree.pack(fill='both',expand=True)
    def update_snapshot(self,s,backend):
        self.mode.config(text=f"Current privilege = {s.privilege}    PC = 0x{s.pc:016x}    event = {s.event or '-'}")
        for mode,(oval,_) in self.nodes.items(): self.canvas.itemconfig(oval,fill="#fde68a" if mode==s.privilege else "#e5e7eb")
        self.tree.delete(*self.tree.get_children())
        for name in CSR_ORDER:
            v=s.csrs.get(name)
            self.tree.insert('', 'end', values=(name, '-' if v is None else f"0x{v:016x}"))
