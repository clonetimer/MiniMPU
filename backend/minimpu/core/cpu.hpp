#pragma once
#include "bus.hpp"
#include "mmu.hpp"
#include "pmp.hpp"
#include "types.hpp"
#include <array>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace minimpu {
namespace rv64 {
inline unsigned opcode(uint32_t i){ return i & 0x7fu; }
inline unsigned rd(uint32_t i){ return (i >> 7u) & 31u; }
inline unsigned funct3(uint32_t i){ return (i >> 12u) & 7u; }
inline unsigned rs1(uint32_t i){ return (i >> 15u) & 31u; }
inline unsigned rs2(uint32_t i){ return (i >> 20u) & 31u; }
inline unsigned funct7(uint32_t i){ return i >> 25u; }
inline int64_t imm_i(uint32_t i){ return sign_extend64(i >> 20u, 12); }
inline int64_t imm_s(uint32_t i){ return sign_extend64(((i >> 25u) << 5u) | ((i >> 7u) & 31u), 12); }
inline int64_t imm_b(uint32_t i){ return sign_extend64((((i >> 31u)&1u)<<12u)|(((i>>7u)&1u)<<11u)|(((i>>25u)&0x3fu)<<5u)|(((i>>8u)&0xfu)<<1u),13); }
inline int64_t imm_u(uint32_t i){ return int64_t(int32_t(i & 0xfffff000u)); }
inline int64_t imm_j(uint32_t i){ return sign_extend64((((i>>31u)&1u)<<20u)|(((i>>12u)&0xffu)<<12u)|(((i>>20u)&1u)<<11u)|(((i>>21u)&0x3ffu)<<1u),21); }

inline uint32_t I(unsigned op,unsigned d,unsigned f3,unsigned s1,int64_t imm){ return (uint32_t(imm)&0xfffu)<<20u | s1<<15u | f3<<12u | d<<7u | op; }
inline uint32_t R(unsigned d,unsigned f3,unsigned s1,unsigned s2,unsigned f7=0){ return f7<<25u | s2<<20u | s1<<15u | f3<<12u | d<<7u | 0x33u; }
inline uint32_t R32(unsigned d,unsigned f3,unsigned s1,unsigned s2,unsigned f7=0){ return f7<<25u | s2<<20u | s1<<15u | f3<<12u | d<<7u | 0x3bu; }
inline uint32_t S(unsigned f3,unsigned s1,unsigned s2,int64_t imm){ uint32_t v=uint32_t(imm); return ((v>>5u)&0x7fu)<<25u|s2<<20u|s1<<15u|f3<<12u|(v&31u)<<7u|0x23u; }
inline uint32_t B(unsigned f3,unsigned s1,unsigned s2,int64_t imm){ uint32_t v=uint32_t(imm); return ((v>>12u)&1u)<<31u|((v>>5u)&0x3fu)<<25u|s2<<20u|s1<<15u|f3<<12u|((v>>1u)&0xfu)<<8u|((v>>11u)&1u)<<7u|0x63u; }
inline uint32_t U(unsigned op,unsigned d,uint32_t imm){ return (imm&0xfffff000u)|d<<7u|op; }
inline uint32_t J(unsigned d,int64_t imm){ uint32_t v=uint32_t(imm); return ((v>>20u)&1u)<<31u|((v>>1u)&0x3ffu)<<21u|((v>>11u)&1u)<<20u|((v>>12u)&0xffu)<<12u|d<<7u|0x6fu; }
inline uint32_t CSR(unsigned f3,unsigned d,unsigned csr,unsigned s1_or_zimm){ return (csr&0xfffu)<<20u|(s1_or_zimm&31u)<<15u|f3<<12u|d<<7u|0x73u; }
inline uint32_t AMO(unsigned funct5,unsigned d,unsigned s1,unsigned s2,unsigned width_f3){ return funct5<<27u|s2<<20u|s1<<15u|width_f3<<12u|d<<7u|0x2fu; }
} // namespace rv64

