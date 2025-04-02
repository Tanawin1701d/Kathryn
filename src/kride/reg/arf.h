//
// Created by tanawin on 27/3/2025.
//

#ifndef KATHRYN_SRC_KRIDE_REG_ARF_H
#define KATHRYN_SRC_KRIDE_REG_ARF_H

#include "kride/incl.h"
#include "vector"

namespace kathryn{

    ///// the last one is the master

    Operable& isCommiting  (int specIdx, int archIdx);
    Operable& isNewDep     (int specIdx, int archIdx);
    Operable& newDepAddr   (int specIdx, int archIdx);
    Operable& isSpecTagAct (int specTag); //// the


    struct Rt: public Module{

        D_ALL& d;
        ////// we will migrate to using offer
        mWire(robCommit0, 1);
        mWire(robCommit1, 1);

        std::vector<Table> rtTables;

        explicit Rt(D_ALL& din):
            rtTables(SPEC_TAG_LEN, Table("rtTable",
                                        {C_VALID, C_SPEC_PREF},
                                        {1      , RRF_SEL    },
                                        AMT_AREG)),
            d(din){}

        void flow() override{
            ////////// first priority is branch exec unit
            /// when it prediction success or misprediction


            /////// case misprediction
            /**
             *  mispredict -----> all table(spectag) and all arch idx must be recovery
             *  success    -----> the predict matched table will be free and replace with the latest renaming
             *  (from master table and commiting instruction)
             *  commiting  -----> the table will update all busy status of the all cell that msb is not activate
             *
             *
             * */
                                            ////// include the master
            for (int specIdx = 0; specIdx <= SPEC_TAG_LEN; specIdx++){
                for (int archIdx = 0; archIdx < AMT_AREG; archIdx++){
                    Table&   rtTable = rtTables[specIdx];
                    Reg&     valid  = rtTable.get(archIdx).get(C_VALID);
                    Reg&     phyReg = rtTable.get(archIdx).get(C_SPEC_PREF);
                    ///////// if mispredict
                    zif (d.exb.misPred){
                        for (int checkRcvId = 0; checkRcvId < SPEC_TAG_LEN; checkRcvId++){
                            zif(d.exb.specTagRcv == checkRcvId){
                                Table& rcvRtTable = rtTables[checkRcvId];
                                valid   = rcvRtTable.get(archIdx).get(C_VALID);
                                phyReg  = rcvRtTable.get(archIdx).get(C_SPEC_PREF);
                            }
                        }
                    }zelif(d.exb.sucPred){ ////// in success there is NO NEW INSTRUCTION TO RENAME
                        ////////// now we are free bring the master to the success but we must obey
                        ///////// reorder buffer
                        zif(d.exb.specTagRcv == specIdx){
                            /////// obey the rob if it want to unser the busy
                            zif( (robCommit0 && (d.com.commitAddr0 == archIdx)) ||
                                 (robCommit1 && (d.com.commitAddr1 == archIdx)))
                                valid <<= 0;
                            zelse {
                                valid  <<= rtTables[SPEC_TAG_LEN].get(archIdx).get(C_VALID);
                                phyReg <<= rtTables[SPEC_TAG_LEN].get(archIdx).get(C_SPEC_PREF);
                            }
                        }
                    }zelse{
                        zif(isNewDep(specIdx, archIdx)){
                            valid  <<= 1;
                            phyReg <<= newDepAddr(specIdx, archIdx);
                        }zelif(isCommiting(specIdx, archIdx)){
                            valid <<= 0;
                        }
                    };


                }
            }
        }

    };

}

#endif //KATHRYN_SRC_KRIDE_REG_ARF_H
