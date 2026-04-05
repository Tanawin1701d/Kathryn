//
// Created by tanawin on 4/7/2024.
//
#include "kathryn.h"
#include "genEle.h"

namespace kathryn{

    class testGenMod5: public Module{
    public:
        mVal(con, 8, 48);
        mReg(a, 8);
        mReg(b, 8);
        mReg(c, 8);
        explicit testGenMod5(int x): Module(){
            a.asOutputGlob("a");
            b.asOutputGlob("b");
            c.asOutputGlob("c");
        }

        void flow() override{

            seq{
                a <<= con;
                b <<= a;
                c <<= b;
            }
        }

    };

    class GenEle5: public GenEle{
    public:
        explicit GenEle5(int id): GenEle(id){};

        void start(PARAM& param) override{
            mMod(m3, testGenMod5, 1);
        }
    };

    GenEle5 testGen5(5);

}