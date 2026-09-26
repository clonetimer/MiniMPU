#pragma once
#include "types.hpp"
#include <array>
#include <cstdint>

namespace minimpu {

class Pmp {
public:
    static constexpr unsigned kEntries = 8;
private:
    std::array<uint8_t, kEntries> cfg_{};
    std::array<uint64_t, kEntries> addr_{};

    static unsigned low_ones(uint64_t value) {
        unsigned count = 0;
        while ((value & 1ull) != 0 && count < 61) { ++count; value >>= 1; }
        return count;
    }

    bool range(unsigned index, uint64_t& lo, uint64_t& hi) const {
        const unsigned a = (cfg_[index] >> 3u) & 0x3u;
        if (a == 0) return false;
        if (a == 1) { // TOR
            lo = index == 0 ? 0 : (addr_[index - 1] << 2u);
            hi = addr_[index] << 2u;
            return hi > lo;
        }
        if (a == 2) { // NA4
            lo = addr_[index] << 2u;
            hi = lo + 4u;
            return true;
        }
        const unsigned ones = low_ones(addr_[index]); // NAPOT, minimum encoding is 8 bytes
        const uint64_t size = 1ull << (ones + 3u);
        const uint64_t encoded_mask = ones == 64 ? ~0ull : ((1ull << ones) - 1ull);
        lo = (addr_[index] & ~encoded_mask) << 2u;
        hi = lo + size;
        return true;
    }

public:
    uint8_t cfg(unsigned index) const { return cfg_.at(index); }
    uint64_t addr(unsigned index) const { return addr_.at(index); }

    void write_cfg(unsigned index, uint8_t value) {
        if (cfg_.at(index) & 0x80u) return;
        // Bits 6:5 are reserved/WARL. R=0,W=1 is a reserved collective WARL encoding;
        // this implementation maps it to W=0 while preserving the other legal fields.
        value &= uint8_t(~0x60u);
        if ((value & 0x2u) && !(value & 0x1u)) value &= uint8_t(~0x2u);
        cfg_[index] = value;
    }

    void write_addr(unsigned index, uint64_t value) {
        if (cfg_.at(index) & 0x80u) return;
        // If the next TOR entry is locked, this address also forms its locked lower bound.
        if (index + 1 < kEntries && (cfg_[index + 1] & 0x80u) && (((cfg_[index + 1] >> 3u) & 3u) == 1u)) return;
        addr_[index] = value;
    }

    bool check(uint64_t physical_address, unsigned size_bytes, AccessType access, PrivilegeMode mode) const {
        const uint64_t end = physical_address + size_bytes;
        if (end < physical_address) return false;
        for (unsigned index = 0; index < kEntries; ++index) {
            uint64_t lo = 0, hi = 0;
            if (!range(index, lo, hi)) continue;
            const bool overlaps = physical_address < hi && lo < end;
            if (!overlaps) continue;
            if (!(physical_address >= lo && end <= hi)) return false; // whole access must fit first matching entry
            if (mode == PrivilegeMode::Machine && !(cfg_[index] & 0x80u)) return true;
            const uint8_t required = access == AccessType::Fetch ? 0x4u : (access == AccessType::Store ? 0x2u : 0x1u);
            return (cfg_[index] & required) != 0;
        }
        return mode == PrivilegeMode::Machine;
    }
};

} // namespace minimpu
