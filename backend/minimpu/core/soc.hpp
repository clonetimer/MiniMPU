#pragma once
#include "cpu.hpp"
#include "devices.hpp"
#include "memory.hpp"
#include <cstdint>
#include <functional>
#include <stdexcept>

namespace minimpu {

class SoC {
public:
    static constexpr uint64_t MROM_BASE=0x00001000ull;
    static constexpr uint64_t CLINT_BASE=0x02000000ull;
    static constexpr uint64_t PLIC_BASE=0x0c000000ull;
    static constexpr uint64_t UART_BASE=0x10000000ull;
    static constexpr uint64_t SIMCTRL_BASE=0x10010000ull;
    static constexpr uint64_t DRAM_BASE=0x80000000ull;
private:
    Bus bus_;
    Memory mrom_{64*1024,false};
    Clint clint_;
    Plic plic_;
    Uart16550 uart_;
    SimControl simctrl_;
    Memory dram_;
    Cpu cpu_;
public:
    explicit SoC(size_t dram_size=64*1024*1024, std::function<void(uint8_t)> uart_sink={})
        : uart_(std::move(uart_sink)), dram_(dram_size,true), cpu_(bus_,DRAM_BASE) {
        bus_.map(MROM_BASE,mrom_.size(),mrom_,"mrom");
        bus_.map(CLINT_BASE,0x10000,clint_,"clint");
        bus_.map(PLIC_BASE,0x400000,plic_,"plic");
        bus_.map(UART_BASE,0x100,uart_,"uart16550");
        bus_.map(SIMCTRL_BASE,0x1000,simctrl_,"simctrl");
        bus_.map(DRAM_BASE,dram_.size(),dram_,"dram");
        plic_.set_priority(10,1); // UART
    }
    Bus& bus(){return bus_;} Memory& dram(){return dram_;} Memory& mrom(){return mrom_;} Clint& clint(){return clint_;} Plic& plic(){return plic_;} Uart16550& uart(){return uart_;} Cpu& cpu(){return cpu_;} SimControl& simctrl(){return simctrl_;}
    void allow_all_for_supervisor(){ cpu_.pmp().write_addr(0,1ull<<54u); cpu_.pmp().write_cfg(0,0x0fu); }
    void load(uint64_t physical_address,const uint8_t* data,size_t length){
        if(physical_address>=DRAM_BASE && physical_address-DRAM_BASE+length<=dram_.size()){ dram_.load(physical_address-DRAM_BASE,data,length); return; }
        if(physical_address>=MROM_BASE && physical_address-MROM_BASE+length<=mrom_.size()){ mrom_.load(physical_address-MROM_BASE,data,length); return; }
        throw std::out_of_range("load address outside MROM/DRAM");
    }
    unsigned step(){
        if(uart_.irq()) plic_.pulse(10);
        cpu_.set_machine_software_irq(clint_.software_irq()); cpu_.set_machine_timer_irq(clint_.timer_irq());
        cpu_.set_machine_external_irq(plic_.irq_m()); cpu_.set_supervisor_external_irq(plic_.irq_s());
        const unsigned cycles=cpu_.step(); clint_.tick(cycles); return cycles;
    }
    uint64_t run(uint64_t max_instructions){ for(uint64_t i=0;i<max_instructions && !simctrl_.stopped();++i) step(); return cpu_.instret(); }
};

} // namespace minimpu
