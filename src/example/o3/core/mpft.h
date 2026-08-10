//
// Created by tanawin on 27/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_MPFT_H
#define KATHRYN_SRC_EXAMPLE_O3_MPFT_H

#include "slotParam.h"
#include "example/o3/simulation/proberGrp.h"

namespace kathryn::o3{

    //// miss predict fix table
    struct Mpft{   ///MD MPFT
        Table _table;   ///CTRL_HWD MPFT

        Mpft(): _table(smMPFT, SPECTAG_LEN){           ///CTRL_HWD MPFT
            _table.makeColResetEvent(mpft_valid, 0);   ///CTRL_DT MPFT
            _table.makeColResetEvent(mpft_fixTag, 0);  ///CTRL_DT MPFT
            /////// debug probe
            dataStructProbGrp.mpft.init(&_table); ///DC
        }

        void onPredSuc(opr& tag1){                                   ///CTRL_HC MPFT

            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){         ///HLH MPFT
                lhs(mpft_fixTag) <<= (lhs(mpft_fixTag) & (~tag1));   ///CTRL_CL MPFT
                zif (tag1.sl(rowIdx)){ ///// matched tag             ///CTRL_CL MPFT
                    ///// clean entire row
                    lhs(mpft_valid) <<= 0;                           ///CTRL_DT MPFT
                    lhs(mpft_fixTag) <<= 0;                          ///CTRL_DT MPFT
                }
            });

        }
        void onMisPred(){   ///HLH MPFT
            _table.doGlobColAsm({mpft_valid}, 0);    ///CTRL_DT MPFT
            _table.doGlobColAsm({mpft_fixTag}, 0);   ///CTRL_DT MPFT
        }
        void onAddNew(opr& setTag1, opr& tag1,    ///CTRL_HC MPFT
                      opr& setTag2, opr& tag2){   ///CTRL_HC MPFT

            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){   ///HLH MPFT

                opr& isTag1 = tag1.sl(rowIdx);            ///CTRL_CL MPFT
                opr& isTag2 = tag2.sl(rowIdx);            ///CTRL_CL MPFT
                Reg& vl     = lhs(mpft_valid);            ///CTRL_DT MPFT
                Reg& ft     = lhs(mpft_fixTag);           ///CTRL_DT MPFT
                ////// set the new row
                zif (isTag1 && setTag1){                  ///CTRL_CL MPFT
                    vl <<= 1;                             ///CTRL_DT MPFT
                    ft <<= tag1;                          ///CTRL_DT MPFT
                    zif (setTag2) ft <<= (tag1 | tag2);   ///CTRL_CL MPFT
                }
                zif (isTag2 && setTag2){   ///CTRL_CL MPFT
                    vl <<= 1;              ///CTRL_DT MPFT
                    ft <<= tag2;           ///CTRL_DT MPFT
                }
                ///// it is suppose to be other tag because vl for both is not set
                zif(vl){                                                 ///CTRL_CL MPFT
                    zif(setTag1) ft <<= (ft | tag1);                     ///CTRL_CL MPFT
                    zif(setTag2) ft <<= (ft | tag2);                     ///CTRL_CL MPFT
                    zif(setTag1 && setTag2) ft <<= (ft | tag1 | tag2);   ///CTRL_CL MPFT
                }
            });
        }

        opr& getFixTag(OH searchTag){                        ///CTRL_HC MPFT
            return _table[OH(searchTag)](mpft_fixTag).v();   ///CTRL_HC MPFT
        }

        opr& isUsed(int specIdx){return _table(specIdx)(mpft_valid);}   ///CTRL_HC MPFT


    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_MPFT_H
