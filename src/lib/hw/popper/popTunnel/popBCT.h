//
// Created by tanawin on 1/2/2025.
//

#ifndef src_lib_hw_popper_popTunnel_POPBCT_H
#define src_lib_hw_popper_popTunnel_POPBCT_H

#include "popTunnelBase.h"

namespace kathryn{

    //////// POP_BROADCAST_TUNNEL ////// it will broadcast only one cycle
    struct PopBCT: PopTunnelBase{

        WireSlot bcInterSlot;
        mWire(active, 1);
        mVal(VALID  , 1, 1);

        std::vector<Wire*> sendStates;

        explicit PopBCT(std::string itfName, const RowMeta& sendRowMeta):
            PopTunnelBase(PTT_BROAD_CAST,std::move(itfName)),
            bcInterSlot(sendRowMeta, 0){}

        void reqBc(Slot& bcData){
            transferBase(bcData, bcInterSlot, active, sendStates, &VALID);
            incMasterSendSrc();
        }
        Operable& isBc (){ return active;     }
        void      reqBc(){ incSlaveRecvSrc();} //// it only declares that there is person listen to this tunnnel
        WireSlot& getBc(){ return bcInterSlot;}

        bool integrityCheck() override{
            if (getMasterRecvSrc() != 0 || getSlaveSendSrc() != 0){
                std::cout << TC_RED << "[ERROR] " << _intfName << "  : must have no slave data transfer" << TC_DEF << std::endl;
                return false;
            }

            if (getMasterSendSrc() == 0){
                std::cout << TC_YELLOW << "[WARN] " << _intfName << "  : has no broadCaster" << TC_DEF << std::endl;
                return false;
            }

            if (getSlaveRecvSrc() == 0){
                std::cout << TC_YELLOW << "[WARN] " << _intfName << "  : has no broadCast listenner" << TC_DEF << std::endl;
                return false;
            }
            return true;
        }

        void blk_bcUntil(Slot& bcData, Operable& cond){
            cdowhile(!cond){
                reqBc(bcData);
            }
        }

    };

}

#endif //src_lib_hw_popper_popTunnel_POPBCT_H
