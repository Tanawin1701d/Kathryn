//
// Created by tanawin on 11/12/2566.
//

#ifndef KATHRYN_MODELTEST1_H
#define KATHRYN_MODELTEST1_H

#include "test.h"
#include "model/controller/controller.h"
#include "model/FlowBlock/seq/seq.h"
#include "model/hwComponent/module/module.h"
#include "model/hwComponent/abstract/makeComponent.h"
#include "application/visualizer/vis.h"

namespace kathryn{

    class testMod: public Module{
        makeReg(a, 32);
        makeReg(b, 64);
        makeReg(c, 16);
    public:
        explicit testMod(int x): Module(){

        }

        void flow() override {
            seq{
                a <<= b;
                b <<= c;
                c <<= a;
            }
        }

    };

    class testMod2: public Module{
        makeReg(a, 32);
        makeReg(b, 64);
        makeReg(c, 16);
    public:
        explicit testMod2(int x): Module(){

        }

        void flow() override {
            par{
                a <<= b;
                b <<= c;
                c <<= a;
            }
        }

    };

    class testMod3: public Module{
        makeReg(a, 32);
        makeReg(b, 64);
        makeReg(c, 16);
    public:
        explicit testMod3(int x): Module(){

        }

        void flow() override {
            seq {
                par {
                    a <<= b;
                    b <<= c;
                    c <<= a;
                }
                par{
                    b <<= c;
                };

            }
        }

    };

    class test1: public Test{

    public:
        void test() override{
            makeMod(tm, testMod3, 0);

            auto testVis = Vis(&tm);
            testVis.execute();
            testVis.print();

        }

    };

}

#endif //KATHRYN_MODELTEST1_H
