//
// Created by tanawin on 1/6/2024.
//

#ifndef MODEL_COMPILE_PROXY_EVENT_H
#define MODEL_COMPILE_PROXY_EVENT_H

#include "proxyEventBase.h"

namespace kathryn{


    class ProxySimEvent: public ProxySimEventBase{
    public:
        ////// sim proxy
        void startRegisterCallBack()  override;
        void startVolatileEleSim()    override;
        void startNonVolatileEleSim() override;
        ///// vcd collector
        void startVcdDecVarUser()    override;
        void startVcdDecVarInternal()override;
        void startVcdColUser()       override;
        void startVcdColInternal()   override;
        ///// start flowblock CollectData
        void startPerfCol()          override;

    };



}

#endif //MODEL_COMPILE_PROXY_EVENT_H
