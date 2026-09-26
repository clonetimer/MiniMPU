from tkinter import ttk

MAP=[
    ('MROM','0x0000_1000','64 KiB','boot/ROM window'),
    ('CLINT','0x0200_0000','64 KiB','software/timer interrupt'),
    ('PLIC','0x0c00_0000','4 MiB','external interrupt controller'),
    ('UART16550','0x1000_0000','256 B','serial MMIO'),
    ('SIMCTRL','0x1001_0000','4 KiB','simulation exit'),
    ('DRAM','0x8000_0000','64 MiB','code/data/page tables'),
]
class MemoryPage(ttk.Frame):
    def __init__(self,parent):
        super().__init__(parent,padding=14)
        ttk.Label(self,text="Memory / Address Space",style="Title.TLabel").pack(anchor='w')
        self.current=ttk.Label(self,text="",style='Mono.TLabel'); self.current.pack(anchor='w',pady=(5,10))
        tree=ttk.Treeview(self,columns=('name','base','size','use'),show='headings',height=8)
        for c,h,w in [('name','Region',130),('base','Base',160),('size','Size',100),('use','Purpose',420)]: tree.heading(c,text=h); tree.column(c,width=w)
        for row in MAP: tree.insert('', 'end', values=row)
        tree.pack(fill='x')
        ttk.Label(self,text="EduOS teaching mapping: kernel identity @ 0x80000000; user code VA 0x40000000 → PA 0x80200000; user stack VA 0x40010000 → PA 0x80300000.",wraplength=1000).pack(anchor='w',pady=(14,0))
    def update_snapshot(self,s,backend):
        self.current.config(text=f"PC/VA=0x{s.pc:016x}   I-PA={'0x%016x'%s.i_pa if s.i_pa else '-'}   D-VA={'0x%016x'%s.d_va if s.d_va else '-'}   D-PA={'0x%016x'%s.d_pa if s.d_pa else '-'}")
