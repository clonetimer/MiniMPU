#pragma once
#include "bus.hpp"
#include <cstdint>
#include <stdexcept>
#include <vector>
namespace minimpu {

class WordCache64 {
public:
    struct Stats { uint64_t hits=0, misses=0, bypass=0, writes=0; };
private:
    struct Line { bool valid=false; uint64_t tag=0; uint64_t data=0; };
    Bus& bus_; uint64_t base_, limit_; std::vector<Line> lines_; Stats stats_{};
    bool cacheable(uint64_t a) const { return a>=base_ && a<limit_; }
    size_t index(uint64_t a) const { return size_t((a>>3u)%lines_.size()); }
    uint64_t tag(uint64_t a) const { return (a>>3u)/lines_.size(); }
public:
    WordCache64(Bus& bus, size_t lines, uint64_t base=0x80000000ull, uint64_t limit=0x84000000ull)
        : bus_(bus),base_(base),limit_(limit),lines_(lines) { if(lines==0) throw std::invalid_argument("cache lines"); }
    uint64_t read64(uint64_t address){
        const uint64_t a=address&~7ull;
        if(!cacheable(a)){++stats_.bypass;return bus_.read(a,8);}
        auto& l=lines_[index(a)]; const uint64_t t=tag(a);
        if(l.valid&&l.tag==t){++stats_.hits;return l.data;}
        ++stats_.misses;l.valid=true;l.tag=t;l.data=bus_.read(a,8);return l.data;
    }
    void write64(uint64_t address,uint64_t value,uint8_t strobe=0xff){
        const uint64_t a=address&~7ull; ++stats_.writes;
        for(unsigned i=0;i<8;++i)if(strobe&(1u<<i))bus_.write(a+i,1,(value>>(i*8u))&0xffu);
        if(cacheable(a)){auto& l=lines_[index(a)];if(l.valid&&l.tag==tag(a))for(unsigned i=0;i<8;++i)if(strobe&(1u<<i))l.data=(l.data&~(0xffull<<(i*8u)))|(uint64_t(uint8_t(value>>(i*8u)))<<(i*8u));}
        else ++stats_.bypass;
    }
    void flush(){for(auto& l:lines_)l.valid=false;}
    const Stats& stats()const{return stats_;}
};

struct AxiSingleBeatContract {
    static bool address_aligned(uint64_t address){ return (address&7ull)==0; }
    static bool legal(unsigned len,unsigned size,unsigned burst,bool last){ return len==0&&size==3&&burst==1&&last; }
};
} // namespace minimpu
