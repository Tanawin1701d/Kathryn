//
// Created by tanawin on 27/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_MPFT_H
#define KATHRYN_SRC_EXAMPLE_O3_MPFT_H

#include "slotParam.h"
#include "example/o3/simulation/proberGrp.h"  ///DC

namespace kathryn::o3{

    //// miss predict fix table
    struct Mpft{
        Table _table;

        Mpft(): _table(smMPFT, SPECTAG_LEN){
            _table.makeColResetEvent(mpft_valid, 0);
            _table.makeColResetEvent(mpft_fixTag, 0);
            /////// debug probe
            dataStructProbGrp.mpft.init(&_table); ///DC
        }

        void onPredSuc(opr& tag1){

            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){
                lhs(mpft_fixTag) <<= (lhs(mpft_fixTag) & (~tag1));
                zif (tag1.sl(rowIdx)){ ///// matched tag
                    ///// clean entire row
                    lhs(mpft_valid) <<= 0;
                    lhs(mpft_fixTag) <<= 0;
                }
            });

        }
        void onMisPred(){
            _table.doGlobColAsm({mpft_valid}, 0);
            _table.doGlobColAsm({mpft_fixTag}, 0);
        }
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
                zif(vl){
                    zif(setTag1) ft <<= (ft | tag1);
                    zif(setTag2) ft <<= (ft | tag2);
                    zif(setTag1 && setTag2) ft <<= (ft | tag1 | tag2);
                }
            });
        }

        opr& getFixTag(OH searchTag){
            return _table[OH(searchTag)](mpft_fixTag).v();
        }

        opr& isUsed(int specIdx){return _table(specIdx)(mpft_valid);}


    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_MPFT_H
