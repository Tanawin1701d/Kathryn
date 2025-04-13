//
// Created by tanawin on 7/4/2025.
//

#ifndef KATHRYN_SRC_KRIDE_RSV_I_RSVBASE_H
#define KATHRYN_SRC_KRIDE_RSV_I_RSVBASE_H

#include"kride/incl.h"
#include"rsvBase.h"

namespace kathryn{

    struct IRSV_BASE: RsvBase{
        Table entries;
        mWire(allocPtr, entries._identWidth);

        IRSV_BASE(int rsvId, const std::string& rm,
                  RowMeta& userMeta, int addrLength,
                  D_ALL& din, D_IO_RSV& dcIn):
        RsvBase(rsvId, rm, din, dcIn),
        entries(rm, RsvEntryMeta + userMeta, addrLength){}


        void buildIssPtr(){
            /////// we don't care the mispredict deletation
            /// because this station will stall the system
            /// the valid will be set from buildCirSearchLogic
            dc.issueBuf = entries.buildCirSearchLogic(entries._meta, false, 0, false, &allocPtr,
            scmp{return (lhsSlot.at("busy") == 0) && (rhsSlot.at("busy") == 1);}
            );
        }

        void updateAllocPtrMisP(){
            Candidate allocCan = entries.buildCirSearchLogic(entries._meta, true, 0, false, &allocPtr,
                scmp{
                    return ((lhsSlot.at("busy") == 1) & (~d.shouldInv(lhsSlot.at("specTag"))))
                    &&     ((rhsSlot.at("busy") == 0) & (~d.shouldInv(rhsSlot.at("specTag"))));
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
            offer(rsvName){
                ofcc(EXEC, d.exb.misPred ){updateAllocPtrMisP();}
                ofcc(DP  , dc.allocatable){ allocPtr <<= allocPtr + dc.req;}
            }
        }

        void flow() override{
            buildIssPtr();
        }

        Operable& buildAkb() override{ //// do something with it

        }
    };
}


#endif //KATHRYN_SRC_KRIDE_RSV_I_RSVBASE_H
