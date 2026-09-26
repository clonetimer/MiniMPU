#pragma once
#include "bus.hpp"
#include "pmp.hpp"
#include "types.hpp"
#include <array>
#include <cstdint>

namespace minimpu {

class Sv39Mmu {
    struct TlbEntry {
        bool valid = false;
        uint64_t vpn = 0;
        uint64_t ppn = 0;
        uint16_t asid = 0;
        uint8_t level = 0;
        uint8_t flags = 0;
        bool global = false;
    };
    std::array<TlbEntry, 32> tlb_{};
    unsigned replace_ = 0;

    static bool canonical(uint64_t va) {
        const uint64_t upper = va >> 39u;
        return ((va >> 38u) & 1u) ? upper == ((1ull << 25u) - 1ull) : upper == 0;
    }

    static bool permissions(uint8_t flags, AccessType access, PrivilegeMode mode, bool sum, bool mxr) {
        const bool r = flags & 0x02u;
        const bool w = flags & 0x04u;
        const bool x = flags & 0x08u;
        const bool u = flags & 0x10u;
        if (mode == PrivilegeMode::User && !u) return false;
        if (mode == PrivilegeMode::Supervisor && u) {
            if (access == AccessType::Fetch) return false;
            if (!sum) return false;
        }
        if (access == AccessType::Fetch) return x;
        if (access == AccessType::Store) return w;
        return r || (mxr && x);
    }

    static uint64_t compose_pa(uint64_t pte, uint64_t va, unsigned level) {
        const uint64_t vpn0 = (va >> 12u) & 0x1ffu;
        const uint64_t vpn1 = (va >> 21u) & 0x1ffu;
        const uint64_t pte_ppn0 = (pte >> 10u) & 0x1ffu;
        const uint64_t pte_ppn1 = (pte >> 19u) & 0x1ffu;
        const uint64_t pte_ppn2 = (pte >> 28u) & 0x3ffffffu;
        uint64_t ppn0 = pte_ppn0, ppn1 = pte_ppn1;
        if (level == 2) { ppn1 = vpn1; ppn0 = vpn0; }
        else if (level == 1) { ppn0 = vpn0; }
        return (pte_ppn2 << 30u) | (ppn1 << 21u) | (ppn0 << 12u) | (va & 0xfffu);
    }

    uint64_t tlb_lookup(uint64_t va, uint16_t asid, AccessType access, PrivilegeMode mode, bool sum, bool mxr, bool& hit) const {
        const uint64_t vpn = va >> 12u;
        for (const auto& e : tlb_) {
            if (!e.valid || (!e.global && e.asid != asid)) continue;
            uint64_t match_mask = ~0ull;
            if (e.level == 2) match_mask = ~((1ull << 18u) - 1ull);
            else if (e.level == 1) match_mask = ~((1ull << 9u) - 1ull);
            if ((vpn & match_mask) != (e.vpn & match_mask)) continue;
            if (!permissions(e.flags, access, mode, sum, mxr)) throw PageFault(access, va, "TLB permission fault");
            hit = true;
            uint64_t ppn = e.ppn;
            if (e.level == 2) ppn = (ppn & ~((1ull << 18u) - 1ull)) | (vpn & ((1ull << 18u) - 1ull));
            else if (e.level == 1) ppn = (ppn & ~((1ull << 9u) - 1ull)) | (vpn & ((1ull << 9u) - 1ull));
            return (ppn << 12u) | (va & 0xfffu);
        }
        hit = false;
        return 0;
    }

public:
    void flush() { for (auto& e : tlb_) e.valid = false; }
    void flush(uint64_t virtual_address, uint16_t asid) {
        const uint64_t vpn = virtual_address >> 12u;
        for (auto& e : tlb_) {
            if (!e.valid) continue;
            if (!e.global && e.asid != asid) continue;
            uint64_t mask = ~0ull;
            if (e.level == 2) mask = ~((1ull << 18u) - 1ull);
            else if (e.level == 1) mask = ~((1ull << 9u) - 1ull);
            if ((e.vpn & mask) == (vpn & mask)) e.valid = false;
        }
    }

    uint64_t translate(Bus& bus, uint64_t va, AccessType access, PrivilegeMode mode,
                       uint64_t satp, bool sum, bool mxr, const Pmp* pmp = nullptr) {
        const uint64_t satp_mode = satp >> 60u;
        if (mode == PrivilegeMode::Machine || satp_mode == 0) return va;
        if (satp_mode != 8u || !canonical(va)) throw PageFault(access, va, "unsupported/non-canonical virtual address");
        const uint16_t asid = uint16_t((satp >> 44u) & 0xffffu);
        bool hit = false;
        if (const uint64_t pa = tlb_lookup(va, asid, access, mode, sum, mxr, hit); hit) return pa;

        const uint64_t vpn[3] = {(va >> 12u) & 0x1ffu, (va >> 21u) & 0x1ffu, (va >> 30u) & 0x1ffu};
        uint64_t table = (satp & ((1ull << 44u) - 1ull)) << 12u;
        for (int level = 2; level >= 0; --level) {
            const uint64_t pte_address = table + vpn[level] * 8u;
            uint64_t pte = 0;
            if (pmp && !pmp->check(pte_address, 8, AccessType::Load, PrivilegeMode::Supervisor))
                throw AccessFault(access, va, "PMP denied implicit page-table read");
            try { pte = bus.read(pte_address, 8); }
            catch (const BusFault&) { throw AccessFault(access, va, "page-table physical access fault"); }
            const bool v = pte & 0x1u, r = pte & 0x2u, w = pte & 0x4u, x = pte & 0x8u;
            if (!v || (!r && w)) throw PageFault(access, va, "invalid PTE");
            if (r || x) {
                if (level > 0) {
                    const uint64_t low_ppn_mask = level == 2 ? ((1ull << 18u) - 1ull) : ((1ull << 9u) - 1ull);
                    if (((pte >> 10u) & low_ppn_mask) != 0) throw PageFault(access, va, "misaligned superpage");
                }
                const uint8_t flags = uint8_t(pte & 0xffu);
                if (!permissions(flags, access, mode, sum, mxr)) throw PageFault(access, va, "PTE permission fault");
                // This educational model implements hardware-managed A/D updates (Svadu-style behavior).
                uint64_t updated = pte | (1ull << 6u);
                if (access == AccessType::Store) updated |= 1ull << 7u;
                if (updated != pte) {
                    if (pmp && !pmp->check(pte_address, 8, AccessType::Store, PrivilegeMode::Supervisor))
                        throw AccessFault(access, va, "PMP denied implicit A/D update");
                    try { bus.write(pte_address, 8, updated); }
                    catch (const BusFault&) { throw AccessFault(access, va, "A/D update physical access fault"); }
                    pte = updated;
                }
                const uint64_t pa = compose_pa(pte, va, unsigned(level));
                auto& slot = tlb_[replace_++ % tlb_.size()];
                slot.valid = true; slot.vpn = va >> 12u; slot.ppn = pa >> 12u; slot.asid = asid;
                slot.level = uint8_t(level); slot.flags = uint8_t(pte & 0xffu); slot.global = (pte & 0x20u) != 0;
                return pa;
            }
            table = ((pte >> 10u) & ((1ull << 44u) - 1ull)) << 12u;
        }
        throw PageFault(access, va, "Sv39 walk exhausted");
    }
};

} // namespace minimpu