class Cpu {
    Bus& bus_;
    std::array<uint64_t,32> regs_{};
    uint64_t pc_ = 0;
    PrivilegeMode mode_ = PrivilegeMode::Machine;
    Sv39Mmu mmu_;
    Pmp pmp_;

    uint64_t mstatus_ = (2ull<<32u) | (2ull<<34u) | (3ull<<11u); // UXL/SXL=64, MPP=M
    uint64_t medeleg_ = 0, mideleg_ = 0, mie_ = 0, mtvec_ = 0, mscratch_ = 0, mepc_ = 0, mcause_ = 0, mtval_ = 0;
    uint64_t stvec_ = 0, sscratch_ = 0, sepc_ = 0, scause_ = 0, stval_ = 0, satp_ = 0;
    uint64_t mcounteren_ = 0, scounteren_ = 0;
    uint64_t cycle_ = 0, instret_ = 0;
    bool irq_msip_ = false, irq_mtip_ = false, irq_meip_ = false;
    bool irq_ssip_ = false, irq_stip_ = false, irq_seip_ = false;
    uint64_t reservation_pa_ = 0; unsigned reservation_size_ = 0; bool reservation_valid_ = false;

    struct TrapSignal { uint64_t cause; uint64_t tval; };
    struct InterruptChoice { bool valid=false; uint64_t cause=0; PrivilegeMode target=PrivilegeMode::Machine; };

    static constexpr uint64_t MSTATUS_SIE  = 1ull<<1u;
    static constexpr uint64_t MSTATUS_MIE  = 1ull<<3u;
    static constexpr uint64_t MSTATUS_SPIE = 1ull<<5u;
    static constexpr uint64_t MSTATUS_MPIE = 1ull<<7u;
    static constexpr uint64_t MSTATUS_SPP  = 1ull<<8u;
    static constexpr uint64_t MSTATUS_MPP  = 3ull<<11u;
    static constexpr uint64_t MSTATUS_MPRV = 1ull<<17u;
    static constexpr uint64_t MSTATUS_SUM  = 1ull<<18u;
    static constexpr uint64_t MSTATUS_MXR  = 1ull<<19u;
    static constexpr uint64_t SSTATUS_MASK = MSTATUS_SIE|MSTATUS_SPIE|MSTATUS_SPP|MSTATUS_SUM|MSTATUS_MXR|(3ull<<32u);

    static uint64_t misa_value(){ return (2ull<<62u)|(1ull<<0u)|(1ull<<8u)|(1ull<<12u)|(1ull<<18u)|(1ull<<20u); }
    uint64_t mip_value() const {
        return (irq_ssip_?1ull<<1u:0)|(irq_msip_?1ull<<3u:0)|(irq_stip_?1ull<<5u:0)|(irq_mtip_?1ull<<7u:0)|(irq_seip_?1ull<<9u:0)|(irq_meip_?1ull<<11u:0);
    }
    [[noreturn]] void illegal(uint32_t inst) const { throw TrapSignal{2,inst}; }

    PrivilegeMode effective_mode(AccessType access) const {
        if (access != AccessType::Fetch && mode_ == PrivilegeMode::Machine && (mstatus_ & MSTATUS_MPRV)) {
            const unsigned mpp = unsigned((mstatus_ >> 11u) & 3u);
            if (mpp == 0) return PrivilegeMode::User;
            if (mpp == 1) return PrivilegeMode::Supervisor;
        }
        return mode_;
    }

