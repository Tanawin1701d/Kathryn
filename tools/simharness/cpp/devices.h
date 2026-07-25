// C++ device models behind the simharness bus: flat RAM, 8250 UART, CLINT,
// and the riscv-tests HTIF tohost watcher. All accesses are little-endian,
// byte-addressed, size in {1,2,4,8}.

#pragma once

#include <cstdint>
#include <cstdio>
#include <deque>
#include <string>
#include <vector>

// ---- RAM --------------------------------------------------------------------
struct Ram {
    uint64_t             base;
    std::vector<uint8_t> data;

    Ram(uint64_t base_, uint64_t size_) : base(base_), data(size_, 0) {}

    bool     contains(uint64_t a) const { return a >= base && a < base + data.size(); }
    uint64_t read (uint64_t a, int size) const;
    void     write(uint64_t a, int size, uint64_t v);
    bool     load_image(const std::string &path, uint64_t addr);   // raw binary
};

// ---- 8250 / 16550 UART ------------------------------------------------------
// 1-byte registers at base+0..7 (reg-shift 0). TX bytes go straight to stdout;
// RX is fed by the harness main loop (stdin). LSR always reports THR empty.
struct Uart8250 {
    uint64_t            base;
    std::deque<uint8_t> rx;
    uint8_t             ier = 0, lcr = 0, mcr = 0, scr = 0;
    uint8_t             dll = 1, dlm = 0;                 // divisor latch (DLAB)

    explicit Uart8250(uint64_t base_) : base(base_) {}

    bool     contains(uint64_t a) const { return a >= base && a < base + 8; }
    uint64_t read (uint64_t a, int size);
    void     write(uint64_t a, int size, uint64_t v);
    void     push_rx(uint8_t b) { rx.push_back(b); }
    bool     irq_pending() const {
        return ((ier & 0x1) && !rx.empty()) || ((ier & 0x2) != 0);   // RDA | THRE
    }
};

// ---- CLINT ------------------------------------------------------------------
// Single hart: msip@+0x0, mtimecmp@+0x4000, mtime@+0xBFF8 (SiFive layout).
struct Clint {
    uint64_t base;
    uint64_t mtime    = 0;
    uint64_t mtimecmp = ~0ull;
    uint32_t msip     = 0;
    uint64_t div      = 1, div_cnt = 0;                   // mtime tick divider

    explicit Clint(uint64_t base_, uint64_t div_) : base(base_), div(div_ ? div_ : 1) {}

    bool     contains(uint64_t a) const { return a >= base && a < base + 0xC000; }
    uint64_t read (uint64_t a, int size) const;
    void     write(uint64_t a, int size, uint64_t v);
    void     tick() { if (++div_cnt >= div) { div_cnt = 0; ++mtime; } }
    bool     mtip() const { return mtime >= mtimecmp; }
    bool     msip_pending() const { return (msip & 1) != 0; }
};

// ---- HTIF tohost ------------------------------------------------------------
// riscv-tests convention: write 1 = pass; odd value n = fail with code n>>1.
struct Htif {
    uint64_t addr    = 0;                                 // 0 = disabled
    bool     done    = false;
    int      exit_rc = 0;

    void check_write(uint64_t a, uint64_t v) {
        if (!addr || a != addr || done) return;
        done    = true;
        exit_rc = (v == 1) ? 0 : int(v >> 1);
    }
};
