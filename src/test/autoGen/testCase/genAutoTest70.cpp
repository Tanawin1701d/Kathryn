//
// >64-bit Verilog generation test: 128-bit datapath and a wide literal.
// Combinational so the emitted Verilog can be checked functionally without a
// clock/reset FSM. Confirms declarations emit [127:0] and valueGen emits a
// sized literal (128'h...) rather than a truncated 64-bit decimal.
//

#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod70: public Module{
    public:
        mWire(inp,  128);
        mWire(outX, 128);
        mWire(outP, 128);

        //// wide (>64-bit) constant via the string literal API
        mVal(mask, 128, "0xFFFFFFFFFFFFFFFF_0000000000000001");

        explicit testGenMod70(int x): Module(){}

        void flow() override{

            inp.asInputGlob("inp");
            outX.asOutputGlob("outX");
            outP.asOutputGlob("outP");

            outX = inp ^ mask;    //// combinational wide xor with a wide literal
            outP = inp + mask;    //// combinational wide add with a wide literal

        }

    };

    class GenEle70: public GenEle{
    public:
        explicit GenEle70(int id): GenEle(id){}

        void start(PARAM& param) override{
            mMod(myMd, testGenMod70, 1);
        }

    };

    GenEle70 testCase70(70);

}
