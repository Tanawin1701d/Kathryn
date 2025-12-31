//
// Created by tanawin on 27/3/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"



namespace kathryn{

    class testSimMod37: public Module{
    public:
        mReg(a, 128);
        mReg(b, 128);
        mReg(c, 64+32);

        mReg(r0, 128);
        mReg(r1, 128);
        mReg(r2, 128);
        mReg(r3, 128);

        mReg(d0, 8);

        explicit testSimMod37(int x): Module(){}

        void flow() override{
            seq{
                a(64, 72) <<= 1;
                b(64, 72) <<= 3;
                r0 <<= a & b;
                r1 <<= a | b;
                r2 <<= a ^ b;
                r3 <<= !a;
                c  <<= ~c;
                c  <<= c >> 66;
                c  <<= c << 66;
                d0 <<= a(64, 72);

            }
        }

    };

    ///static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simAutoTest37.vcd";
    ////static std::string profilePath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/profAutoTest37.vcd";


    class sim37 :public SimAutoInterface{
    public:

        testSimMod37* _md;

        sim37(testSimMod37* md, int idx, const std::string& prefix, SimProxyBuildMode simProxyBuildMode):SimAutoInterface(idx,
                                                 300,
                                                 prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                                 prefix + "simAutoResult"+std::to_string(idx)+".prof", simProxyBuildMode),
                                _md(md)
        {}

        void describeCon() override{

            conNextCycle(1);

            ull testA = ((ValRepBase)_md->a).getLargeVal()[1];
            testAndPrint("testL_a", testA, 1);

            conNextCycle(1);

            ull testB = ((ValRepBase)_md->b).getLargeVal()[1];
            testAndPrint("testL_b", testB, 3);

            conNextCycle(1);

            ull testR0 = ((ValRepBase)_md->r0).getLargeVal()[1];
            testAndPrint("testL_r0", testR0, 1);

            conNextCycle(1);

            ull testR1 = ((ValRepBase)_md->r1).getLargeVal()[1];
            testAndPrint("testL_r1", testR1, 3);

            conNextCycle(1);

            ull testR2 = ((ValRepBase)_md->r2).getLargeVal()[1];
            testAndPrint("testL_r2", testR2, 2);

            conNextCycle(1);

            ull testR3 = ((ValRepBase)_md->r3).getLargeVal()[1];
            testAndPrint("testL_r3", testR3, 0);

            conNextCycle(1);

            ull testCH = ((ValRepBase)_md->c).getLargeVal()[1];
            ull testCL = ((ValRepBase)_md->c).getLargeVal()[0];
            testAndPrint("testH_c0", testCH, UINT32_MAX);
            testAndPrint("testL_c0", testCL, UINT64_MAX);

            conNextCycle(1);

            ull testCH_1 = ((ValRepBase)_md->c).getLargeVal()[1];
            ull testCL_1 = ((ValRepBase)_md->c).getLargeVal()[0];
            testAndPrint("testH_c1", testCH_1, 0);
            testAndPrint("testL_c1", testCL_1, 0x3FFFFFFF);

            conNextCycle(1);

            ull testCH_2 = ((ValRepBase)_md->c).getLargeVal()[1];
            ull testCL_2 = ((ValRepBase)_md->c).getLargeVal()[0];
            testAndPrint("testH_c2", testCH_2, 0xFFFFFFFC);
            testAndPrint("testL_c2", testCL_2, 0);


            conNextCycle(1);

            ull testDL_3 = ((ValRepBase)_md->d0).getVal();
            testAndPrint("testDL_3", testDL_3, 1);

            // for (int i = 0; i < 2; i++){
            //     conEndCycle();
            //     testAndPrint("dowhilePre", ull(_md->x),0);
            //     conNextCycle(1);
            // }
            // for(int i = 1; i <= 6; i++){
            //     conEndCycle();
            //     testAndPrint("dowhileVal", ull(_md->x),i);
            //     conNextCycle(1);
            // }
            // for (int i = 0; i < 2; i++){
            //     conEndCycle();
            //     testAndPrint("dowhilePost", ull(_md->x),0);
            //     conNextCycle(1);
            // }
        }
    };


    class Sim37TestEle: public AutoTestEle{
    public:
        explicit Sim37TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) override{
            mMod(d, testSimMod37, 1);
            startModelKathryn();
            sim37 simulator((testSimMod37*) &d, _simId, prefix, simProxyBuildMode);
            simulator.simStart();
        }

    };

    Sim37TestEle ele37(37);
}