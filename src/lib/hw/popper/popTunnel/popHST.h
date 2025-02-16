//
// Created by tanawin on 26/1/2025.
//

#ifndef src_lib_hw_popper_POPTUNNEL_H
#define src_lib_hw_popper_POPTUNNEL_H



#include "popTunnelBase.h"

namespace kathryn{


    ////// POP_HAND_SHAKE_TUNNEL
    struct PopHST: PopTunnelBase{

        WireSlot sendInterSlot;
        WireSlot retInterSlot;
        mWire(reqSendSig , 1);
        mWire(ackSendSig , 1);
        mWire(reqRetSig  , 1);
        mWire(ackRetSig  , 1);
        mVal (VALID      , 1, 1);

        /**
         *  sending timming procedure
         *  requestor <-------------------------> Recipient
         *   -------> reqSend(SlotBase)
         *   w4 ackSend signal
         *   relieve                   <------- ackSend()
         *   <-------------- this is optional -------------->
         *                             <------- reqRet(Slot)
         *                             w4 ackRet signal
         *   --------> ackRet()
         * */

        std::vector<Wire*> sendStates;
        std::vector<Wire*> retStates;


        explicit PopHST(std::string itfName, const RowMeta& sendRowMeta, const RowMeta& retRowMeta):
                PopTunnelBase(PTT_HAND_SHAKE, std::move(itfName)),
                sendInterSlot(sendRowMeta, 0),
                retInterSlot (retRowMeta , 0){}




        /**
         * sending
         */
        void reqSend(const Slot& sendData){
            transferBase(sendData, sendInterSlot, reqSendSig, sendStates, &VALID);
            incMasterSendSrc();
        }
        Operable& isReqSend  () { return reqSendSig;                }
        WireSlot& getReqSend () { return sendInterSlot;             }
        void      ackSend    () { ackSendSig = 1; incSlaveRecvSrc();}
        Operable& isAckSend  () { return ackSendSig;                }
        /***
         * retrieving
         */
         void reqRet(Slot& retData){
             transferBase(retData, retInterSlot, reqRetSig, retStates, &VALID);
             incSlaveSendSrc();
         }
         Operable& isReqRet () { return reqRetSig;                 }
         WireSlot& getReqRet() {return retInterSlot;               }
         void      ackRet   () { ackRetSig = 1; incMasterRecvSrc();}
         Operable& isAckRet () { return ackRetSig;                 }

        /**
         * integrity check
         */
        bool integrityCheck() override{

            bool integrityCheck = true;

            if (getMasterSendSrc() == 0){
                std::cout << TC_YELLOW << "[WARN] " << _intfName << "  : has no master sender" << TC_DEF << std::endl;
                integrityCheck = false;
            }
            if (getSlaveRecvSrc() == 0){
                std::cout << TC_YELLOW << "[WARN] " << _intfName << "  : has no slave rcv" << TC_DEF << std::endl;
                integrityCheck = false;
            }

            if (getSlaveSendSrc() != 0){
                if (getMasterRecvSrc() == 0){
                    std::cout << TC_YELLOW << "[WARN] " << _intfName << "  : slave return but have no master recv" << TC_DEF << std::endl;
                    integrityCheck = false;
                }
            }

            return integrityCheck;

        }

        /** behavior block recruitment */

        ////// one way handshake
        void blk_reqAndWait4Ack(Slot& sendData){
            par{
                cdowhile(~isAckSend()){
                    reqSend(sendData);
                }
            }
        }

        void blk_wait4Ack(){
            par{
                cdowhile(~isReqSend()){
                    ackSend();
                }
            }
        }


        ///// for MASTER
        ///// it will send data and retrieve back data
        ///// typically it will use at least ONE CYCLE
        void blk_reqAndWait4Ret(Slot& sendData){
            par{
                cdowhile(~isAckSend()){
                    reqSend(sendData);
                }
                cdowhile(~isReqRet()){ markJoinMaster
                    ackRet();
                }
            }
        }

        //// for SLAVE
        ///// it will wait for req data and return back data
        ///// typically it will use at least ONE CYCLE

        void blk_wait4ReqAndRet(Slot& retData){
            par{
                cdowhile(~isReqSend()){
                    ackSend();
                }
                cdowhile(~isAckRet()){ markJoinMaster
                    reqRet(retData);
                }
            }
        }



    };
}

#endif //src_lib_hw_popper_POPTUNNEL_H
