#include "elf.hpp"
#include "soc.hpp"
#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
std::string json_escape(const std::string& s) {
    std::ostringstream o;
    for (unsigned char c : s) {
        switch (c) {
        case '\\': o << "\\\\"; break;
        case '"': o << "\\\""; break;
        case '\n': o << "\\n"; break;
        case '\r': o << "\\r"; break;
        case '\t': o << "\\t"; break;
        default:
            if (c < 0x20) {
                static const char* hex = "0123456789abcdef";
                o << "\\u00" << hex[c >> 4] << hex[c & 15];
            } else o << char(c);
        }
    }
    return o.str();
}

std::string hx(uint64_t v) {
    std::ostringstream o; o << "0x" << std::hex << v; return o.str();
}

const char* priv_name(minimpu::PrivilegeMode p) {
    switch (p) {
    case minimpu::PrivilegeMode::Machine: return "M";
    case minimpu::PrivilegeMode::Supervisor: return "S";
    case minimpu::PrivilegeMode::User: return "U";
    default: return "?";
    }
}

class Engine {
    std::string kernel_, user_bin_;
    std::unique_ptr<minimpu::SoC> soc_;
    std::string uart_;
public:
    Engine(std::string kernel, std::string user_bin) : kernel_(std::move(kernel)), user_bin_(std::move(user_bin)) { reset(); }

    void reset() {
        uart_.clear();
        soc_ = std::make_unique<minimpu::SoC>(64*1024*1024, [this](uint8_t b){ uart_.push_back(char(b)); });
        auto load = [this](uint64_t a, const uint8_t* p, size_t n){ soc_->load(a,p,n); };
        auto k = minimpu::load_elf64(kernel_, load);
        std::ifstream uf(user_bin_, std::ios::binary);
        if (!uf) throw std::runtime_error("cannot open user binary: "+user_bin_);
        std::vector<uint8_t> ub((std::istreambuf_iterator<char>(uf)), {});
        if (!ub.empty()) soc_->load(0x80200000ull, ub.data(), ub.size());
        soc_->cpu().set_pc(k.entry);
    }

    void step(uint64_t n) {
        for (uint64_t i=0; i<n && !soc_->simctrl().stopped(); ++i) soc_->step();
    }

    std::string state() const {
        const auto& c = soc_->cpu();
        std::ostringstream o;
        o << "{\"pc\":\"" << hx(c.pc()) << "\""
          << ",\"cycle\":" << c.cycle()
          << ",\"instret\":" << c.instret()
          << ",\"privilege\":\"" << priv_name(c.mode()) << "\""
          << ",\"stopped\":" << (soc_->simctrl().stopped()?"true":"false")
          << ",\"exit_code\":" << soc_->simctrl().code()
          << ",\"uart\":\"" << json_escape(uart_) << "\"";
        o << ",\"regs\":[";
        for (unsigned i=0;i<32;++i) { if(i) o << ','; o << "\"" << hx(c.reg(i)) << "\""; }
        o << "]";
        o << ",\"csrs\":{";
        struct C { const char* n; unsigned a; } csrs[] = {
            {"mstatus",0x300},{"misa",0x301},{"medeleg",0x302},{"mideleg",0x303},{"mie",0x304},{"mtvec",0x305},
            {"mepc",0x341},{"mcause",0x342},{"mtval",0x343},{"stvec",0x105},{"sepc",0x141},{"scause",0x142},
            {"stval",0x143},{"satp",0x180}
        };
        for (size_t i=0;i<sizeof(csrs)/sizeof(csrs[0]);++i) {
            if(i) o << ',';
            o << "\"" << csrs[i].n << "\":\"" << hx(c.csr_debug(csrs[i].a)) << "\"";
        }
        o << "}}";
        return o.str();
    }

    uint64_t mem(uint64_t addr, unsigned size) { return soc_->bus().read(addr,size); }
};
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "usage: minimpu_studio_bridge <kernel.elf> <user.bin>\n";
        return 2;
    }
    try {
        Engine engine(argv[1], argv[2]);
        std::string line;
        while (std::getline(std::cin, line)) {
            try {
                std::istringstream is(line);
                std::string cmd; is >> cmd;
                if (cmd == "QUIT") break;
                if (cmd == "STATE") {
                    std::cout << engine.state() << '\n' << std::flush;
                } else if (cmd == "RESET") {
                    engine.reset(); std::cout << engine.state() << '\n' << std::flush;
                } else if (cmd == "STEP") {
                    uint64_t n=1; is >> n; engine.step(n); std::cout << engine.state() << '\n' << std::flush;
                } else if (cmd == "MEM") {
                    std::string a; unsigned size=8; is >> a >> size;
                    auto v=engine.mem(std::stoull(a,nullptr,0),size);
                    std::cout << "{\"value\":\"" << hx(v) << "\"}\n" << std::flush;
                } else {
                    std::cout << "{\"error\":\"unknown command\"}\n" << std::flush;
                }
            } catch (const std::exception& e) {
                std::cout << "{\"error\":\"" << json_escape(e.what()) << "\"}\n" << std::flush;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n'; return 1;
    }
    return 0;
}