    uint64_t translate(uint64_t va, AccessType access, unsigned size) {
        const PrivilegeMode eff = effective_mode(access);
        uint64_t pa = 0;
        try { pa = mmu_.translate(bus_, va, access, eff, satp_, (mstatus_&MSTATUS_SUM)!=0, (mstatus_&MSTATUS_MXR)!=0, &pmp_); }
        catch (const AccessFault&) {
            const uint64_t cause = access==AccessType::Fetch?1:(access==AccessType::Load?5:7);
            throw TrapSignal{cause,va};
        }
        catch (const PageFault&) {
            const uint64_t cause = access==AccessType::Fetch?12:(access==AccessType::Load?13:15);
            throw TrapSignal{cause,va};
        }
        if (!pmp_.check(pa,size,access,eff)) {
            const uint64_t cause = access==AccessType::Fetch?1:(access==AccessType::Load?5:7);
            throw TrapSignal{cause,va};
        }
        return pa;
    }

    uint64_t load_virtual(uint64_t va,unsigned size,bool signed_load) {
        if (va & (size-1u)) throw TrapSignal{4,va};
        const uint64_t pa = translate(va,AccessType::Load,size);
        uint64_t v=0; try { v=bus_.read(pa,size); } catch(const BusFault&){ throw TrapSignal{5,va}; }
        if (!signed_load || size==8) return v;
        return uint64_t(sign_extend64(v,size*8u));
    }
    void store_virtual(uint64_t va,unsigned size,uint64_t value) {
        if (va & (size-1u)) throw TrapSignal{6,va};
        const uint64_t pa=translate(va,AccessType::Store,size);
        try { bus_.write(pa,size,value); } catch(const BusFault&){ throw TrapSignal{7,va}; }
        if (reservation_valid_ && pa < reservation_pa_+reservation_size_ && reservation_pa_ < pa+size) reservation_valid_=false;
    }
    uint32_t fetch() {
        if (pc_ & 3u) throw TrapSignal{0,pc_};
        const uint64_t pa=translate(pc_,AccessType::Fetch,4);
        try { return uint32_t(bus_.read(pa,4)); } catch(const BusFault&){ throw TrapSignal{1,pc_}; }
    }

    InterruptChoice choose_interrupt() const {
        const uint64_t pending = mip_value() & mie_;
        const uint64_t order[] = {11,3,7,9,1,5};
        for (uint64_t cause : order) {
            if (!(pending & (1ull<<cause))) continue;
            const bool delegated = (mideleg_ & (1ull<<cause)) != 0;
            if (delegated) {
                if (mode_ == PrivilegeMode::Machine) continue;
                const bool global = mode_ == PrivilegeMode::User || (mstatus_ & MSTATUS_SIE);
                if (global) return {true,cause,PrivilegeMode::Supervisor};
            } else {
                const bool global = mode_ != PrivilegeMode::Machine || (mstatus_ & MSTATUS_MIE);
                if (global) return {true,cause,PrivilegeMode::Machine};
            }
        }
        return {};
    }

    void enter_trap(uint64_t cause,uint64_t tval,bool interrupt,PrivilegeMode forced_target=PrivilegeMode::User) {
        PrivilegeMode target = forced_target;
        if (target == PrivilegeMode::User) {
            const bool delegated = mode_ != PrivilegeMode::Machine && (((interrupt?mideleg_:medeleg_) >> cause)&1u);
            target = delegated ? PrivilegeMode::Supervisor : PrivilegeMode::Machine;
        }
        const uint64_t encoded = (interrupt?(1ull<<63u):0)|cause;
        if (target == PrivilegeMode::Supervisor) {
            if (mstatus_ & MSTATUS_SIE) mstatus_ |= MSTATUS_SPIE; else mstatus_ &= ~MSTATUS_SPIE;
            mstatus_ &= ~MSTATUS_SIE;
            if (mode_ == PrivilegeMode::Supervisor) mstatus_ |= MSTATUS_SPP; else mstatus_ &= ~MSTATUS_SPP;
            sepc_=pc_; scause_=encoded; stval_=tval; mode_=PrivilegeMode::Supervisor;
            const uint64_t base=stvec_&~3ull; pc_=base+(((stvec_&3u)==1u&&interrupt)?4u*cause:0u);
        } else {
            if (mstatus_ & MSTATUS_MIE) mstatus_ |= MSTATUS_MPIE; else mstatus_ &= ~MSTATUS_MPIE;
            mstatus_ &= ~MSTATUS_MIE;
            mstatus_ = (mstatus_ & ~MSTATUS_MPP) | (uint64_t(mode_)<<11u);
            mepc_=pc_; mcause_=encoded; mtval_=tval; mode_=PrivilegeMode::Machine;
            const uint64_t base=mtvec_&~3ull; pc_=base+(((mtvec_&3u)==1u&&interrupt)?4u*cause:0u);
        }
        reservation_valid_=false;
    }

