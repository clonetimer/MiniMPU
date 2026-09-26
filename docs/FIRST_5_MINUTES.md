# First 5 Minutes — MiniMPU Studio V0.1.1

这份练习只解决一件事：**先学会怎么操作软件，再开始学 MPU。**

## 0. 启动

```bash
./run_studio.sh
```

保持默认：

- Backend = `RTL Replay`
- Page = `Course ← 从这里开始`

不要先点顶部工具栏。

## 1. 点击 `① 开始入门`
预期看到：

- `M-mode`
- `PC = 0x80000000`
- 进度 `1 / 5`
- 第 ② 步解锁

先记住：**PC 是“现在执行到哪里”，M/S/U 是“现在以什么权限执行”。**

## 2. 点击 `② 看 M→S`
软件自动跑到第一次权限切换。

预期：

- privilege 从 `M` 变成 `S`
- 软件报告对应 cycle 和 PC

此时不用研究所有 CSR，只理解“机器态把执行权交给了监督态”。

## 3. 点击 `③ 看 ECALL`
软件自动跑到 U-mode 发出系统调用之后。

预期重点：

- 当前进入 `S-mode`
- `scause = 8`
- `sepc = 0x40000010`（当前教学镜像）

含义：U-mode 的 ECALL 触发了 S-mode trap。

## 4. 点击 `④ 看 Page Fault`
软件自动跑到第一次 page fault。

重点只看两个字段：

- `scause`：哪一类 fault
- `stval`：哪个地址触发 fault

这是后续学习内存保护时最重要的故障入口。

## 5. 点击 `⑤ 我来 Step Cycle`
软件先 Reset，然后推进一个真实 RTL cycle。

你现在已经知道顶部三个最重要的操作：

- `Reset`
- `Run Until Event`
- `Step Cycle`

## 6. 入门完成后
建议按顺序学习：

`Chapter 04 → 07 → 08 → 16 → 17 → 18`

第一次不要同时看所有页面。每章只打开课程要求的观察页。

---

参考截图：

- `verification/studio_v0_1_1_onboarding.png`
- `verification/studio_v0_1_1_onboarding_complete.png`
