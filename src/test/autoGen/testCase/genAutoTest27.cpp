//
// Created by tanawin on 2/12/2025.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod27: public Module{
    public:
        mVal (hf1, 8, 36);
        mVal (hf2, 8, 12);
        mReg (a, 8);
        mReg (b, 8);
        mReg (c, 8);
        mWire(d, 8);
        mReg(switchVal, 3);
        mReg(subCheck, 1);
        ////////test io
        //mIn (e, 8);
        //mIn (f, 8);
        // mOut(g, 8);
        // mOut(h, 8);
        explicit testGenMod27(int x): Module(){
            a.asOutputGlob("a");
            b.asOutputGlob("b");
            c.asOutputGlob("c");
            d.asOutputGlob("d");

        }

        void flow() override{
            //////// connect output wire
            a.makeResetEvent(2);

            seq{
                par{
                    switchVal <<= 4;
                    subCheck  <<= 1;
                }

                par{
                    ztate(switchVal){
                        zcase(0b100){
                            a <<=  9;
                            b <<= 24;
                            zif(subCheck){
                                b <<= 48;
                            }
                        }
                        zcase(0b001){
                            a <<= 10;
                            b <<= 107;
                            b <<= 108;
                        }
                        zcasedef{
                            b <<= 404;
                        }
                    }
                }
            }


        }

    };

    class GenEle27: public GenEle{
    public:
        explicit GenEle27(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod27, 1);
        }
    };

    GenEle27 testGen27(27);

}