    uint64_t read_csr(unsigned csr,uint32_t inst) const {
        switch(csr){
        case 0x100: return (mstatus_&SSTATUS_MASK)|(2ull<<32u);
        case 0x104: return mie_&mideleg_;
        case 0x105: return stvec_;
        case 0x106: return scounteren_;
        case 0x140: return sscratch_;
        case 0x141: return sepc_;
        case 0x142: return scause_;
        case 0x143: return stval_;
        case 0x144: return mip_value()&mideleg_;
        case 0x180: return satp_;
        case 0x300: return mstatus_;
        case 0x301: return misa_value();
        case 0x302: return medeleg_;
        case 0x303: return mideleg_;
        case 0x304: return mie_;
        case 0x305: return mtvec_;
        case 0x306: return mcounteren_;
        case 0x340: return mscratch_;
        case 0x341: return mepc_;
        case 0x342: return mcause_;
        case 0x343: return mtval_;
        case 0x344: return mip_value();
        case 0x3a0: { uint64_t v=0; for(unsigned i=0;i<Pmp::kEntries;++i) v|=uint64_t(pmp_.cfg(i))<<(8u*i); return v; }
        case 0xb00: case 0xc00: return cycle_;
        case 0xb02: case 0xc02: return instret_;
        case 0xc01: return cycle_; // architectural time proxy; SoC exposes real CLINT time separately
        case 0xf11: case 0xf12: case 0xf13: return 0;
        case 0xf14: return 0;
        default:
            if(csr>=0x3b0&&csr<0x3b0+Pmp::kEntries) return pmp_.addr(csr-0x3b0);
            throw TrapSignal{2,inst};
        }
    }
    void write_csr(unsigned csr,uint64_t value,uint32_t inst) {
        switch(csr){
        case 0x100: mstatus_=(mstatus_&~SSTATUS_MASK)|(value&SSTATUS_MASK); return;
        case 0x104: mie_=(mie_&~mideleg_)|(value&mideleg_); return;
        case 0x105: stvec_=value&~2ull; return;
        case 0x106: scounteren_=value; return;
        case 0x140: sscratch_=value; return;
        case 0x141: sepc_=value&~3ull; return;
        case 0x142: scause_=value; return;
        case 0x143: stval_=value; return;
        case 0x144: irq_ssip_=(value&(1ull<<1u))!=0; irq_stip_=(value&(1ull<<5u))!=0; irq_seip_=(value&(1ull<<9u))!=0; return;
        case 0x180: satp_=value; mmu_.flush(); return;
        case 0x300: {
            const uint64_t writable=MSTATUS_SIE|MSTATUS_MIE|MSTATUS_SPIE|MSTATUS_MPIE|MSTATUS_SPP|MSTATUS_MPP|MSTATUS_MPRV|MSTATUS_SUM|MSTATUS_MXR;
            mstatus_=(mstatus_&~writable)|(value&writable)|(2ull<<32u)|(2ull<<34u); return;
        }
        case 0x302: medeleg_=value; return;
        case 0x303: mideleg_=value; return;
        case 0x304: mie_=value; return;
        case 0x305: mtvec_=value&~2ull; return;
        case 0x306: mcounteren_=value; return;
        case 0x340: mscratch_=value; return;
        case 0x341: mepc_=value&~3ull; return;
        case 0x342: mcause_=value; return;
        case 0x343: mtval_=value; return;
        case 0x344: irq_ssip_=(value&(1ull<<1u))!=0; irq_stip_=(value&(1ull<<5u))!=0; irq_seip_=(value&(1ull<<9u))!=0; return;
        case 0x3a0: for(unsigned i=0;i<Pmp::kEntries;++i) pmp_.write_cfg(i,uint8_t(value>>(8u*i))); return;
        case 0xb00: cycle_=value; return;
        case 0xb02: instret_=value; return;
        default:
            if(csr>=0x3b0&&csr<0x3b0+Pmp::kEntries){ pmp_.write_addr(csr-0x3b0,value); return; }
            throw TrapSignal{2,inst};
        }
    }
    void csr_access_check(unsigned csr,bool write,uint32_t inst) const {
        const unsigned required=(csr>>8u)&3u;
        if(unsigned(mode_)<required) throw TrapSignal{2,inst};
        if(write && ((csr>>10u)&3u)==3u) throw TrapSignal{2,inst};
    }

