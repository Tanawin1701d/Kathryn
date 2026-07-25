// simharness main loop: clocks the verilated DUT and serves its memory bus
// from the C++ device models (see README.md for the port contract / plusargs).
// The model is always built with `--prefix Vdut`, so this file is DUT-agnostic.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

#include <verilated.h>
#include "Vdut.h"
#if VM_TRACE
#include <verilated_vcd_c.h>
#endif

#include "devices.h"

// ---- plusarg helpers --------------------------------------------------------

static const char *plusarg_str(int argc, char **argv, const char *name) {
    std::string key = std::string("+") + name + "=";
    for (int i = 1; i < argc; ++i)
        if (std::strncmp(argv[i], key.c_str(), key.size()) == 0)
            return argv[i] + key.size();
    return nullptr;
}

static bool plusarg_flag(int argc, char **argv, const char *name) {
    std::string key = std::string("+") + name;
    for (int i = 1; i < argc; ++i)
        if (key == argv[i]) return true;
    return false;
}

static uint64_t plusarg_u64(int argc, char **argv, const char *name, uint64_t dflt) {
    const char *s = plusarg_str(argc, argv, name);
    return s ? std::strtoull(s, nullptr, 0) : dflt;
}

// ---- stdin → UART RX --------------------------------------------------------

static void poll_stdin(Uart8250 &uart) {
    char buf[64];
    ssize_t n = ::read(STDIN_FILENO, buf, sizeof(buf));
    for (ssize_t i = 0; i < n; ++i) uart.push_rx(uint8_t(buf[i]));
}

// ---- main -------------------------------------------------------------------

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);

    const bool     quiet        = plusarg_flag(argc, argv, "quiet");
    const bool     bus_log      = plusarg_flag(argc, argv, "bus-log");
    const uint64_t ram_base     = plusarg_u64(argc, argv, "ram-base",     0x80000000ull);
    const uint64_t ram_size     = plusarg_u64(argc, argv, "ram-size",     0x8000000ull);
    const uint64_t clint_base   = plusarg_u64(argc, argv, "clint-base",   0x2000000ull);
    const uint64_t uart_base    = plusarg_u64(argc, argv, "uart-base",    0x10000000ull);
    const uint64_t mtime_div    = plusarg_u64(argc, argv, "mtime-div",    1);
    const uint64_t max_cycles   = plusarg_u64(argc, argv, "max-cycles",   0);
    const uint64_t reset_cycles = plusarg_u64(argc, argv, "reset-cycles", 4);

    Ram      ram (ram_base, ram_size);
    Uart8250 uart(uart_base);
    Clint    clint(clint_base, mtime_div);
    Htif     htif;
    htif.addr = plusarg_u64(argc, argv, "tohost", 0);

    // +image=<file>@<hexaddr>, repeatable
    for (int i = 1; i < argc; ++i) {
        if (std::strncmp(argv[i], "+image=", 7) != 0) continue;
        std::string spec  = argv[i] + 7;
        auto        at    = spec.rfind('@');
        if (at == std::string::npos) { std::fprintf(stderr, "simharness: +image needs <file>@<hexaddr>\n"); return 2; }
        uint64_t    addr  = std::strtoull(spec.c_str() + at + 1, nullptr, 16);
        if (!ram.load_image(spec.substr(0, at), addr)) return 2;
        if (!quiet) std::fprintf(stderr, "simharness: loaded %s @0x%llx\n",
                                 spec.substr(0, at).c_str(), (unsigned long long)addr);
    }

    // non-blocking stdin for interactive console input
    ::fcntl(STDIN_FILENO, F_SETFL, ::fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK);

    auto *top = new Vdut;

#if VM_TRACE
    VerilatedVcdC *vcd      = nullptr;
    const char    *vcd_path = plusarg_str(argc, argv, "vcd");
    if (vcd_path) {
        Verilated::traceEverOn(true);
        vcd = new VerilatedVcdC;
        top->trace(vcd, 99);
        vcd->open(vcd_path);
    }
#endif

    top->clk  = 0;
    top->mrst = 1;
    top->mem_ack   = 0;
    top->mem_rdata = 0;
    top->mtip = top->msip = top->meip = 0;

    uint64_t cycle = 0;
    int      rc    = 124;                                  // watchdog default

    while (!Verilated::gotFinish()) {
        top->clk = 0; top->eval();
#if VM_TRACE
        if (vcd) vcd->dump(cycle * 10);
#endif
        top->clk = 1; top->eval();
#if VM_TRACE
        if (vcd) vcd->dump(cycle * 10 + 5);
#endif
        ++cycle;
        if (cycle == reset_cycles) top->mrst = 0;

        clint.tick();
        top->mtip = clint.mtip();
        top->msip = clint.msip_pending();
        top->meip = 0;

        // Serve the bus: request sampled this posedge → ack/rdata next cycle.
        if (top->mem_req && !top->mem_ack) {
            uint64_t addr = top->mem_addr;
            int      size = 1 << top->mem_size;
            if (bus_log)
                std::fprintf(stderr, "[%8llu] %s addr=%llx size=%d wdata=%llx\n",
                             (unsigned long long)cycle, top->mem_we ? "W" : "R",
                             (unsigned long long)addr, size,
                             (unsigned long long)top->mem_wdata);
            if (top->mem_we) {
                uint64_t v = top->mem_wdata;
                if      (ram.contains(addr))   ram.write(addr, size, v);
                else if (uart.contains(addr))  uart.write(addr, size, v);
                else if (clint.contains(addr)) clint.write(addr, size, v);
                htif.check_write(addr, v);
            } else {
                uint64_t v = 0;
                if      (ram.contains(addr))   v = ram.read(addr, size);
                else if (uart.contains(addr))  v = uart.read(addr, size);
                else if (clint.contains(addr)) v = clint.read(addr, size);
                top->mem_rdata = v;
            }
            top->mem_ack = 1;
        } else if (!top->mem_req) {
            top->mem_ack = 0;
        }

        if ((cycle & 0xFFF) == 0) poll_stdin(uart);

        if (htif.done)                        { rc = htif.exit_rc; break; }
        if (max_cycles && cycle >= max_cycles) {
            if (!quiet) std::fprintf(stderr, "simharness: watchdog at %llu cycles\n",
                                     (unsigned long long)cycle);
            rc = 124; break;
        }
    }

    if (!quiet) std::fprintf(stderr, "simharness: done rc=%d after %llu cycles\n",
                             rc, (unsigned long long)cycle);
#if VM_TRACE
    if (vcd) vcd->close();
#endif
    top->final();
    delete top;
    return rc;
}
