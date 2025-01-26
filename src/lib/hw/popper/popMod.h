//
// Created by tanawin on 26/1/2025.
//

#ifndef src_lib_hw_popper_POPMOD_H
#define src_lib_hw_popper_POPMOD_H
#include "popInterrupt.h"
#include "popTunnelMng.h"
#include "model/hwComponent/module/module.h"

namespace kathryn{


    struct PopMod: Module, PopIntrable, PopTunnelable{

        explicit PopMod(PopIntrMng* intrMng, PopTunnelMng* tunnelMng):
            PopIntrable(intrMng), PopTunnelable(tunnelMng){}


        /**send and recv from tunneling*/

        std::pair<Slot&, Operable&> tunnelSendAndRcv(PopTunnel& sendTunnel, Slot& sendData, PopTunnel& recvTunnel){
            par{
                cdowhile(~sendTunnel.isAck()){
                    sendTunnel.send(sendData);
                }
                cdowhile(~recvTunnel.isReq()){
                    recvTunnel.ack();
                }
            }
            return {recvTunnel.recv(), recvTunnel.isAck()};
        }

        std::pair<Slot&, Operable&> tunnelSendAndRcv(const std::string& sendTunnelName,
                                                     Slot& sendData,
                                                     const std::string& recvTunnelName){
            return tunnelSendAndRcv(getPopTunnel(sendTunnelName),
                                    sendData,
                                    getPopTunnel(recvTunnelName));
        }


    };

}

#endif //src_lib_hw_popper_POPMOD_H
