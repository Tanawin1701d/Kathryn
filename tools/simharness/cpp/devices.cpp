#include "devices.h"

#include <cstring>
#include <fstream>

// ---- RAM --------------------------------------------------------------------

uint64_t Ram::read(uint64_t a, int size) const {
    uint64_t off = a - base;
    if (off + size > data.size()) return 0;
    uint64_t v = 0;
    std::memcpy(&v, data.data() + off, size);              // little-endian host
    return v;
}

void Ram::write(uint64_t a, int size, uint64_t v) {
    uint64_t off = a - base;
    if (off + size > data.size()) return;
    std::memcpy(data.data() + off, &v, size);
}

bool Ram::load_image(const std::string &path, uint64_t addr) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) { std::fprintf(stderr, "simharness: cannot open image %s\n", path.c_str()); return false; }
    auto     len = static_cast<uint64_t>(f.tellg());
    uint64_t off = addr - base;
    if (addr < base || off + len > data.size()) {
        std::fprintf(stderr, "simharness: image %s (%llu bytes @0x%llx) outside RAM\n",
                     path.c_str(), (unsigned long long)len, (unsigned long long)addr);
        return false;
    }
    f.seekg(0);
    f.read(reinterpret_cast<char *>(data.data() + off), len);
    return true;
}

// ---- UART -------------------------------------------------------------------

uint64_t Uart8250::read(uint64_t a, int /*size*/) {
    bool dlab = (lcr & 0x80) != 0;
    switch (a - base) {
        case 0:                                            // RBR (or DLL with DLAB)
            if (dlab) return dll;
            if (rx.empty()) return 0;
            { uint8_t b = rx.front(); rx.pop_front(); return b; }
        case 1: return dlab ? dlm : ier;                   // IER / DLM
        case 2:                                            // IIR: RDA > THRE > none
            if ((ier & 0x1) && !rx.empty()) return 0x04;
            if  (ier & 0x2)                 return 0x02;
            return 0x01;
        case 3: return lcr;
        case 4: return mcr;
        case 5: return uint64_t(0x60 | (rx.empty() ? 0 : 1));  // LSR: TEMT|THRE|DR
        case 6: return 0xB0;                               // MSR: CTS|DSR|DCD
        case 7: return scr;
    }
    return 0;
}

void Uart8250::write(uint64_t a, int /*size*/, uint64_t v) {
    bool dlab = (lcr & 0x80) != 0;
    switch (a - base) {
        case 0:                                            // THR (or DLL with DLAB)
            if (dlab) { dll = uint8_t(v); break; }
            std::fputc(int(v & 0xFF), stdout);
            std::fflush(stdout);
            break;
        case 1: if (dlab) dlm = uint8_t(v); else ier = uint8_t(v); break;
        case 2: break;                                     // FCR: ignore
        case 3: lcr = uint8_t(v); break;
        case 4: mcr = uint8_t(v); break;
        case 7: scr = uint8_t(v); break;
        default: break;                                    // LSR/MSR read-only
    }
}

// ---- CLINT ------------------------------------------------------------------

uint64_t Clint::read(uint64_t a, int size) const {
    uint64_t off = a - base, v = 0;
    if      (off < 4)                          v = msip;
    else if (off >= 0x4000 && off < 0x4008)    v = mtimecmp >> ((off - 0x4000) * 8);
    else if (off >= 0xBFF8 && off < 0xC000)    v = mtime    >> ((off - 0xBFF8) * 8);
    else return 0;
    return size == 8 ? v : v & ((1ull << (size * 8)) - 1);
}

void Clint::write(uint64_t a, int size, uint64_t v) {
    uint64_t off = a - base;
    if (off < 4) { msip = uint32_t(v) & 1; return; }
    // Sub-word writes merge into the 64-bit register at the byte offset.
    auto merge = [&](uint64_t reg, uint64_t byte_off) {
        if (size == 8) return v;
        uint64_t sh   = byte_off * 8;
        uint64_t mask = ((1ull << (size * 8)) - 1) << sh;
        return (reg & ~mask) | ((v << sh) & mask);
    };
    if      (off >= 0x4000 && off < 0x4008) mtimecmp = merge(mtimecmp, off - 0x4000);
    else if (off >= 0xBFF8 && off < 0xC000) mtime    = merge(mtime,    off - 0xBFF8);
}
