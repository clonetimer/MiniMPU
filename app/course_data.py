LESSONS = [
    {"chapter":"04", "title":"M/S/U Privilege", "page":"Privilege / CSR", "goal":"看懂 M→S→U 状态切换以及 xRET 对权限级的影响。", "checkpoint":"能解释当前 privilege、mstatus.MPP/SPP 与返回目标。"},
    {"chapter":"07", "title":"Sv39 MMU", "page":"MMU / TLB / PTW", "goal":"把 VA 拆成 VPN[2:0]，沿三级页表得到 PA。", "checkpoint":"能解释 VA 0x40000000 为什么映射到 PA 0x80200000。"},
    {"chapter":"08", "title":"TLB / SFENCE.VMA", "page":"MMU / TLB / PTW", "goal":"区分 TLB hit 与 page walk，并理解 SFENCE.VMA。", "checkpoint":"能从 trace 判断当前访问是否发生 PTW。"},
    {"chapter":"16", "title":"Syscall / Trap", "page":"Privilege / CSR", "goal":"跟踪 U-mode ECALL 到 S-mode trap handler。", "checkpoint":"能读 scause/sepc/stval 并解释为何返回 sepc+4。"},
    {"chapter":"17", "title":"Page Fault / Isolation", "page":"MMU / TLB / PTW", "goal":"区分无映射 fault 与权限 fault。", "checkpoint":"能解释 scause=13/15 与 stval 的含义。"},
    {"chapter":"18", "title":"Full EduOS", "page":"Timeline", "goal":"串起 M→S→U、syscall、page fault 与退出。", "checkpoint":"能独立复述完整 EduOS 执行链。"},
]

BEGINNER_STEPS = [
    {
        "key": "start",
        "title": "从起点开始",
        "text": "切到 RTL Replay 并复位。先只认两个东西：PC 是当前执行位置，M/S/U 是当前权限级。",
        "button": "① 开始入门",
    },
    {
        "key": "privilege",
        "title": "看第一次权限切换",
        "text": "软件自动跑到第一次 M→S，并把关键状态显示出来。你只需要观察：privilege 从 M 变成 S。",
        "button": "② 看 M→S",
    },
    {
        "key": "ecall",
        "title": "看一次系统调用",
        "text": "自动跑到 U-mode 执行 ECALL 后进入 S-mode。重点看 scause=8 和 sepc。",
        "button": "③ 看 ECALL",
    },
    {
        "key": "fault",
        "title": "看保护机制生效",
        "text": "自动跑到第一次 page fault。重点看 scause 和 stval：它们告诉你“为什么错、哪个地址错”。",
        "button": "④ 看 Page Fault",
    },
    {
        "key": "step",
        "title": "自己走一步",
        "text": "先复位，再亲手推进 1 个 RTL cycle，理解顶部 Step Cycle 的用途。",
        "button": "⑤ 我来 Step Cycle",
    },
]
