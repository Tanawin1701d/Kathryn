//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod19: public Module{
    public:
        mVal(hf1, 8, 36);
        mVal(hf2, 8, 12);
        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        mWire(d, 1);
        explicit testGenMod19(int x): Module(){
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

                cwhile((c == 1) & (a < 48)){
                    seq{ intrStart(d)
                        c = 1;
                        a <<= a + 1;

                    }
                }

            }

            seq{
                syWait(5);
                d = 1;
            }


        }

    };

    class GenEle19: public GenEle{
    public:
        explicit GenEle19(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod19, 1);
        }
    };

    GenEle19 testGen19(19);

}