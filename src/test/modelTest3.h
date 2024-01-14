//
// Created by tanawin on 13/1/2567.
//

#ifndef KATHRYN_MODELTEST3_H
#define KATHRYN_MODELTEST3_H

#include "test.h"
#include "model/controller/controller.h"
#include "model/FlowBlock/seq/seq.h"
#include "model/FlowBlock/loop/cbreak.h"
#include "model/hwComponent/module/module.h"
#include "model/hwComponent/abstract/makeComponent.h"
#include "application/visualizer/vis.h"

namespace kathryn{

    class testMod7: public Module{

        makeReg(a, 32);
        makeReg(b, 16);
        makeReg(c, 15);
        makeReg(d, 16);

    public:
        explicit testMod7(int x): Module(){}

        void flow() override{

        cwhile(b >= d) {
            a <<= d;
        }

        }


    };

    class testMod8: public Module{
        makeReg(a, 32);
        makeReg(b, 16);
        makeReg(c, 15);

    public:
        explicit testMod8(int x): Module(){}

        void flow() override{

            cwhile(a >= b){
                seq{
                    a <<= b;
                    b <<= c;
                    cbreak;
                }
                seq{
                    a <<= c;
                }
                par{
                    c <<= a;
                    b <<= a;
                };
            }

        }
    };

    class test3: public Test{

    public:
        void test() override{
            makeMod(tm, testMod8, 0);

            auto mdLogVal = new MdLogVal();
            tm.addMdLog(mdLogVal);
            logMD("tm", mdLogVal);

        }

    };

}


#endif //KATHRYN_MODELTEST3_H
