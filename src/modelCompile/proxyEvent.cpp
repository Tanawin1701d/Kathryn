//
// Created by tanawin on 1/6/2024.
//
#include <utility>
#include "proxyEvent.h"
#include "params/simParam.h"


namespace kathryn{

    void ProxySimEvent::startRegisterCallBack()  {std::cout <<"startRegisterCallBack\n";}
    void ProxySimEvent::startVolatileEleSim()    {std::cout <<"startVolatileEleSim\n";}
    void ProxySimEvent::startNonVolatileEleSim() {std::cout <<"startNonVolatileEleSim\n";}
    void ProxySimEvent::startVcdDecVarUser()     {std::cout <<"startVcdDecVarUser\n";}
    void ProxySimEvent::startVcdDecVarInternal() {std::cout <<"startVcdDecVarInternal\n";}
    void ProxySimEvent::startVcdColUser()        {std::cout <<"startVcdColUser\n";}
    void ProxySimEvent::startVcdColInternal()    {std::cout <<"startVcdColInternal\n";}
    void ProxySimEvent::startPerfCol()           {std::cout <<"startPerfCol\n";}


    extern "C" {
        ProxySimEventBase*  simEngineCreator(){
            std::cout << "creating sim engine\n";
            return new ProxySimEvent();
        }
    }

}




