//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod21: public Module{
    public:
        mVal(hf1, 8, 36);
        mVal(hf2, 8, 12);
        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        mWire(d, 1);
        explicit testGenMod21(int x): Module(){
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

                cwhile(  c == 1 ){
                    cwhile(true){
                        intrStart(d);
                        seq{
                            cif(a == 48){
                                sbreak;
                            }celse{
                                a = a + 1;
                            }
                        }
                    }
                    b = 1;
                }


            }

            seq{
                syWait(5);
                d = 1;
            }


        }

    };

    class GenEle21: public GenEle{
    public:
        explicit GenEle21(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod21, 1);
        }
    };

    GenEle21 testGen21(21);

}