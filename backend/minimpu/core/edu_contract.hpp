#pragma once
#include <cstdint>

namespace minimpu::edu {
constexpr std::uint64_t DRAM_BASE      = 0x80000000ull;
constexpr std::uint64_t KERNEL_ENTRY   = 0x80000000ull;
constexpr std::uint64_t ROOT_PT_PA     = 0x80100000ull;
constexpr std::uint64_t MMIO_L1_PA     = 0x80101000ull;
constexpr std::uint64_t USER_L1_PA     = 0x80102000ull;
constexpr std::uint64_t USER_L0_PA     = 0x80103000ull;
constexpr std::uint64_t USER_CODE_PA   = 0x80200000ull;
constexpr std::uint64_t USER_STACK_PA  = 0x80300000ull;
constexpr std::uint64_t USER_CODE_VA   = 0x40000000ull;
constexpr std::uint64_t USER_STACK_VA  = 0x40010000ull;
constexpr std::uint64_t USER_STACK_TOP = USER_STACK_VA + 0x1000ull;
constexpr std::uint64_t UART_BASE      = 0x10000000ull;
constexpr std::uint64_t SIMCTRL_BASE   = 0x10010000ull;
constexpr std::uint64_t BAD_VA         = 0x50000000ull;

enum Syscall : std::uint64_t {
    SYS_WRITE = 1,
    SYS_GETPID = 2,
    SYS_EXIT = 3,
};
}
