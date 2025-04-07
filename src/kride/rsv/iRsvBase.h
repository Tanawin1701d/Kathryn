//
// Created by tanawin on 7/4/2025.
//

#ifndef KATHRYN_SRC_KRIDE_RSV_I_RSVBASE_H
#define KATHRYN_SRC_KRIDE_RSV_I_RSVBASE_H

#include"kride/incl.h"

namespace kathryn{

    struct IRSV_BASE: Module{
        D_IO_RSV dc;
        D_ALL& d;
        RowMeta& userMeta;
        Table entries;
        mWire(allocPtr, entries._identWidth);

        IRSV_BASE(D_ALL& din,
                  const std::string& tableName,
                  RowMeta& userMeta, int addrLength):
        d(din), userMeta(userMeta),
        entries(tableName, IORsvEntry + userMeta, addrLength){}


        void buildIssPtr(){
            /////// we don't care the mispredict deletation
            /// because this station will stall the system
            /// the valid will be set from buildCirSearchLogic
            dc.issueCand = entries.buildCirSearchLogic(IORsvEntry + userMeta, false, 0, false, &allocPtr,
            scmp{return (lhsSlot.at("busy") == 0) && (rhsSlot.at("busy") == 1);}
            );
        }

        void updateAllocPtrMisP(){
            Candidate allocCan = entries.buildCirSearchLogic(IORsvEntry + userMeta, true, 0, false, &allocPtr,
                scmp{
                    return ((lhsSlot.at("busy") == 1) & (~d.exb.shouldInv(lhsSlot.at("specTag"))))
                    &&     ((lhsSlot.at("busy") == 0) & (~d.exb.shouldInv(lhsSlot.at("specTag"))));
                });
         }

        void allocatable(){
            auto& allocable1=  ~entries.buildGetLogic(allocPtr).at("busy");
            auto& allocable2=  ~entries.buildGetLogic(allocPtr + 1).at("busy");
            zif (dc.req == 0) {dc.allocatable = 1;}
            zelif(dc.req == 1){dc.allocatable = allocable1;}
            zelif(dc.req == 2){dc.allocatable = allocable1 & allocable2;}

        }

        void flowBase(){
            ////// do update alloc pointer
            offer(RSV_CEN){
                ofcc(EXEC, d.exb.misPred ){updateAllocPtrMisP();}
                ofcc(DP  , dc.allocatable){ allocPtr <<= allocPtr + dc.req;}
            }
        }




    };


}


#endif //KATHRYN_SRC_KRIDE_RSV_I_RSVBASE_H
