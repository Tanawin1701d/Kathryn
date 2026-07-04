# tc35 — kathryn.lib Bundle + Decoupled: a consumer channel (in_ch) accumulates
# payload on every fire into `acc`; the value flows through a plain-Bundle
# connect_from chain to a producer channel (out_ch) whose fires are counted.
# Exercises: consumer/producer IO marking, fire(), connect_from, backpressure.

from __future__ import annotations

from kathryn import *
from kathryn import emit_verilog
from kathryn.lib import Bundle, Decoupled

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import RisingEdge, Timer

import cocotb_pool

NAME = "tc35_lib_bundle_handshake"


# ---- model -------------------------------------------------------------------
class tc35_lib_bundle_handshake(Module):
    @init
    def com_declare(self):
        self.in_ch  = Decoupled("in_ch",  {"data": 8}).mark_consumer_io()
        self.out_ch = Decoupled("out_ch", {"data": 8}).mark_producer_io()

        self.acc  = reg(8, "acc")
        self.sent = reg(8, "sent")

        self.mid  = Bundle("mid",  {"data": 8})
        self.outb = Bundle("outb", {"data": 8})

        self.acc.mark_output ("acc_out")
        self.sent.mark_output("sent_out")

    @flow
    def my_flow(self):
        self.acc.reset(0)
        self.sent.reset(0)

        # consumer side: always ready; accumulate payload on fire
        self.in_ch.ready *= 1
        with zif(self.in_ch.fire()):
            self.acc |= self.acc + self.in_ch.data

        # plain-bundle relay: acc -> mid -> (connect_from) outb -> out_ch payload
        self.mid.data *= self.acc
        self.outb.connect_from(self.mid)

        # producer side: always offering the current acc; count accepted beats
        self.out_ch.valid *= 1
        self.out_ch.data  *= self.outb.data
        with zif(self.out_ch.fire()):
            self.sent |= self.sent + 1


# ---- build -------------------------------------------------------------------
def build(output_folder: str) -> None:
    reset()
    build_model(tc35_lib_bundle_handshake())
    emit_verilog(output_folder)


# ---- simulation (cocotb) -----------------------------------------------------
async def _reset(dut):
    cocotb.start_soon(Clock(dut.clk, 10, unit="ns").start())
    dut.mrst.value         = 1
    dut.in_ch_valid.value  = 0
    dut.in_ch_data.value   = 0
    dut.out_ch_ready.value = 0
    await RisingEdge(dut.clk)
    await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    dut.mrst.value = 0


@cocotb.test()
async def check_consumer_fire(dut):
    # Each single-cycle valid beat accumulates exactly once; gaps do nothing.
    await _reset(dut)
    await Timer(1, unit="ns")
    assert int(dut.in_ch_ready.value) == 1, "consumer must be always-ready"

    total = 0
    for d in (5, 7, 11):
        dut.in_ch_valid.value = 1
        dut.in_ch_data.value  = d
        await RisingEdge(dut.clk)
        dut.in_ch_valid.value = 0
        total += d
        await RisingEdge(dut.clk)              # idle gap — must not re-fire
        await Timer(1, unit="ns")
        assert int(dut.acc_out.value) == total, \
            f"acc = {dut.acc_out.value!s} (expected {total})"


@cocotb.test()
async def check_producer_backpressure(dut):
    # out_ch.valid is constant-high; `sent` advances only while ready is high,
    # and the payload equals acc relayed through the connect_from chain.
    await _reset(dut)

    # push one beat of 9 through the consumer so acc = 9
    dut.in_ch_valid.value = 1
    dut.in_ch_data.value  = 9
    await RisingEdge(dut.clk)
    dut.in_ch_valid.value = 0
    await Timer(1, unit="ns")
    assert int(dut.out_ch_valid.value) == 1
    assert int(dut.out_ch_data.value)  == 9, \
        f"payload relay broken: {dut.out_ch_data.value!s}"

    # ready low → sent frozen
    for _ in range(4):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    assert int(dut.sent_out.value) == 0, "sent advanced without ready"

    # ready high for exactly 3 cycles → sent == 3
    dut.out_ch_ready.value = 1
    for _ in range(3):
        await RisingEdge(dut.clk)
    await Timer(1, unit="ns")
    dut.out_ch_ready.value = 0
    assert int(dut.sent_out.value) == 3, \
        f"sent = {dut.sent_out.value!s} (expected 3)"


# ---- register into the shared pool ------------------------------------------
cocotb_pool.register(NAME, build, __name__)
