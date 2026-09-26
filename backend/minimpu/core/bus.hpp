#pragma once
#include "types.hpp"
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace minimpu {

struct Device {
    virtual ~Device() = default;
    virtual uint64_t read(uint64_t offset, unsigned size_bytes) = 0;
    virtual void write(uint64_t offset, unsigned size_bytes, uint64_t value) = 0;
};

class Bus {
    struct Mapping {
        uint64_t base;
        uint64_t size;
        Device* device;
        std::string name;
    };
    std::vector<Mapping> mappings_;

    const Mapping& find(uint64_t address, unsigned size_bytes) const {
        if (!(size_bytes == 1 || size_bytes == 2 || size_bytes == 4 || size_bytes == 8))
            throw std::invalid_argument("invalid access size");
        if ((address & (size_bytes - 1u)) != 0)
            throw BusFault(FaultKind::Misaligned, address, "misaligned bus access");
        for (const auto& mapping : mappings_) {
            if (address >= mapping.base && address - mapping.base <= mapping.size - size_bytes)
                return mapping;
        }
        throw BusFault(FaultKind::Unmapped, address, "unmapped bus access");
    }

public:
    void map(uint64_t base, uint64_t size, Device& device, std::string name) {
        if (size == 0) throw std::invalid_argument("zero-sized mapping");
        if (base + size < base) throw std::invalid_argument("mapping overflow");
        for (const auto& existing : mappings_) {
            const bool overlap = base < existing.base + existing.size && existing.base < base + size;
            if (overlap) throw std::invalid_argument("overlapping bus mapping");
        }
        mappings_.push_back({base, size, &device, std::move(name)});
        std::sort(mappings_.begin(), mappings_.end(), [](const Mapping& a, const Mapping& b){ return a.base < b.base; });
    }

    uint64_t read(uint64_t address, unsigned size_bytes) {
        const auto& mapping = find(address, size_bytes);
        return mapping.device->read(address - mapping.base, size_bytes) & mask_for_size(size_bytes);
    }

    void write(uint64_t address, unsigned size_bytes, uint64_t value) {
        const auto& mapping = find(address, size_bytes);
        mapping.device->write(address - mapping.base, size_bytes, value & mask_for_size(size_bytes));
    }

    bool mapped(uint64_t address, unsigned size_bytes = 1) const {
        try { (void)find(address, size_bytes); return true; }
        catch (const BusFault&) { return false; }
    }
};

} // namespace minimpu
