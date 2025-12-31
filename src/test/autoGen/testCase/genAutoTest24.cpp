//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod24: public Module{
    public:
        mVal (hf1, 8, 36);
        mVal (hf2, 8, 12);
        mReg (a, 8);
        mReg (b, 8);
        mReg (c, 8);
        mWire(d, 8);
        ////////test io
        //mIn (e, 8);
        //mIn (f, 8);
        mOut(g, 8);
        mOut(h, 8);
        explicit testGenMod24(int x): Module(){
            a.asOutputGlob("a");
            b.asOutputGlob("b");
            c.asOutputGlob("c");
            d.asOutputGlob("d");

        }

        void flow() override{
            //////// connect output wire
            g = a;
            h = b;
            ////////// we use d as a interrupt
            seq{
                par{
                    a = 0;
                    b = 0;
                    c = 0;
                    d = 0;
                }

                 a <<= a+1;
                 b <<= b+2;
                 c <<= c+3;
                d    = 4;

            }


        }

    };

    class GenEle24: public GenEle{
    public:
        explicit GenEle24(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod24, 1);
        }
    };

    GenEle24 testGen24(24);

}