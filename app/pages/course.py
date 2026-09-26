import tkinter as tk
from tkinter import ttk
from app.course_data import BEGINNER_STEPS, LESSONS


class CoursePage(ttk.Frame):
    """Course page with a deliberately small first-run path.

    The onboarding does not add a new simulator command.  It calls the same
    Reset / Run Until / Step Cycle operations exposed by the V0.1 toolbar.
    """

    def __init__(self, parent, open_page, beginner_action):
        super().__init__(parent, padding=14)
        self.open_page = open_page
        self.beginner_action = beginner_action
        self.completed = set()
        self.step_widgets = {}

        ttk.Label(self, text="MiniMPU Studio — 从这里开始", style="Title.TLabel").pack(anchor="w")
        ttk.Label(
            self,
            text="第一次打开时不要先研究所有按钮。先按顺序完成下面 5 步；每一步软件都会自动跑到关键位置并告诉你该看什么。",
            wraplength=980,
        ).pack(anchor="w", pady=(4, 8))

        intro = ttk.LabelFrame(self, text="5 分钟入门 · 先学会怎么操作", padding=10)
        intro.pack(fill="x", pady=(0, 10))

        glossary = ttk.Frame(intro)
        glossary.pack(fill="x", pady=(0, 8))
        for term, desc in [
            ("PC", "当前正在执行的位置"),
            ("Privilege", "M / S / U 当前权限级"),
            ("Event", "ECALL、Page Fault 等关键事件"),
        ]:
            box = ttk.LabelFrame(glossary, text=term, padding=(8, 4))
            box.pack(side="left", fill="x", expand=True, padx=(0, 6))
            ttk.Label(box, text=desc, wraplength=250).pack(anchor="w")

        self.progress_var = tk.StringVar(value="入门进度：0 / 5")
        ttk.Label(intro, textvariable=self.progress_var, font=("TkDefaultFont", 10, "bold")).pack(anchor="w", pady=(0, 5))

        steps = ttk.Frame(intro)
        steps.pack(fill="x")
        for i, step in enumerate(BEGINNER_STEPS, start=1):
            row = ttk.Frame(steps, padding=(0, 3))
            row.pack(fill="x")
            status = tk.StringVar(value="○")
            ttk.Label(row, textvariable=status, width=2, font=("TkDefaultFont", 11, "bold")).pack(side="left", anchor="n")
            text = ttk.Frame(row)
            text.pack(side="left", fill="x", expand=True)
            ttk.Label(text, text=f"{i}. {step['title']}", font=("TkDefaultFont", 10, "bold")).pack(anchor="w")
            ttk.Label(text, text=step["text"], wraplength=760).pack(anchor="w")
            button = ttk.Button(row, text=step["button"], command=lambda k=step["key"]: self._run_beginner(k))
            button.pack(side="right", padx=(8, 0))
            self.step_widgets[step["key"]] = (status, button)
            if i > 1:
                button.state(["disabled"])

        self.beginner_result = tk.StringVar(value="提示：从“① 开始入门”开始，不需要提前理解 MMU、CSR 或页表。")
        ttk.Label(intro, textvariable=self.beginner_result, wraplength=980).pack(anchor="w", pady=(8, 0))

        ttk.Separator(self).pack(fill="x", pady=6)
        ttk.Label(self, text="完成 5 步后，再选一个重点实验", font=("TkDefaultFont", 11, "bold")).pack(anchor="w", pady=(3, 5))

        grid = ttk.Frame(self)
        grid.pack(fill="both", expand=True)
        for i, lesson in enumerate(LESSONS):
            box = ttk.LabelFrame(grid, text=f"Chapter {lesson['chapter']}  {lesson['title']}", padding=8)
            box.grid(row=i//3, column=i%3, sticky="nsew", padx=4, pady=4)
            row = ttk.Frame(box)
            row.pack(fill="both", expand=True)
            ttk.Label(row, text=lesson['goal'], wraplength=235).pack(side="left", fill="x", expand=True, anchor="w")
            ttk.Button(row, text="进入实验", width=10, command=lambda p=lesson['page']: open_page(p)).pack(side="right", padx=(6, 0))
        for c in range(3):
            grid.columnconfigure(c, weight=1)
        for r in range(2):
            grid.rowconfigure(r, weight=1)

        self.live_var = tk.StringVar(value="当前状态：尚未开始")

    def _run_beginner(self, key):
        result = self.beginner_action(key)
        if result:
            self.mark_completed(key, result)

    def mark_completed(self, key, message=""):
        self.completed.add(key)
        widget = self.step_widgets.get(key)
        if widget:
            status, button = widget
            status.set("✓")
            button.state(["disabled"])
        keys = [step["key"] for step in BEGINNER_STEPS]
        if key in keys:
            next_index = keys.index(key) + 1
            if next_index < len(keys):
                self.step_widgets[keys[next_index]][1].state(["!disabled"])
        self.progress_var.set(f"入门进度：{len(self.completed)} / {len(BEGINNER_STEPS)}")
        if message:
            self.beginner_result.set(message)
        if len(self.completed) == len(BEGINNER_STEPS):
            self.beginner_result.set("✓ 入门完成：你已经会 Reset、Run Until Event 和 Step Cycle。现在可以从 Chapter 04 / 07 / 16 / 17 中任选一章继续。")

    def reset_beginner(self):
        self.completed.clear()
        for status, button in self.step_widgets.values():
            status.set("○")
            button.state(["disabled"])
        self.step_widgets[BEGINNER_STEPS[0]["key"]][1].state(["!disabled"])
        self.progress_var.set("入门进度：0 / 5")
        self.beginner_result.set("提示：从“① 开始入门”开始，不需要提前理解 MMU、CSR 或页表。")

    def update_snapshot(self, snapshot, backend):
        if not snapshot:
            return
        extra = f" | event={snapshot.event}" if snapshot.event else ""
        self.live_var.set(
            f"当前状态：{snapshot.backend} | cycle {snapshot.cycle} | {snapshot.privilege}-mode | PC 0x{snapshot.pc:016x}{extra}"
        )
