#pragma once
#include "devices.hpp"
#include <array>
#include <cstdint>
#include <functional>

namespace minimpu {

struct SbiRet { int64_t error = 0; uint64_t value = 0; };

class SbiDispatcher {
    Clint& clint_;
    std::function<void(uint8_t)> console_;
    bool reset_requested_ = false;
public:
    static constexpr uint64_t EID_BASE = 0x10;
    static constexpr uint64_t EID_TIME = 0x54494d45;
    static constexpr uint64_t EID_HSM  = 0x48534d;
    static constexpr uint64_t EID_SRST = 0x53525354;
    static constexpr uint64_t EID_DBCN = 0x4442434e;
    static constexpr int64_t SBI_SUCCESS = 0;
    static constexpr int64_t SBI_ERR_NOT_SUPPORTED = -2;
    static constexpr int64_t SBI_ERR_INVALID_PARAM = -3;

    explicit SbiDispatcher(Clint& clint, std::function<void(uint8_t)> console = {})
        : clint_(clint), console_(std::move(console)) {}
    bool reset_requested() const { return reset_requested_; }

    bool extension_supported(uint64_t eid) const {
        return eid==EID_BASE || eid==EID_TIME || eid==EID_SRST;
    }

    SbiRet call(uint64_t eid, uint64_t fid, const std::array<uint64_t,6>& a) {
        if (eid == EID_BASE) {
            switch (fid) {
            case 0: return {0, 3ull<<24u}; // SBI 3.0
            case 1: return {0, 0x4d4d5055ull}; // "MMPU" teaching implementation ID
            case 2: return {0, 0x00010000ull};
            case 3: return {0, extension_supported(a[0]) ? 1ull : 0ull};
            case 4: case 5: case 6: return {0, 0};
            default: return {SBI_ERR_NOT_SUPPORTED,0};
            }
        }
        if (eid == EID_TIME && fid == 0) { clint_.set_mtimecmp(a[0]); return {}; }
        if (eid == EID_HSM) {
            if (fid == 2) return a[0] == 0 ? SbiRet{0,0} : SbiRet{SBI_ERR_INVALID_PARAM,0}; // hart0 started
            if (fid == 0) return a[0] == 0 ? SbiRet{0,0} : SbiRet{SBI_ERR_INVALID_PARAM,0};
            if (fid == 1) return {SBI_ERR_NOT_SUPPORTED,0};
            return {SBI_ERR_NOT_SUPPORTED,0};
        }
        if (eid == EID_SRST && fid == 0) { reset_requested_ = true; return {}; }
        if (eid == EID_DBCN && fid == 2) { if (console_) console_(uint8_t(a[0])); return {}; }
        return {SBI_ERR_NOT_SUPPORTED,0};
    }
};

} // namespace minimpu
