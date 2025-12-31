//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod23: public Module{
    public:
        mVal(hf1, 8, 36);
        mVal(hf2, 8, 12);
        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        mWire(d, 8);
        explicit testGenMod23(int x): Module(){
            a.asOutputGlob("a");
            b.asOutputGlob("b");
            c.asOutputGlob("c");
            d.asOutputGlob("d");
        }

        void flow() override{

            ////////// we use d as a interrupt
            seq{
                par{
                    a = 0;
                    b = 0;
                    c = 0;
                    d = 0;
                }

                a <<= 1;
                cdowhile(c <= 5){
                    par{
                        b = c;
                        c <<= c + 1;
                    }
                }

            }


        }

    };

    class GenEle23: public GenEle{
    public:
        explicit GenEle23(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod23, 1);
        }
    };

    GenEle23 testGen23(23);

}