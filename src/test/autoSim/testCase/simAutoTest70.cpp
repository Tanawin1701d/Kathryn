//
// >64-bit end-to-end datapath test (128-bit regs, wide literals, wide ops).
// Exercises the UintX<N> runtime (+,-,*,/,%,<<,slice,>) and the wide-literal
// input API through the full model -> generated-sim pipeline.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"

namespace kathryn{

    class testSimMod70: public Module{
    public:
        mReg(a,       128);
        mReg(b,       128);
        mReg(sum,     128);
        mReg(diff,    128);
        mReg(prod,    128);
        mReg(quo,     128);
        mReg(rem,     128);
        mReg(shifted, 128);
        mReg(sliced,  128);
        mReg(cmp,       1);

        //// wide literals via the new string API (underscores are ignored)
        mVal(big,   128, "0x00000000_00000002_00000000_00000003");   // 2*2^64 + 3  -> {3,2}
        mVal(small, 128, 5);                                          // ull literal into a wide const
        mVal(two,   128, 2);
        mVal(mask,  128, "0xFFFFFFFFFFFFFFFF_FFFFFFFFFFFFFFFF");      // all ones

        explicit testSimMod70(int x): Module(){}

        void flow() override{
            seq{
                a       <<= big;
                b       <<= small;
                sum     <<= a + b;              // {8, 2}
                diff    <<= a - b;              // {0xFFFFFFFFFFFFFFFE, 1}
                prod    <<= a * b;              // (2*2^64+3)*5 = {15, 10}
                quo     <<= a / two;            // (2*2^64+3)/2 = 2^64+1 = {1, 1}
                rem     <<= a % two;            // {1, 0}
                shifted <<= a << small;         // (2*2^64+3)<<5 = {96, 64}
                sliced  <<= mask(64, 128);      // upper 64 ones -> {0xFFFFFFFFFFFFFFFF, 0}
                cmp     <<= a > b;              // 1  (a.hi=2 > b.hi=0 : validates MSB-first compare)
            }
        }
    };


    class sim70 :public SimAutoInterface{
    public:
        testSimMod70* _md;

        sim70(testSimMod70* md, int idx, const std::string& prefix, SimProxyBuildMode spb):
            SimAutoInterface(idx, 100,
                             prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                             prefix + "simAutoResult"+std::to_string(idx)+".prof",
                             spb),
            _md(md)
        {}

        void simAssert() override{
            incCycle(40);   //// well past the ~10-cycle seq
            sim {
                testAndPrint("wide a",       _md->a.v().getLargeVal(),       {3ULL, 2ULL});
                testAndPrint("wide add",     _md->sum.v().getLargeVal(),     {8ULL, 2ULL});
                testAndPrint("wide sub",     _md->diff.v().getLargeVal(),    {0xFFFFFFFFFFFFFFFEULL, 1ULL});
                testAndPrint("wide mul",     _md->prod.v().getLargeVal(),    {15ULL, 10ULL});
                testAndPrint("wide div",     _md->quo.v().getLargeVal(),     {1ULL, 1ULL});
                testAndPrint("wide mod",     _md->rem.v().getLargeVal(),     {1ULL, 0ULL});
                testAndPrint("wide shl",     _md->shifted.v().getLargeVal(), {96ULL, 64ULL});
                testAndPrint("wide slice",   _md->sliced.v().getLargeVal(),  {0xFFFFFFFFFFFFFFFFULL, 0ULL});
                testAndPrint("wide gt(MSB)", (ull)_md->cmp, 1);
            };
        }

        void simDriven() override{}
    };


    class Sim70TestEle: public AutoTestEle{
    public:
        explicit Sim70TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode spb) override{
            mMod(d, testSimMod70, 1);
            startModelKathryn();
            sim70 simulator((testSimMod70*) &d, _simId, prefix, spb);
            simulator.simStart();
        }
    };

    Sim70TestEle ele70(70);

}
