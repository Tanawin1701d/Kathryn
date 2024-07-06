//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod20: public Module{
    public:
        mVal(hf1, 8, 36);
        mVal(hf2, 8, 12);
        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        mWire(d, 1);
        explicit testGenMod20(int x): Module(){
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

                par{
                    cwhile(  c == 1 ){
                        cif(a < 10){ intrStart(d); ////// it start at cif not inside cif
                            a = a+1;
                        }celse{
                            a = a+2;
                        };

                        sif(b < 7){ intrStart(d); ////// it start at cif not inside sif
                            b = b+1;
                        }selif(b < 23){
                            b = b+2;
                        }
                    }
                }

            }

            seq{
                syWait(5);
                par{ d = 1; c = 1;}
            }


        }

    };

    class GenEle20: public GenEle{
    public:
        explicit GenEle20(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod20, 1);
        }
    };

    GenEle20 testGen20(20);

}