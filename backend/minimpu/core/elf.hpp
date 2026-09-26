#pragma once
#include <cstdint>
#include <cstring>
#include <fstream>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

namespace minimpu {

struct Elf64Image { uint64_t entry = 0; };

inline uint16_t rd16(const uint8_t* p){ return uint16_t(p[0]) | (uint16_t(p[1])<<8u); }
inline uint32_t rd32(const uint8_t* p){ return uint32_t(rd16(p)) | (uint32_t(rd16(p+2))<<16u); }
inline uint64_t rd64(const uint8_t* p){ return uint64_t(rd32(p)) | (uint64_t(rd32(p+4))<<32u); }

inline Elf64Image load_elf64(const std::string& path, const std::function<void(uint64_t,const uint8_t*,size_t)>& loader) {
    std::ifstream in(path, std::ios::binary); if(!in) throw std::runtime_error("cannot open ELF: "+path);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)),{});
    if(data.size()<64 || data[0]!=0x7f || data[1]!='E' || data[2]!='L' || data[3]!='F' || data[4]!=2 || data[5]!=1)
        throw std::runtime_error("expected ELF64 little-endian image");
    if(rd16(data.data()+18)!=243) throw std::runtime_error("ELF machine is not RISC-V");
    const uint64_t entry=rd64(data.data()+24), phoff=rd64(data.data()+32); const uint16_t phentsize=rd16(data.data()+54), phnum=rd16(data.data()+56);
    if(phentsize<56 || phoff+uint64_t(phentsize)*phnum>data.size()) throw std::runtime_error("invalid ELF program headers");
    for(unsigned i=0;i<phnum;++i){ const uint8_t* ph=data.data()+phoff+uint64_t(i)*phentsize; if(rd32(ph)!=1)continue;
        const uint64_t off=rd64(ph+8), vaddr=rd64(ph+16), paddr=rd64(ph+24), filesz=rd64(ph+32), memsz=rd64(ph+40), address=paddr?paddr:vaddr;
        if(filesz>memsz || off+filesz>data.size()) throw std::runtime_error("invalid ELF PT_LOAD");
        if(filesz) loader(address,data.data()+off,size_t(filesz));
        if(memsz>filesz){ std::vector<uint8_t> zero(size_t(memsz-filesz),0); loader(address+filesz,zero.data(),zero.size()); }
    }
    return {entry};
}

} // namespace minimpu
