#pragma once
#include "types.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace minimpu {

struct LinuxBootState {
    PrivilegeMode mode = PrivilegeMode::Supervisor;
    uint64_t hartid_a0 = 0;
    uint64_t dtb_a1 = 0;
    uint64_t satp = 0;
    uint64_t kernel_physical_address = 0x80200000ull;
};

inline std::vector<std::string> validate_linux_boot_state(const LinuxBootState& s) {
    std::vector<std::string> errors;
    if (s.mode != PrivilegeMode::Supervisor) errors.emplace_back("Linux S-mode entry must run in Supervisor mode");
    if (s.hartid_a0 != 0) errors.emplace_back("single-hart MiniMPU expects a0/hartid = 0");
    if (s.dtb_a1 == 0 || (s.dtb_a1 & 7u)) errors.emplace_back("a1 must point to an aligned DTB in memory");
    if (s.satp != 0) errors.emplace_back("satp must be zero before Linux installs its own page tables");
    if ((s.kernel_physical_address & ((2ull<<20u)-1u)) != 0) errors.emplace_back("RV64 Linux Image should be loaded on a 2 MiB boundary");
    return errors;
}

} // namespace minimpu
