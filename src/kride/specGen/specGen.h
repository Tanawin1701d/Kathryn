//
// Created by tanawin on 4/4/2025.
//

#ifndef KATHRYN_SRC_KRIDE_SPECGEN_SPECGEN_H
#define KATHRYN_SRC_KRIDE_SPECGEN_SPECGEN_H

#include"kride/incl.h"

namespace kathryn{

    struct SpecGen: public Module{

        D_ALL d;
        mReg(curSpec, SPEC_TAG_LEN);
        mReg(brDepth, SPEC_TAG_SEL);
        expression newSpec[2];
        mWire(specRes1, SPEC_TAG_LEN);
        mWire(specRes2, SPEC_TAG_LEN);

        explicit SpecGen(D_ALL& din):
        d(din), newSpec{expression(1), expression(1)}{}

        void flow(){

            newSpec[0] = g(curSpec(0, SPEC_TAG_LEN-1), curSpec(SPEC_TAG_LEN-1));
            newSpec[1] = g(newSpec[0](0, SPEC_TAG_LEN-1), newSpec[0](SPEC_TAG_LEN-1));

            offer(SPEC_GEN){
                ofcc(EXEC, d.exb.misPred){
                    curSpec <<= d.exb.specTagRcv;
                    brDepth <<= 0;
                }
                ofcc(DEC, brDepth){
                    brDepth <<= brDepth + d.dcd.isBranches[0].extB(SPEC_TAG_SEL) +
                                          d.dcd.isBranches[1].extB(SPEC_TAG_SEL);
                        ////////// make it as array or table
                        zif(d.dcd.isBranches[0] & d.dcd.isBranches[1]){
                            specRes1 = newSpec[0];
                            specRes2 = newSpec[1];

                        }zelif(d.dcd.isBranches[0]){
                            specRes1 = newSpec[0];
                            specRes2 = curSpec;
                        }zelif(d.dcd.isBranches[1]){
                            specRes1 = curSpec;
                            specRes2 = newSpec[0];
                        }
                        zelse{
                            specRes1 = curSpec;
                            specRes2 = curSpec;
                        };

                        curSpec <<= specRes2;
                }

            }
        }

        Operable& allocable(){
            return (brDepth + d.dcd.isBranches[0].extB(SPEC_TAG_SEL) + d.dcd.isBranches[1].extB(SPEC_TAG_SEL))
            <= (d.exb.sucPred.extB(SPEC_TAG_SEL) + BRANCH_ENT_NUM);
        }

    };

}

#endif //KATHRYN_SRC_KRIDE_SPECGEN_SPECGEN_H
