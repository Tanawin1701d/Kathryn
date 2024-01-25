//
// Created by tanawin on 22/1/2567.
//

#ifndef KATHRYN_SIMTEST1_H
#define KATHRYN_SIMTEST1_H

#include "test.h"
#include "model/controller/controller.h"
#include "model/FlowBlock/seq/seq.h"
#include "model/FlowBlock/time/wait.h"
#include "model/hwComponent/module/module.h"
#include "model/hwComponent/abstract/makeComponent.h"
#include "application/visualizer/vis.h"
#include "util/logger/logger.h"

namespace kathryn{

    class testSimMod: public Module{
    public:
        makeReg(a, 8);
        makeReg(b, 8);
        makeReg(c, 8);
        makeVal(iv, 8, 0b10101010);

        explicit testSimMod(int x): Module(){}

        void flow() override{

            a.setToRec();
            b.setToRec();
            c.setToRec();

            seq{
                a <<= iv;
                b <<= a;
                c <<= b;
            }

        }

    };

    class test1: public Test{
    public:

        void test() override {
            makeMod(tm, testSimMod, 0);

            /***/
            auto mdLogVal = new MdLogVal();
            tm.addMdLog(mdLogVal);
            logMD("tm", mdLogVal);
            //////////////////////////////////

        }
    };

}

#endif //KATHRYN_SIMTEST1_H
