//
// Created by tanawin on 30/1/2025.
//

#ifndef KATHRYN_LIB_HW_POPPER_POPTUNNEL_POPTUNNELBASE_H
#define KATHRYN_LIB_HW_POPPER_POPTUNNEL_POPTUNNELBASE_H

#include<string>
#include <utility>

#include "lib/hw/slot/slot.h"
#include "util/termColor/termColor.h"


namespace kathryn{

    enum POP_TUNNEL_TYPE{
        PTT_HAND_SHAKE = 0, ////// requestee must ack requestor
        PTT_BROAD_CAST = 1,
        PTT_CNT        = 2////// requestor send and must receive immediately

    };


    struct PopTunnelBase{
        POP_TUNNEL_TYPE _tunnelType;
        std::string     _intfName;
        int             _amtMasterSendSrc = 0;
        int             _amtSlaveRecvSrc  = 0;

        int             _amtMasterRecvSrc = 0;
        int             _amtSlaveSendSrc  = 0;

        explicit PopTunnelBase(POP_TUNNEL_TYPE tunnelType, std::string itfName):
        _tunnelType(tunnelType),
        _intfName(std::move(itfName)){}

        virtual ~PopTunnelBase() = default;

        ///// buildLogic used to  build communication
        ///// infrastructure logic
        virtual void buildLogic(){}

        ////// src/des tracker
        void incMasterSendSrc() {_amtMasterSendSrc++;}
        void incSlaveRecvSrc () {_amtSlaveRecvSrc++ ;}

        void incMasterRecvSrc() {_amtMasterRecvSrc++;}
        void incSlaveSendSrc()  {_amtSlaveSendSrc++ ;}

        /** getter*/
        [[nodiscard]] int getMasterSendSrc() const {return _amtMasterSendSrc;}
        [[nodiscard]] int getSlaveRecvSrc () const {return _amtSlaveRecvSrc ;}

        [[nodiscard]] int getMasterRecvSrc() const {return _amtMasterRecvSrc;}
        [[nodiscard]] int getSlaveSendSrc()  const {return _amtSlaveSendSrc ;}

        [[nodiscard]] POP_TUNNEL_TYPE getTunnelType() const {return _tunnelType;}

        void transferBase(const Slot&         sendData,    ///// the data needed to send
                          WireSlot&           transferIt,  ///// destination intermediate transfer
                          Wire&               transferSig, ///// the signal indicated to send
                          std::vector<Wire*>& triggerStorage, ////// the storage of trigger signal(s)
                          Operable*           activeSignal){ ////// the active value if the trigger
            /** build trigger state*/
            Wire& trigger = mOprWire("dataTrigger", 1);
            triggerStorage.push_back(&trigger);
            trigger = 1;
            /** make the intermediate transfer slot triggered in when trigger is on*/
            transferIt.assignCore(sendData, trigger);
            transferSig.addUpdateMeta(new UpdateEvent({
                                                        &trigger,
                                                        nullptr,
                                                        activeSignal,
                                                        {0, 1},
                                                        DEFAULT_UE_PRI_USER
                                                     }));
        }

        virtual bool integrityCheck() {return true;}

    };

}

#endif //KATHRYN_LIB_HW_POPPER_POPTUNNEL_POPTUNNELBASE_H
