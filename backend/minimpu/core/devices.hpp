#pragma once
#include "bus.hpp"
#include <array>
#include <cstdint>
#include <deque>
#include <functional>
#include <stdexcept>

namespace minimpu {

class Clint : public Device {
    uint64_t mtime_ = 0;
    uint64_t mtimecmp_ = ~0ull;
    uint32_t msip_ = 0;
public:
    void tick(uint64_t ticks = 1) { mtime_ += ticks; }
    bool software_irq() const { return (msip_ & 1u) != 0; }
    bool timer_irq() const { return mtime_ >= mtimecmp_; }
    uint64_t mtime() const { return mtime_; }
    uint64_t mtimecmp() const { return mtimecmp_; }
    void set_mtimecmp(uint64_t value) { mtimecmp_ = value; }

    uint64_t read(uint64_t offset, unsigned size) override {
        if (offset == 0 && size == 4) return msip_;
        if (offset == 0x4000 && size == 8) return mtimecmp_;
        if (offset == 0xbff8 && size == 8) return mtime_;
        if (offset == 0x4000 && size == 4) return uint32_t(mtimecmp_);
        if (offset == 0x4004 && size == 4) return uint32_t(mtimecmp_ >> 32u);
        if (offset == 0xbff8 && size == 4) return uint32_t(mtime_);
        if (offset == 0xbffc && size == 4) return uint32_t(mtime_ >> 32u);
        throw BusFault(FaultKind::Denied, offset, "unsupported CLINT read");
    }
    void write(uint64_t offset, unsigned size, uint64_t value) override {
        if (offset == 0 && size == 4) { msip_ = uint32_t(value) & 1u; return; }
        if (offset == 0x4000 && size == 8) { mtimecmp_ = value; return; }
        if (offset == 0x4000 && size == 4) { mtimecmp_ = (mtimecmp_ & 0xffffffff00000000ull) | uint32_t(value); return; }
        if (offset == 0x4004 && size == 4) { mtimecmp_ = (mtimecmp_ & 0xffffffffull) | (uint64_t(uint32_t(value)) << 32u); return; }
        if (offset == 0xbff8 && size == 8) { mtime_ = value; return; }
        if (offset == 0xbff8 && size == 4) { mtime_ = (mtime_ & 0xffffffff00000000ull) | uint32_t(value); return; }
        if (offset == 0xbffc && size == 4) { mtime_ = (mtime_ & 0xffffffffull) | (uint64_t(uint32_t(value)) << 32u); return; }
        throw BusFault(FaultKind::Denied, offset, "unsupported CLINT write");
    }
};

class Plic : public Device {
    static constexpr unsigned kSources = 32;
    std::array<uint32_t, kSources> priority_{};
    uint32_t pending_ = 0;
    std::array<uint32_t, 2> enable_{};    // context0 M, context1 S
    std::array<uint32_t, 2> threshold_{};
    std::array<uint32_t, 2> active_{};

    unsigned best(unsigned context) const {
        unsigned best_id = 0; uint32_t best_priority = 0;
        uint32_t candidates = pending_ & enable_[context];
        for (unsigned id = 1; id < kSources; ++id) {
            if (!(candidates & (1u << id))) continue;
            if (priority_[id] > threshold_[context] && (priority_[id] > best_priority || (priority_[id] == best_priority && id < best_id))) {
                best_id = id; best_priority = priority_[id];
            }
        }
        return best_id;
    }
    unsigned claim(unsigned context) {
        const unsigned id = best(context);
        if (id) { pending_ &= ~(1u << id); active_[context] |= 1u << id; }
        return id;
    }
public:
    void pulse(unsigned source) { if (source == 0 || source >= kSources) throw std::invalid_argument("PLIC source"); pending_ |= 1u << source; }
    bool irq_m() const { return best(0) != 0; }
    bool irq_s() const { return best(1) != 0; }
    void set_priority(unsigned source, uint32_t priority) { priority_.at(source) = priority & 7u; }
    void set_enable(unsigned context, uint32_t bits) { enable_.at(context) = bits & ~1u; }

