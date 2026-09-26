#pragma once
#include "bus.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>

namespace minimpu {

class Memory : public Device {
    std::vector<uint8_t> bytes_;
    bool guest_writable_;
public:
    explicit Memory(size_t size_bytes, bool guest_writable = true, uint8_t fill = 0)
        : bytes_(size_bytes, fill), guest_writable_(guest_writable) {}

    size_t size() const { return bytes_.size(); }
    const std::vector<uint8_t>& bytes() const { return bytes_; }

    uint64_t read(uint64_t offset, unsigned size_bytes) override {
        if (offset + size_bytes > bytes_.size()) throw BusFault(FaultKind::Unmapped, offset, "memory read out of range");
        uint64_t value = 0;
        for (unsigned i = 0; i < size_bytes; ++i) value |= uint64_t(bytes_[offset + i]) << (8u * i);
        return value;
    }

    void write(uint64_t offset, unsigned size_bytes, uint64_t value) override {
        if (!guest_writable_) throw BusFault(FaultKind::Denied, offset, "read-only memory");
        poke(offset, size_bytes, value);
    }

    void poke(uint64_t offset, unsigned size_bytes, uint64_t value) {
        if (offset + size_bytes > bytes_.size()) throw BusFault(FaultKind::Unmapped, offset, "memory write out of range");
        for (unsigned i = 0; i < size_bytes; ++i) bytes_[offset + i] = uint8_t(value >> (8u * i));
    }

    void load(uint64_t offset, const uint8_t* data, size_t length) {
        if (offset + length > bytes_.size()) throw std::out_of_range("memory load out of range");
        std::copy(data, data + length, bytes_.begin() + static_cast<std::ptrdiff_t>(offset));
    }

    void fill(uint8_t value = 0) { std::fill(bytes_.begin(), bytes_.end(), value); }
};

} // namespace minimpu
