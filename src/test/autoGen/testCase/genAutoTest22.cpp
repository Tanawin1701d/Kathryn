//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod22: public Module{
    public:
        mVal(hf1, 8, 36);
        mVal(hf2, 8, 12);
        mReg(a, 8);
        mWire(b, 8);
        mReg(c, 8);
        mWire(d, 1);
        explicit testGenMod22(int x): Module(){
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

                pipWrap{
                    /////// pipe block 0
                    pipBlk{
                        intrStart(d);
                        intrReset(d);

                        cif(a < 5){
                            a = a + 1;
                        }celse{
                            cwhile(true){
                                seq {
                                    b  = 1;
                                }
                            }
                        }
                    }
                    /////// pipe block 1
                    pipBlk{
                        c = c + 1;
                    }
                }

            }

            seq{
                syWait(15);
                par{
                    a = 0;
                    d = 1;
                }
            }


        }

    };

    class GenEle22: public GenEle{
    public:
        explicit GenEle22(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod22, 1);
        }
    };

    GenEle22 testGen22(22);

}