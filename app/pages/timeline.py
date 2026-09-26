from tkinter import ttk

class TimelinePage(ttk.Frame):
    def __init__(self,parent,events):
        super().__init__(parent,padding=14)
        ttk.Label(self,text="Timeline / RTL Evidence",style="Title.TLabel").pack(anchor='w')
        panes=ttk.Panedwindow(self,orient='vertical'); panes.pack(fill='both',expand=True,pady=(8,0))
        top=ttk.Frame(panes); bottom=ttk.Frame(panes); panes.add(top,weight=1); panes.add(bottom,weight=2)
        self.events_tree=ttk.Treeview(top,columns=('cycle','event','priv','pc'),show='headings',height=7)
        for c,h,w in [('cycle','Cycle',100),('event','Event',240),('priv','Priv',60),('pc','PC',180)]: self.events_tree.heading(c,text=h); self.events_tree.column(c,width=w)
        for e in events: self.events_tree.insert('', 'end', values=(e.get('cycle'),e.get('event'),e.get('priv'),e.get('pc')))
        self.events_tree.pack(fill='both',expand=True)
        self.rows=ttk.Treeview(bottom,columns=('mark','cycle','instret','priv','pc','core','i_mmu','i_ptw','d_mmu','d_ptw','scause'),show='headings',height=12)
        widths={'mark':35,'cycle':75,'instret':75,'priv':45,'pc':130,'core':70,'i_mmu':70,'i_ptw':70,'d_mmu':70,'d_ptw':70,'scause':70}
        for c in self.rows['columns']: self.rows.heading(c,text=c); self.rows.column(c,width=widths[c],anchor='w')
        self.rows.pack(fill='both',expand=True)
        self.uart=ttk.Label(bottom,text="",wraplength=1000); self.uart.pack(anchor='w',pady=(6,0))
    def update_snapshot(self,s,backend):
        self.rows.delete(*self.rows.get_children())
        rows=getattr(backend,'nearby_rows',lambda radius=6:[])()
        for idx,r in rows:
            mark='▶' if getattr(backend,'index',-1)==idx else ''
            self.rows.insert('', 'end', values=(mark,r.get('cycle'),r.get('instret'),r.get('priv'),r.get('pc'),r.get('core_state'),r.get('immu_state'),r.get('iptw_state'),r.get('dmmu_state'),r.get('dptw_state'),r.get('scause')))
        tail=s.uart[-160:].replace('\n',' ⏎ ') if s.uart else ''
        self.uart.config(text=("UART tail: "+tail) if tail else "RTL replay: table shows measured cycles around the current cursor. Architecture mode shows live UART here.")
