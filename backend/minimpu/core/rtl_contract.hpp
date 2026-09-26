#pragma once
#include <cstdint>
#include <string>
namespace minimpu {
struct RtlPlatformContract {
    static constexpr uint64_t reset_pc=0x80000000ull;
    static constexpr uint64_t kernel_pa=0x80200000ull;
    static constexpr uint64_t dtb_pa=0x83e00000ull;
    static constexpr uint64_t dram_base=0x80000000ull;
    static constexpr uint64_t dram_limit=0x84000000ull;
    static constexpr uint64_t clint=0x02000000ull;
    static constexpr uint64_t plic=0x0c000000ull;
    static constexpr uint64_t uart=0x10000000ull;
    static constexpr uint64_t simctrl=0x10010000ull;
};
inline std::string validate_rtl_linux_contract(){
    if(RtlPlatformContract::reset_pc<RtlPlatformContract::dram_base||RtlPlatformContract::reset_pc>=RtlPlatformContract::dram_limit)return "reset PC outside DRAM";
    if(RtlPlatformContract::kernel_pa<RtlPlatformContract::dram_base||RtlPlatformContract::kernel_pa>=RtlPlatformContract::dram_limit)return "kernel outside DRAM";
    if(RtlPlatformContract::dtb_pa<RtlPlatformContract::dram_base||RtlPlatformContract::dtb_pa>=RtlPlatformContract::dram_limit)return "DTB outside DRAM";
    if((RtlPlatformContract::kernel_pa&0x1fffffull)!=0)return "kernel must be 2 MiB aligned";
    return {};
}
} // namespace minimpu
