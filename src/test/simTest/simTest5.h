//
// Created by tanawin on 30/1/2567.
//

#ifndef KATHRYN_SIMTEST5_H
#define KATHRYN_SIMTEST5_H


#include "test/test.h"
#include "model/controller/controller.h"
#include "model/FlowBlock/seq/seq.h"
#include "model/FlowBlock/time/wait.h"
#include "model/hwComponent/module/module.h"
#include "model/hwComponent/abstract/makeComponent.h"
#include "application/visualizer/vis.h"
#include "util/logger/logger.h"
#include "sim/interface/simInterface.h"


namespace kathryn{

    class testSimMod: public Module{
    public:

        makeVal(bnk, 32, 48);
        makeVal(zero, 32, 0);
        makeReg(a , 32);
        makeReg(b , 32);

        explicit testSimMod(int x): Module(){}

        void flow() override{

            seq{
                a <<= zero;
                cwhile(a < bnk){
                    makeVal(one, 32, 1);
                    a <<= a + one;
                }
                b <<= bnk;
            }

        }

    };

    static std::string vcdPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/simTest5.vcd";

    class sim1 :public SimInterface{
    public:
        testSimMod* _md = nullptr;
        sim1(testSimMod* md):SimInterface(100, vcdPath),
                             _md(md){
            assert(_md != nullptr);
        }

        void describe() override{

            incCycle(2);

            for (int i = 0; i < 100; i++) {
                sim {
                    //_md->iv.sv() = NumConverter::cvtStrToValRep(8, 7);
                };
                incCycle(1);
            }


        }
    };

    class test1: public Test{
    public:

        void test() override {
            makeMod(tm, testSimMod, 0);

            /**logger */
            auto mdLogVal = new MdLogVal();
            tm.addMdLog(mdLogVal);
            logMD("tm", mdLogVal);
            //////////////////////////////////
            sim1 s((testSimMod*)(&tm));
            s.simStart();

        }
    };



}

#endif //KATHRYN_SIMTEST5_H