    static uint64_t mulh_uu(uint64_t a,uint64_t b){
        const uint64_t a0=uint32_t(a), a1=a>>32u, b0=uint32_t(b), b1=b>>32u;
        uint64_t t=a0*b0; const uint64_t k=t>>32u;
        t=a1*b0+k; const uint64_t w1=uint32_t(t), w2=t>>32u;
        t=a0*b1+w1;
        return a1*b1+w2+(t>>32u);
    }
    static uint64_t mulh_ss(uint64_t a,uint64_t b){
        uint64_t hi=mulh_uu(a,b); if(a>>63u)hi-=b; if(b>>63u)hi-=a; return hi;
    }
    static uint64_t mulh_su(uint64_t a,uint64_t b){
        uint64_t hi=mulh_uu(a,b); if(a>>63u)hi-=b; return hi;
    }

    void execute_atomic(uint32_t inst,uint64_t& next_pc) {
        (void)next_pc;
        const unsigned f3=rv64::funct3(inst); if(f3!=2&&f3!=3) illegal(inst);
        const unsigned size=f3==2?4:8, d=rv64::rd(inst), s1=rv64::rs1(inst), s2=rv64::rs2(inst);
        const unsigned f5=(inst>>27u)&0x1fu; const uint64_t va=regs_[s1];
        const AccessType at=(f5==0x02u)?AccessType::Load:AccessType::Store;
        if(va&(size-1u)) throw TrapSignal{at==AccessType::Load?4ull:6ull,va};
        const uint64_t pa=translate(va,at,size);
        uint64_t old=0;
        try { old=bus_.read(pa,size); } catch(const BusFault&){ throw TrapSignal{at==AccessType::Load?5ull:7ull,va}; }
        auto rd_old=[&](){ regs_[d]=size==4?sext32(uint32_t(old)):old; };
        if(f5==0x02u){ if(s2!=0) illegal(inst); rd_old(); reservation_pa_=pa; reservation_size_=size; reservation_valid_=true; return; }
        if(f5==0x03u){
            const bool ok=reservation_valid_&&reservation_pa_==pa&&reservation_size_==size; reservation_valid_=false;
            if(ok){ try{ bus_.write(pa,size,regs_[s2]); }catch(const BusFault&){ throw TrapSignal{7,va}; } regs_[d]=0; } else regs_[d]=1; return;
        }
        uint64_t lhs=size==4?uint32_t(old):old, rhs=size==4?uint32_t(regs_[s2]):regs_[s2], result=0;
        switch(f5){
        case 0x00: result=lhs+rhs; break; // AMOADD
        case 0x01: result=rhs; break;     // AMOSWAP
        case 0x04: result=lhs^rhs; break;
        case 0x08: result=lhs|rhs; break;
        case 0x0c: result=lhs&rhs; break;
        case 0x10: result=size==4?(int32_t(lhs)<int32_t(rhs)?lhs:rhs):(int64_t(lhs)<int64_t(rhs)?lhs:rhs); break;
        case 0x14: result=size==4?(int32_t(lhs)>int32_t(rhs)?lhs:rhs):(int64_t(lhs)>int64_t(rhs)?lhs:rhs); break;
        case 0x18: result=lhs<rhs?lhs:rhs; break;
        case 0x1c: result=lhs>rhs?lhs:rhs; break;
        default: illegal(inst);
        }
        try{ bus_.write(pa,size,result); }catch(const BusFault&){ throw TrapSignal{7,va}; }
        reservation_valid_=false; rd_old();
    }

