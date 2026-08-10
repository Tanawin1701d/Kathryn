#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class UartTx71: public Module{
    public:
        const int _cyclesPerBit;
        const int _frameBits;
        mReg(shiftReg, _frameBits);
        mWire(tx, 1);
        mVal(one, 1);
        SyncMeta  syn;  //// ready/valid handshake with the controller

        UartTx71(int frequency, int baudRate, int dataBits = 8):
            _cyclesPerBit(frequency / baudRate),
            _frameBits   (dataBits + 2){}

        void flow() override{

            const int allOnes = (1 << _frameBits) - 1;
            shiftReg.makeResetEvent(allOnes);
            (tx = shiftReg(0)).asOutputGlob("uart_tx");

            pip(syn){
                seq{
                    cloop(bid, 10){
                        syWait(_cyclesPerBit - 1);
                        zif (bid != 9){
                            shiftReg <<= g(one, (shiftReg)(1, 10));
                        }
                    }
                }
            }
        }
    };


    ////////////////////////////////////////////////////////////////////////////
    // Module 2 : the controller / top (Chisel `UartTxCtrl`)
    ////////////////////////////////////////////////////////////////////////////
    class UartTxCtrl71: public Module{
    public:
        const int  _dataBits = 8;
        const int  _count    = 5;         //// send 'A' .. 'E'
        UartTx71&  uart;                  //// the transmitter submodule
        mVal(firstChar, _dataBits, 0x41); //// 'A'
        mVal(zero, 1, 0);
        mVal(one, 1, 1);

        UartTxCtrl71(int frequency, int baudRate):
            uart(mOprMod("uart", UartTx71, frequency, baudRate, _dataBits))
        {}

        void flow() override{

            cloop(lid, _count){
                zync(uart.syn){
                    uart.shiftReg <<= g(one, firstChar + lid, zero);
                }
            }
        }
    };


    ////////////////////////////////////////////////////////////////////////////
    // Testbench : decode the serial line, like UartTxCtrlSpec
    ////////////////////////////////////////////////////////////////////////////
    class sim71 :public SimAutoInterface{
    public:
        UartTxCtrl71* _md;

        sim71(UartTxCtrl71* md, int idx, const std::string& prefix, SimProxyBuildMode spb):
            SimAutoInterface(idx, 3000,
                             prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                             prefix + "simAutoResult"+std::to_string(idx)+".prof",
                             spb),
            _md(md)
        {}

        void describeCon() override{

            //// frame geometry taken straight from the DUT, so the decode follows any reconfig
            const int cpb   = _md->uart._cyclesPerBit;   //// cycles per bit          (8)
            const int nData = _md->_dataBits;            //// data bits per frame     (8)
            const int nMsg  = _md->_count;               //// bytes streamed 'A'..'E' (5)
            const int base  = 'A';                       //// first character sent

            //// advance `adv` cycles, settle the end of that cycle, and sample the serial line
            auto sample = [&](int adv) -> int {
                conNextCycle(adv);
                conEndCycle();
                return (int) ull(_md->uart.tx);
            };

            //// The first frame is presented almost immediately (the concrete sim starts a
            //// couple of cycles in), so we do NOT pre-wait for an idle-high line -- the
            //// per-byte falling-edge hunt below re-synchronises on every start bit anyway.

            //// decode each byte exactly like UartTxCtrlSpec: find the start bit, then
            //// sample the start / data / stop bits at their centres.
            for (int k = 0; k < nMsg; k++){
                const char expected = (char)(base + k);

                //// 1) hunt for the start bit -- the falling edge off the idle-high line
                int guard = 0;
                while (sample(1) != 0){
                    if (++guard > 2000){
                        testAndPrint(std::string("byte ") + expected + ": start bit not found", 0, 1);
                        return;
                    }
                }

                //// 2) step to the centre of the start bit; it must be low
                int startBit = sample(cpb / 2);
                testAndPrint(std::string("byte ") + expected + ": start bit low", (ull) startBit, 0ULL);

                //// 3) sample the 8 data bits at their centres, LSB first
                int rx = 0;
                for (int b = 0; b < nData; b++){
                    rx |= (sample(cpb) & 1) << b;
                }

                //// 4) one bit later the stop bit centre must be high
                int stopBit = sample(cpb);
                testAndPrint(std::string("byte ") + expected + ": stop bit high", (ull) stopBit, 1ULL);

                //// 5) the reconstructed byte must be 'A' + k
                testAndPrint(std::string("byte ") + expected + " decoded", (ull) rx, (ull) expected);
            }

            //// 6) once the message drains the line rests idle-high again
            int tail = sample(cpb);
            testAndPrint("line idle-high after message", (ull) tail, 1ULL);
        }
    };


    class Sim71TestEle: public AutoTestEle{
    public:
        explicit Sim71TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode spb) override{
            //// reconfigurable, exactly like `new UartTxCtrl(frequency = 8, baudRate = 1)`
            //// in UartTxCtrlSpec -> cyclesPerBit = 8, dataBits = 8 -> 10-bit frame.
            mMod(d, UartTxCtrl71, 8, 1);
            startModelKathryn();
            sim71 simulator((UartTxCtrl71*) &d, _simId, prefix, spb);
            simulator.simStart();
        }
    };

    Sim71TestEle ele71(-1);

}
