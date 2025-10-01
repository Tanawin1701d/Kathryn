//
// Created by tanawin on 27/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_MPFT_H
#define KATHRYN_SRC_EXAMPLE_O3_MPFT_H

#include "kathryn.h"
#include "stageStruct.h"

namespace kathryn::o3{

    //// miss predict fix table
    struct Mpft{
        Table _table;

        Mpft(): _table(smMPFT, SPECTAG_LEN){}

        void onPredSuc(opr& tag1){

            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){
                zif (tag1.sl(rowIdx)){ ///// matched tag
                    ///// clean entire row
                    lhs(mpft_valid) <<= 0;
                    lhs(mpft_fixTag) <<= 0;
                }zelif(lhs(mpft_valid)){
                    ///// fix other tag
                    lhs(mpft_fixTag) <<=
                        (lhs(mpft_fixTag) | (~tag1));
                }
            });

        }
        void onMissPred(){ _table.doReset();}
        void onAddNew(opr& setTag1, opr& tag1,
                      opr& setTag2, opr& tag2){

            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){

                opr& isTag1 = tag1.sl(rowIdx);
                opr& isTag2 = tag2.sl(rowIdx);
                Reg& vl     = lhs(mpft_valid);
                Reg& ft     = lhs(mpft_fixTag);
                ////// set the new row
                zif (isTag1 && setTag1){
                    vl <<= 1;
                    ft <<= tag1;
                    zif (setTag2) ft <<= (tag1 | tag2);
                }
                zif (isTag2 && setTag2){
                    vl <<= 1;
                    ft <<= tag2;
                }
                ///// it is suppose to be other tag because vl for both is not set
                Operable* newTag = &lhs(mpft_fixTag);
                zif(vl){
                    zif(setTag1 && setTag2) ft <<= (ft | tag1 | tag2);
                    zif(setTag1) ft <<= (ft | tag1);
                    zif(setTag2) ft <<= (ft | tag2);
                }
            });
        }


    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_MPFT_H
