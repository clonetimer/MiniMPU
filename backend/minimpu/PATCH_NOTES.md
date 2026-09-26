# Studio adapter notes

The bundled C++ reference-model copy changes the teaching-only `SimControl` device to accept both 32-bit and 64-bit accesses at offset 0. The RTL SIMCTRL already accepts a 64-bit bus beat and only requires byte-lane 0. EduOS writes this register through a `volatile uint64_t*`, so this adapter aligns the reference-model peripheral access width with the measured RTL behavior. No ISA, privilege, MMU, PMP or architectural state semantics are changed.
