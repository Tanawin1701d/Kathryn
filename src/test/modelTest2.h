//
// Created by tanawin on 12/12/2566.
//

#ifndef KATHRYN_MODELTEST2_H
#define KATHRYN_MODELTEST2_H

#include "test.h"
#include "model/controller/controller.h"
#include "model/FlowBlock/seq/seq.h"
#include "model/hwComponent/module/module.h"
#include "model/hwComponent/abstract/makeComponent.h"
#include "application/visualizer/vis.h"


namespace kathryn{

    class testMod5: public Module{
        makeReg(a, 32);
        makeReg(b, 64);
        makeReg(c, 16);
        makeReg(d, 21);
    public:
        explicit testMod5(int x): Module(){

        }

        void flow() override {
            seq{
                a <<= b;
                cwhile(a < b){
                    par{
                        c <<= a;
                        d <<= a;
                    }
                }
            }
        }
    };

    class testMod6: public Module{
        makeReg(a, 32);
        makeReg(b, 64);
        makeReg(c, 16);
        makeReg(d, 21);
    public:
        explicit testMod6(int x): Module(){

        }

        void flow() override {
            seq{
                a <<= b;
                cif(a == b){
                    par{
                        b <<= c;
                        c <<= a;
                    }
                }celif(b < c){
                    seq{
                        a <<= c;
                        b <<= c;
                        d <<= a;
                    }
                }celse{
                    a <<= b;
                };
                d <<= b;
            }
        }
    };

    class test2: public Test{

    public:
        void test() override{
            makeMod(tm, testMod6, 0);

            auto testVis = Vis(&tm);
            testVis.execute();
            testVis.print();

        }

    };

}

#endif //KATHRYN_MODELTEST2_H
