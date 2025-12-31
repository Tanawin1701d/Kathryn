//
// Created by tanawin on 12/12/2566.
//

#ifndef KATHRYN_MODELTEST2_H
#define KATHRYN_MODELTEST2_H

#include "kathryn.h"
#include "test.h"



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
            par{


                
            }
        }
    };

    class testMod6: public Module{
        makeReg(a, 32);
        makeReg(b, 64);
        makeReg(c, 16);
        makeReg(d, 21);
    public:
        explicit testMod6(int x): Module(){}

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

            auto mdLogVal = new MdLogVal();
            tm.addMdLog(mdLogVal);
            logMD("tm", mdLogVal);

        }

    };

}

#endif //KATHRYN_MODELTEST2_H
