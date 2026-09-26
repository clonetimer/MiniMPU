#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>

namespace minimpu {

enum class PrivilegeMode : uint8_t { User = 0, Supervisor = 1, Machine = 3 };
enum class AccessType : uint8_t { Fetch, Load, Store };

enum class FaultKind : uint8_t { Unmapped, Misaligned, Denied };

struct BusFault : std::runtime_error {
    FaultKind kind;
    uint64_t address;
    BusFault(FaultKind kind_value, uint64_t address_value, const std::string& message)
        : std::runtime_error(message), kind(kind_value), address(address_value) {}
};

struct PageFault : std::runtime_error {
    AccessType access;
    uint64_t virtual_address;
    PageFault(AccessType access_value, uint64_t address_value, const std::string& message)
        : std::runtime_error(message), access(access_value), virtual_address(address_value) {}
};

struct AccessFault : std::runtime_error {
    AccessType access;
    uint64_t virtual_address;
    AccessFault(AccessType access_value, uint64_t address_value, const std::string& message)
        : std::runtime_error(message), access(access_value), virtual_address(address_value) {}
};

inline uint64_t mask_for_size(unsigned size_bytes) {
    switch (size_bytes) {
    case 1: return 0xffull;
    case 2: return 0xffffull;
    case 4: return 0xffffffffull;
    case 8: return ~0ull;
    default: throw std::invalid_argument("access size must be 1/2/4/8 bytes");
    }
}

inline int64_t sign_extend64(uint64_t value, unsigned bits) {
    const uint64_t sign = 1ull << (bits - 1u);
    const uint64_t mask = bits == 64 ? ~0ull : ((1ull << bits) - 1ull);
    value &= mask;
    return static_cast<int64_t>((value ^ sign) - sign);
}

inline uint64_t sext32(uint32_t value) {
    return static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(value)));
}

} // namespace minimpu
