//
// Created by tanawin on 2/12/2025.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod26: public Module{
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
        // mOut(g, 8);
        // mOut(h, 8);
        explicit testGenMod26(int x): Module(){
            a.asOutputGlob("a");
            b.asOutputGlob("b");
            c.asOutputGlob("c");
            d.asOutputGlob("d");

        }

        void flow() override{
            //////// connect output wire
            a.makeResetEvent(2);

            zif (a < 16){
                a <<= a + 3;
                a <<= a + 2;
                zif (a < 8){
                    a <<= a + 1;
                }
            }zelif(a < 32){
                zif (a < 24){
                    a <<= 24;
                }zelse{
                    a <<= 48;
                }
            }


        }

    };

    class GenEle26: public GenEle{
    public:
        explicit GenEle26(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod26, 1);
        }
    };

    GenEle26 testGen26(26);

}