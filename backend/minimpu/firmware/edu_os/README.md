# EduOS firmware

This is the teaching payload for chapters 14-18. It is deliberately small enough to read end-to-end.

Execution path:

`M reset -> PMP/delegation -> MRET -> S kernel -> build Sv39 -> SATP/SFENCE -> SRET -> U program -> ECALL/page fault -> S trap -> SRET -> U -> SYS_EXIT`

Physical/virtual layout:

- kernel: PA/VA `0x80000000` (1 GiB S-only identity superpage)
- page tables: `0x80100000..0x80103fff`
- user code: PA `0x80200000`, VA `0x40000000` (U, R/X)
- user stack: PA `0x80300000`, VA `0x40010000` (U, R/W)
- UART/SIMCTRL: S-only 2 MiB mapping at VA=PA `0x10000000`

The user program has no UART mapping. Printing is therefore a real `ECALL` syscall, not a user-mode MMIO shortcut.
