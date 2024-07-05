//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod12: public Module{
    public:
        mVal(hf1, 8, 36);
        mVal(hf2, 8, 12);
        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        mReg(d, 8);
        explicit testGenMod12(int x): Module(){
            a.asOutputGlob("a");
            b.asOutputGlob("b");
            c.asOutputGlob("c");
            d.asOutputGlob("d");
        }

        void flow() override{

            seq{
                par{
                    a = 0;
                    b = 10;
                    c = 0;
                    d = 0;
                }
                par{
                    cwhile(a < 48){
                        zif(a < b){
                            c <<= c + 1;
                        }zelse{
                            d <<= d + 1;
                        }
                        a <<= a + 1;
                    }
                }
            }
        }

    };

    class GenEle12: public GenEle{
    public:
        explicit GenEle12(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod12, 1);
        }
    };

    GenEle12 testGen12(12);

}