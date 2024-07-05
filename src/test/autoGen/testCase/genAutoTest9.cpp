//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod9: public Module{
    public:
        mVal(hf1, 8, 36);
        mVal(hf2, 8, 12);
        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        mReg(d, 8);
        explicit testGenMod9(int x): Module(){
            a.asOutputGlob("a");
            b.asOutputGlob("b");
            c.asOutputGlob("c");
            d.asOutputGlob("d");
        }

        void flow() override{

            seq{
                par{
                    a = 0;
                    b = 0;
                    c = 0;
                    d = 0;
                }
                par{
                    cwhile(a < 48){
                        cif( a < 10){
                            b <<= b + 1;
                            a <<= a + 1;
                        }celif( a < 20){
                            c <<= c + 1;
                            a <<= a + 1;
                        }celse{
                            seq{
                                d <<= d + 1;
                                d <<= d + 1;
                                a <<= a + 1;
                            }
                        }
                    }
                }

            }
        }

    };

    class GenEle9: public GenEle{
    public:
        explicit GenEle9(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod9, 1);
        }
    };

    GenEle9 testGen9(9);

}