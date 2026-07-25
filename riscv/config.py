# Core / SoC configuration. Everything the generator can vary lives here —
# feature gating is plain elaboration-time Python (`if cfg.ext_m:`), so a config
# change regenerates different hardware with no HDL-level parameterization.

from __future__ import annotations

from dataclasses import dataclass, field

MiB = 1024 * 1024


@dataclass(frozen=True)
class CoreConfig:
    xlen            : int  = 64            # datapath width (only 64 exercised)
    ext_m           : bool = False         # M: integer mul/div        (M3)
    ext_a           : bool = False         # A: atomics                (M3)
    ext_c           : bool = False         # C: compressed             (M6, unimplemented)
    ext_zicsr       : bool = False         # CSRs + traps              (M2)
    ext_zifencei    : bool = False         # fence.i (nop — no caches) (M2)
    reset_vector    : int  = 0x8000_0000
    boot_regs       : dict = field(default_factory=dict)   # {reg_idx: value} preset at reset
    iterative_muldiv: bool = False         # reserved: multi-cycle mul/div


@dataclass(frozen=True)
class SocConfig:
    core         : CoreConfig = field(default_factory=CoreConfig)
    mem_backend  : str        = "external" # "external": C++ bus (simharness) | "dsl": mem_blk RAM
    ram_base     : int        = 0x8000_0000
    ram_size     : int        = 128 * MiB
    clint_base   : int        = 0x0200_0000
    uart_base    : int        = 0x1000_0000
    mem_init_file: str | None = None       # dsl backend: $readmemh image


PRESETS: dict[str, SocConfig] = {
    # Minimal RV64I on the external C++ bus — unit tests / bring-up.
    "rv64i_min": SocConfig(core=CoreConfig()),
    # RV64I + Zicsr/Zifencei — riscv-tests p-env style suites (rv64ui/rv64mi).
    "rv64i_zicsr": SocConfig(core=CoreConfig(ext_zicsr=True, ext_zifencei=True)),
    # Full Linux-boot target: boot regs preset a0=hartid(0), a1=DTB address
    # (loaded 126 MiB into RAM by the harness; see tools/linux_image/).
    "linux": SocConfig(core=CoreConfig(
        ext_m=True, ext_a=True, ext_zicsr=True, ext_zifencei=True,
        boot_regs={11: 0x87E0_0000})),
}