    void execute(uint32_t inst,uint64_t& next_pc) {
        const unsigned op=rv64::opcode(inst), d=rv64::rd(inst), s1=rv64::rs1(inst), s2=rv64::rs2(inst), f3=rv64::funct3(inst), f7=rv64::funct7(inst);
        const uint64_t a=regs_[s1], b=regs_[s2];
        auto wr=[&](uint64_t v){ if(d) regs_[d]=v; };
        switch(op){
        case 0x37: wr(uint64_t(rv64::imm_u(inst))); break;
        case 0x17: wr(pc_+uint64_t(rv64::imm_u(inst))); break;
        case 0x6f: { const uint64_t target=pc_+rv64::imm_j(inst); if(target&3u) throw TrapSignal{0,target}; wr(pc_+4); next_pc=target; break; }
        case 0x67: if(f3) illegal(inst); else { const uint64_t target=(a+rv64::imm_i(inst))&~1ull; if(target&3u) throw TrapSignal{0,target}; wr(pc_+4); next_pc=target; } break;
        case 0x63: {
            bool take=false; switch(f3){ case 0:take=a==b;break;case 1:take=a!=b;break;case 4:take=int64_t(a)<int64_t(b);break;case 5:take=int64_t(a)>=int64_t(b);break;case 6:take=a<b;break;case 7:take=a>=b;break;default:illegal(inst); }
            if(take){ const uint64_t target=pc_+rv64::imm_b(inst); if(target&3u) throw TrapSignal{0,target}; next_pc=target; } break;
        }
        case 0x03: {
            const uint64_t addr=a+rv64::imm_i(inst); switch(f3){ case 0:wr(load_virtual(addr,1,true));break;case 1:wr(load_virtual(addr,2,true));break;case 2:wr(load_virtual(addr,4,true));break;case 3:wr(load_virtual(addr,8,false));break;case 4:wr(load_virtual(addr,1,false));break;case 5:wr(load_virtual(addr,2,false));break;case 6:wr(uint32_t(load_virtual(addr,4,false)));break;default:illegal(inst);} break;
        }
        case 0x23: { const uint64_t addr=a+rv64::imm_s(inst); switch(f3){case 0:store_virtual(addr,1,b);break;case 1:store_virtual(addr,2,b);break;case 2:store_virtual(addr,4,b);break;case 3:store_virtual(addr,8,b);break;default:illegal(inst);} break; }
        case 0x13: {
            const int64_t imm=rv64::imm_i(inst); switch(f3){
            case 0:wr(a+uint64_t(imm));break;case 2:wr(int64_t(a)<imm);break;case 3:wr(a<uint64_t(imm));break;case 4:wr(a^uint64_t(imm));break;case 6:wr(a|uint64_t(imm));break;case 7:wr(a&uint64_t(imm));break;
            case 1: if((inst>>26u)!=0) illegal(inst); else wr(a<<(s2&63u)); break;
            case 5: { const unsigned top=inst>>26u; if(top==0) wr(a>>(s2&63u)); else if(top==0x10u) wr(uint64_t(int64_t(a)>>(s2&63u))); else illegal(inst); break; }
            default:illegal(inst); } break;
        }
        case 0x1b: {
            const int64_t imm=rv64::imm_i(inst); uint32_t r=0; switch(f3){
            case 0:r=uint32_t(a+uint64_t(imm));break;
            case 1: if((inst>>25u)!=0) illegal(inst); else r=uint32_t(a)<<(s2&31u); break;
            case 5: if((inst>>25u)==0) r=uint32_t(a)>>(s2&31u); else if((inst>>25u)==0x20u) r=uint32_t(int32_t(a)>>(s2&31u)); else illegal(inst); break;
            default:illegal(inst);} wr(sext32(r)); break;
        }
        case 0x33: {
            if(f7==1){ switch(f3){
                case 0:wr(a*b);break;case 1:wr(mulh_ss(a,b));break;case 2:wr(mulh_su(a,b));break;case 3:wr(mulh_uu(a,b));break;
                case 4: { const int64_t x=int64_t(a),y=int64_t(b); wr(y==0?~0ull:(x==std::numeric_limits<int64_t>::min()&&y==-1?uint64_t(x):uint64_t(x/y))); break; }
                case 5:wr(b==0?~0ull:a/b);break;
                case 6: { const int64_t x=int64_t(a),y=int64_t(b); wr(y==0?uint64_t(x):(x==std::numeric_limits<int64_t>::min()&&y==-1?0:uint64_t(x%y))); break; }
                case 7:wr(b==0?a:a%b);break; default:illegal(inst); } break; }
            switch(f3){case 0:if(f7==0)wr(a+b);else if(f7==0x20)wr(a-b);else illegal(inst);break;case 1:if(f7==0)wr(a<<(b&63u));else illegal(inst);break;case 2:if(f7==0)wr(int64_t(a)<int64_t(b));else illegal(inst);break;case 3:if(f7==0)wr(a<b);else illegal(inst);break;case 4:if(f7==0)wr(a^b);else illegal(inst);break;case 5:if(f7==0)wr(a>>(b&63u));else if(f7==0x20)wr(uint64_t(int64_t(a)>>(b&63u)));else illegal(inst);break;case 6:if(f7==0)wr(a|b);else illegal(inst);break;case 7:if(f7==0)wr(a&b);else illegal(inst);break;default:illegal(inst);} break;
        }
        case 0x3b: {
            uint32_t r=0;
            if(f7==1){ const int32_t x=int32_t(a),y=int32_t(b); const uint32_t ux=uint32_t(a),uy=uint32_t(b); switch(f3){case 0:r=uint32_t(uint64_t(ux)*uint64_t(uy));break;case 4:r=y==0?0xffffffffu:(x==std::numeric_limits<int32_t>::min()&&y==-1?uint32_t(x):uint32_t(x/y));break;case 5:r=uy==0?0xffffffffu:ux/uy;break;case 6:r=y==0?uint32_t(x):(x==std::numeric_limits<int32_t>::min()&&y==-1?0u:uint32_t(x%y));break;case 7:r=uy==0?ux:ux%uy;break;default:illegal(inst);} wr(sext32(r)); break; }
            switch(f3){case 0:if(f7==0)r=uint32_t(a+b);else if(f7==0x20)r=uint32_t(a-b);else illegal(inst);break;case 1:if(f7==0)r=uint32_t(a)<<(b&31u);else illegal(inst);break;case 5:if(f7==0)r=uint32_t(a)>>(b&31u);else if(f7==0x20)r=uint32_t(int32_t(a)>>(b&31u));else illegal(inst);break;default:illegal(inst);} wr(sext32(r)); break;
        }
        case 0x0f: break; // FENCE/FENCE.I: strongly ordered functional model
        case 0x2f: execute_atomic(inst,next_pc); break;
        case 0x73: {
            if(f3==0){
                if((inst&0xfe007fffu)==0x12000073u){ if(mode_==PrivilegeMode::User) illegal(inst); const uint64_t va=regs_[s1]; const uint16_t asid=uint16_t(regs_[s2]); if(s1==0&&s2==0)mmu_.flush(); else mmu_.flush(va,asid); break; }
                const unsigned imm=inst>>20u;
                if(imm==0){ throw TrapSignal{mode_==PrivilegeMode::User?8ull:(mode_==PrivilegeMode::Supervisor?9ull:11ull),0}; }
                if(imm==1) throw TrapSignal{3,pc_};
                if(imm==0x302){ if(mode_!=PrivilegeMode::Machine) illegal(inst); const unsigned mpp=unsigned((mstatus_>>11u)&3u); mode_=mpp==1?PrivilegeMode::Supervisor:(mpp==0?PrivilegeMode::User:PrivilegeMode::Machine); if(mstatus_&MSTATUS_MPIE)mstatus_|=MSTATUS_MIE;else mstatus_&=~MSTATUS_MIE; mstatus_|=MSTATUS_MPIE; if(mpp!=3)mstatus_&=~MSTATUS_MPRV; mstatus_&=~MSTATUS_MPP; next_pc=mepc_; break; }
                if(imm==0x102){ if(mode_==PrivilegeMode::User) illegal(inst); mode_=(mstatus_&MSTATUS_SPP)?PrivilegeMode::Supervisor:PrivilegeMode::User; if(mstatus_&MSTATUS_SPIE)mstatus_|=MSTATUS_SIE;else mstatus_&=~MSTATUS_SIE; mstatus_|=MSTATUS_SPIE; mstatus_&=~MSTATUS_SPP; mstatus_&=~MSTATUS_MPRV; next_pc=sepc_; break; }
                if(imm==0x105) break; // WFI hint in this functional model
                illegal(inst);
            } else {
                const unsigned csr=inst>>20u; const bool immediate=f3>=5; const uint64_t source=immediate?s1:regs_[s1];
                const bool write=(f3==1||f3==5)||((f3==2||f3==3||f3==6||f3==7)&&source!=0);
                csr_access_check(csr,write,inst); const uint64_t old=read_csr(csr,inst); uint64_t nv=old;
                switch(f3){case 1:case 5:nv=source;break;case 2:case 6:nv=old|source;break;case 3:case 7:nv=old&~source;break;default:illegal(inst);}
                if(write) write_csr(csr,nv,inst);
                wr(old);
            } break;
        }
        default: illegal(inst);
        }
    }

public:
    explicit Cpu(Bus& bus,uint64_t reset_pc=0x80000000ull):bus_(bus),pc_(reset_pc){}
    uint64_t reg(unsigned index) const { return regs_.at(index); }
    void set_reg(unsigned index,uint64_t value){ if(index)regs_.at(index)=value; }
    uint64_t pc() const{return pc_;} void set_pc(uint64_t value){pc_=value;}
    PrivilegeMode mode() const{return mode_;} void set_mode_for_test(PrivilegeMode mode){mode_=mode;}
    uint64_t cycle() const{return cycle_;} uint64_t instret() const{return instret_;}
    Pmp& pmp(){return pmp_;} Sv39Mmu& mmu(){return mmu_;}
    void set_machine_software_irq(bool v){irq_msip_=v;} void set_machine_timer_irq(bool v){irq_mtip_=v;} void set_machine_external_irq(bool v){irq_meip_=v;}
    void set_supervisor_software_irq(bool v){irq_ssip_=v;} void set_supervisor_timer_irq(bool v){irq_stip_=v;} void set_supervisor_external_irq(bool v){irq_seip_=v;}
    uint64_t csr_debug(unsigned csr) const { return read_csr(csr,0); }
    void set_csr_debug(unsigned csr,uint64_t value){ write_csr(csr,value,0); }

    unsigned step(){
        ++cycle_;
        const auto irq=choose_interrupt(); if(irq.valid){ enter_trap(irq.cause,0,true,irq.target); regs_[0]=0; return 1; }
        try{
            const uint32_t inst=fetch(); uint64_t next_pc=pc_+4u; execute(inst,next_pc); pc_=next_pc; regs_[0]=0; ++instret_; return 1;
        }catch(const TrapSignal& trap){ enter_trap(trap.cause,trap.tval,false); regs_[0]=0; return 1; }
    }
};

} // namespace minimpu
