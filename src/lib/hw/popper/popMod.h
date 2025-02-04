//
// Created by tanawin on 26/1/2025.
//

#ifndef src_lib_hw_popper_POPMOD_H
#define src_lib_hw_popper_POPMOD_H
#include "popInterrupt.h"
#include "popTunnel/popHST.h"
#include "popTunnel/popBCT.h"
#include "popTunnel/popTunnelMng.h"
#include "model/hwComponent/module/module.h"

namespace kathryn{


    struct PopMod: Module, PopIntrable, PopTunnelable{

        explicit PopMod(PopIntrMng* intrMng, PopTunnelMng* tunnelMng):
            PopIntrable(intrMng), PopTunnelable(tunnelMng){}


        /**req and wait4Ret ///// return {the return data slot, enable signal}*/
        std::pair<Slot&, Operable&> hstReqAndWaitRet(PopHST& hstTunnel, Slot& sendData){
            par{
                cdowhile(~hstTunnel.isAckSend()){
                    hstTunnel.reqSend(sendData);
                }
                cdowhile(~hstTunnel.isReqRet()){ markJoinMaster
                    hstTunnel.ackRet();
                }
            }
            return {hstTunnel.getReqRet(), hstTunnel.isAckRet()};
        }

        std::pair<Slot&, Operable&> hstReqAndWaitRet(const std::string& tunnelName,
                                                     Slot& sendData){
            return hstReqAndWaitRet(getHST(tunnelName),sendData);
        }

        /** wait4req and ret ///// return {the return data slot, enable signal}*/

        std::pair<Slot&, Operable&> hstWaitReqAndRet(PopHST& hstTunnel, Slot& retData){
            par{
                cdowhile(~hstTunnel.isReqSend()){
                    hstTunnel.ackSend();
                }
                cdowhile(~hstTunnel.isAckRet()){ markJoinMaster
                    hstTunnel.reqRet(retData);
                }
            }
            return {hstTunnel.getReqSend(), hstTunnel.isAckRet()};
        }

        std::pair<Slot&, Operable&> hstWaitReqAndRet(const std::string& tunnelName,
                                                     Slot& retDataData){
            return hstWaitReqAndRet(getHST(tunnelName), retDataData);
        }


    };

}

#endif //src_lib_hw_popper_POPMOD_H
