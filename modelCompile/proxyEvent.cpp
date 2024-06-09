//
// Created by tanawin on 1/6/2024.
//
#include <iostream>
#include <utility>
#include "proxyEvent.h"
#include "params/simParam.h"


namespace kathryn{

    void ProxySimEvent::startRegisterCallBack() {std::cout <<"startRegisterCallBack" << std::endl;}
    void ProxySimEvent::startVolatileEleSim()   {std::cout <<"startVolatileEleSim"   << std::endl;}
    void ProxySimEvent::startNonVolatileEleSim(){std::cout <<"startNonVolatileEleSim"<< std::endl;}
    void ProxySimEvent::startVcdDecVarUser()    {std::cout <<"startVcdDecVarUser"    << std::endl;}
    void ProxySimEvent::startVcdDecVarInternal(){std::cout <<"startVcdDecVarInternal"<< std::endl;}
    void ProxySimEvent::startVcdColUser()       {std::cout <<"startVcdColUser"       << std::endl;}
    void ProxySimEvent::startVcdColInternal()   {std::cout <<"startVcdColInternal"   << std::endl;}
    void ProxySimEvent::startPerfCol()          {std::cout <<"startPerfCol"          << std::endl;}

    extern "C" ProxySimEventBase* create() {
        std::cout << "creating proxy simEvent in dynamic object" << std::endl;
        return new ProxySimEvent();
    }


}



//extern "C" void  create(){
//        std::cout << "creating sim engine" << std::endl;
//}






