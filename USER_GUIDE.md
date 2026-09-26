# MiniMPU Studio V0.1.1 — User Guide

## 0. 第一次使用：先完成五步入门
启动软件后不要切页面，也不要切 backend。保持默认 **RTL Replay**，在 Course 页从 `① 开始入门` 顺序做到 `⑤ 我来 Step Cycle`。

每一步只有一个当前可用按钮，并在页面底部给出“刚才发生了什么”的短解释。完成五步后再进入正式章节。

如果你第一次看到 `PC / Privilege / scause / stval`：
- `PC` = 当前执行位置；
- `Privilege` = 当前 M/S/U 权限级；
- `scause` = trap 原因；
- `sepc` = trap 前的指令位置；
- `stval` = 与 fault 相关的地址/值。

## 1. Backend selector
**RTL Replay**：最适合初学和 cycle-level MMU/TLB/PTW 学习。它回放真实 V3.3 Verilator 数据。

**Architecture Model**：最适合看实时寄存器、CSR、trap 行为。它是 instruction-atomic。

切换 backend 会重置所选 backend；两个 backend 之间不转换状态。

## 2. Five existing controls
**Reset** — 回到所选 backend 起点。

**Step Instruction** — 前进一个退休指令。

**Step Cycle** — RTL Replay 中前进一个真实测量 cycle；Architecture Model 中因为不存在 cycle-accurate pipeline，所以等价于一个 instruction step。

**Run / Pause** — 成批推进，同时保持 GUI 可响应。

**Run Until Event** — 跑到 privilege change、U-mode ECALL、page fault、TLB miss 或 SIM exit。初学时优先用它，不建议直接长时间 Run。

## 3. Six pages
**Course** — 第一次使用入口 + 六个重点实验。

**CPU** — PC、当前指令、privilege、cycle/instret、寄存器。

**Privilege / CSR** — xRET、ECALL、trap routing 的主观察页。

**MMU / TLB / PTW** — Sv39、TLB、page walk、page fault 的主观察页。

**Memory** — SoC 固定地址图与当前 VA/PA。

**Timeline** — RTL 测量事件和当前附近的 cycle；Architecture 模式显示 UART tail。

## 4. 推荐的正式学习顺序
完成五步入门后：

`04 Privilege → 07 Sv39 → 08 TLB → 16 Syscall → 17 Page Fault → 18 EduOS`

第一次学习时不要尝试一次理解所有页面。每一章只看课程要求的那几个状态。
