//
// Created by tanawin on 1/6/2024.
//

#ifndef MODEL_COMPILE_PROXY_EVENT_H
#define MODEL_COMPILE_PROXY_EVENT_H

#include "sim/modelSimEngine/base/proxyEventBase.h"

namespace kathryn{


    class ProxySimEvent: public ProxySimEventBase{
    public:
        void intCodeWarmUp()          override;
        ////// sim proxy
        void startRegisterCallBack()  override;
        // void startMainOpEleSim()      override;
        ///// volatile and non volatile (paritial)
        void startMainOpEleSimNeg()   override;
        void startMainOpEleSimPos()   override;
        ///// non-volatile
        void startFinalizeEleSimPos()   override;
        void startFinalizeEleSimNeg()   override;
        ///// vcd collector
        void startVcdDecVarUser()     override;
        void startVcdDecVarInternal() override;
        void startVcdColUser()        override;
        void startVcdColInternal()    override;
        ///// start flowblock CollectData
        void startPerfCol()           override;
        CYCLE mainSim() override;

        ////// set callback system
        int getCallBackNo(int idx) const override;
        int getCallBackAmt() const override;



    };



}

#endif //MODEL_COMPILE_PROXY_EVENT_H