    uint64_t read(uint64_t offset, unsigned size) override {
        if (size != 4) throw BusFault(FaultKind::Denied, offset, "PLIC uses 32-bit registers");
        if (offset < 0x80 && (offset & 3u) == 0) return priority_[offset / 4u];
        if (offset == 0x1000) return pending_;
        if (offset == 0x2000) return enable_[0];
        if (offset == 0x2080) return enable_[1];
        if (offset == 0x200000) return threshold_[0];
        if (offset == 0x200004) return claim(0);
        if (offset == 0x201000) return threshold_[1];
        if (offset == 0x201004) return claim(1);
        throw BusFault(FaultKind::Denied, offset, "unsupported PLIC read");
    }
    void write(uint64_t offset, unsigned size, uint64_t value) override {
        if (size != 4) throw BusFault(FaultKind::Denied, offset, "PLIC uses 32-bit registers");
        if (offset < 0x80 && (offset & 3u) == 0) { priority_[offset / 4u] = uint32_t(value) & 7u; return; }
        if (offset == 0x2000) { enable_[0] = uint32_t(value) & ~1u; return; }
        if (offset == 0x2080) { enable_[1] = uint32_t(value) & ~1u; return; }
        if (offset == 0x200000) { threshold_[0] = uint32_t(value) & 7u; return; }
        if (offset == 0x201000) { threshold_[1] = uint32_t(value) & 7u; return; }
        if (offset == 0x200004 || offset == 0x201004) {
            const unsigned context = offset == 0x200004 ? 0 : 1;
            const unsigned id = unsigned(value);
            if (id < kSources) active_[context] &= ~(1u << id);
            return;
        }
        throw BusFault(FaultKind::Denied, offset, "unsupported PLIC write");
    }
};

class Uart16550 : public Device {
    uint8_t ier_ = 0, lcr_ = 0, mcr_ = 0, dll_ = 1, dlm_ = 0;
    std::deque<uint8_t> rx_;
    std::function<void(uint8_t)> tx_sink_;
public:
    explicit Uart16550(std::function<void(uint8_t)> sink = {}) : tx_sink_(std::move(sink)) {}
    void set_sink(std::function<void(uint8_t)> sink) { tx_sink_ = std::move(sink); }
    void inject_rx(uint8_t byte) { rx_.push_back(byte); }
    bool irq() const { return ((!rx_.empty()) && (ier_ & 1u)) || (ier_ & 2u); }

    uint64_t read(uint64_t offset, unsigned size) override {
        if (size != 1) throw BusFault(FaultKind::Denied, offset, "16550 model uses byte registers");
        const bool dlab = (lcr_ & 0x80u) != 0;
        switch (offset) {
        case 0: if (dlab) return dll_; else { if (rx_.empty()) return 0; uint8_t b = rx_.front(); rx_.pop_front(); return b; }
        case 1: return dlab ? dlm_ : ier_;
        case 2: return irq() ? ((!rx_.empty() && (ier_ & 1u)) ? 0x04u : 0x02u) : 0x01u;
        case 3: return lcr_;
        case 4: return mcr_;
        case 5: return (rx_.empty() ? 0u : 1u) | 0x20u | 0x40u; // DR + THRE + TEMT
        case 6: return 0xb0u;
        case 7: return 0;
        default: throw BusFault(FaultKind::Denied, offset, "unsupported UART read");
        }
    }
    void write(uint64_t offset, unsigned size, uint64_t value) override {
        if (size != 1) throw BusFault(FaultKind::Denied, offset, "16550 model uses byte registers");
        const uint8_t v = uint8_t(value);
        const bool dlab = (lcr_ & 0x80u) != 0;
        switch (offset) {
        case 0: if (dlab) dll_ = v; else if (tx_sink_) tx_sink_(v); return;
        case 1: if (dlab) dlm_ = v; else ier_ = v & 0x0fu; return;
        case 2: if (v & 0x02u) rx_.clear(); return;
        case 3: lcr_ = v; return;
        case 4: mcr_ = v; return;
        case 7: return;
        default: throw BusFault(FaultKind::Denied, offset, "unsupported UART write");
        }
    }
};

class SimControl : public Device {
    bool stopped_ = false;
    uint32_t code_ = 0;
public:
    bool stopped() const { return stopped_; }
    uint32_t code() const { return code_; }
    uint64_t read(uint64_t offset, unsigned size) override {
        if (offset != 0 || (size != 4 && size != 8)) throw BusFault(FaultKind::Denied, offset, "SIMCTRL register");
        return (uint64_t(code_) << 1u) | (stopped_ ? 1u : 0u);
    }
    void write(uint64_t offset, unsigned size, uint64_t value) override {
        if (offset != 0 || (size != 4 && size != 8)) throw BusFault(FaultKind::Denied, offset, "SIMCTRL register");
        if (value & 1u) { stopped_ = true; code_ = uint32_t(value >> 1u); }
    }
};

} // namespace minimpu
