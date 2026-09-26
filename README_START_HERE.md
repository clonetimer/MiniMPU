# MiniMPU Studio V0.1.1 — Start Here

MiniMPU Studio is an interactive teaching application built on the existing MiniMPU EduOS/reference/RTL evidence.

## 第一次打开：不要先研究工具栏
在验证过的 Linux 环境里运行：

```bash
./run_studio.sh
```

软件默认停在左侧 **`Course ← 从这里开始`**。

你只需要按顺序完成页面里的五个按钮：

1. `① 开始入门`
2. `② 看 M→S`
3. `③ 看 ECALL`
4. `④ 看 Page Fault`
5. `⑤ 我来 Step Cycle`

后面的按钮在前一步完成前会保持灰色，所以第一次使用不需要自己判断“下一步点什么”。整个过程约 5 分钟。

完成后再从 Chapter 04 / 07 / 08 / 16 / 17 / 18 里选择正式实验。

## 你在五步里学会的不是 MPU 理论，而是软件操作
- **Reset**：回到仿真起点；
- **Run Until Event**：不用盲目 Run，直接跑到关键事件；
- **Step Cycle**：RTL Replay 中逐拍推进真实 RTL 记录；
- **PC**：当前执行位置；
- **Privilege**：当前处于 M / S / U 哪个权限级；
- **scause / sepc / stval**：发生 trap 后最先看的三个信息。

## 两个 backend 什么时候用
初学时保持默认 **RTL Replay**。它加载 V3.3 真实 Verilator 的 63,409 个 cycle 和 25 个测量事件，适合逐 cycle 看 MMU/TLB/PTW。

等你完成入门后，再切换 **Architecture Model**；它实时运行 C++ reference model，更适合看寄存器和 CSR，但它是 instruction-atomic，不会伪造流水线 cycle。

## Scope
`DESIGN_BASELINE.md` 仍是 V0.1 的冻结产品边界。`ONBOARDING_PATCH.md` 说明 V0.1.1 为什么属于可用性修复而不是功能扩张。`FUTURE.md` 仍只存放以后版本的想法。

如果是课堂第一次使用，也可以直接照着 `docs/FIRST_5_MINUTES.md` 做；其中列出了每一步应该看到的结果。
