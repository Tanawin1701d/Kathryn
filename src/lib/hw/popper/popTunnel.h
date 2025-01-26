//
// Created by tanawin on 26/1/2025.
//

#ifndef src_lib_hw_popper_POPTUNNEL_H
#define src_lib_hw_popper_POPTUNNEL_H

#include "lib/hw/slot/slot.h"

#include "lib/hw/slot/slotProxy.h"

namespace kathryn{



    struct PopTunnel{

        WireSlot transferData;
        mWire(reqSig, 1);
        mWire(ackSig, 1);
        mVal(VALID, 1,1);

        std::vector<Wire*> sendStates;

        int amtSendSrc = 0;
        int amtAckSrc  = 0;


        explicit PopTunnel(const RowMeta& rowMeta):
        transferData(rowMeta, 0){}

        /**
         * sender
         */
        void send(Slot& sendData){

            Wire& trigger = mOprWire("dataTrigger", 1);
            sendStates.push_back(&trigger);
            /** the only CCE */
            trigger = 1;
            /*****************/
            /** make the imm slot get assign*/
            RowMeta   rowMeta    = sendData.getMeta();
            FieldMeta firstField = rowMeta.getField(0);
            FieldMeta lastField  = rowMeta.getField(rowMeta.getSize()-1);
            transferData.assignCore(sendData,
                                firstField._fieldName,
                                lastField._fieldName,
                                trigger);
            /** make the reqSig setup*/
            reqSig.addUpdateMeta(new UpdateEvent({
                    &trigger,
                    nullptr,
                    &VALID,
                    VALID.getOperableSlice(),
                    DEFAULT_UE_PRI_USER
                    }));
            amtSendSrc++;

        }
        Operable& isAck(){ return ackSig;}
        /***
         * retriever
         */
        WireSlot& recv() { return transferData;}
        Operable& isReq(){ return reqSig; }
        void      ack  (){ ackSig = 1; amtAckSrc++; }
        /** get meta data*/
        [[nodiscard]] int getAmtSendSrc() const{return amtSendSrc;}
        [[nodiscard]] int getAmtAckSrc()  const{return amtAckSrc; }

    };
}

#endif //src_lib_hw_popper_POPTUNNEL_H
