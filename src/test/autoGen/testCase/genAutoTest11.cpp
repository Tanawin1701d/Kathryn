//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod11: public Module{
    public:
        mVal(hf1, 8, 36);
        mVal(hf2, 8, 12);
        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        mReg(d, 8);
        explicit testGenMod11(int x): Module(){
            a.asOutputGlob("a");
            b.asOutputGlob("b");
            c.asOutputGlob("c");
            d.asOutputGlob("d");
        }

        void flow() override{

            seq{
                par{
                    a = 14;
                    b = 0;
                    c = 0;
                    d = 0;
                }
                par{
                    sif(a < hf2){
                        b <<= b + 1;
                    }selif( a < hf1){
                        cif (d == 0){
                            d <<= d + 1;
                        }celse{
                            d <<= d + 2;
                        }
                    }selse{
                        c <<= c + 1;
                    }

                }
            }
        }

    };

    class GenEle11: public GenEle{
    public:
        explicit GenEle11(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod11, 1);
        }
    };

    GenEle11 testGen11(11);

}