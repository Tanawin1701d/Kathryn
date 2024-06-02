//
// Created by tanawin on 1/6/2024.
//

#ifndef PROXYEVENT_H
#define PROXYEVENT_H

#include <unordered_map>
#include <params/simParam.h>

#include "sim/logicRep/valRep.h"
#include "sim/event/eventBase.h"
#include "sim/simResWriter/simResWriter.h"

namespace kathryn{


    class ProxySimEvent: public EventBase{
    protected:
        const std::string VCD_WRITE_DES;
        std::unordered_map<ull, ValRepBase*>  callBack;
        VcdWriter* vcdWriter;
        /////// for flow collection we will let flow model handle this instead
        /// we just collect only data that flow block use



    public:
        ////// constructor
        ProxySimEvent(std::string vcdWriteDes);
        ~ProxySimEvent();
        ////// sim base
        void simStartCurCycle()    override;
        void curCycleCollectData() override;
        void simStartNextCycle()   override;
        void simExitCurCycle()     override{}
        bool needToDelete()        override{return false;}
        ////// sim proxy
        void startRegisterCallBack();
        void startVolatileEleSim();
        void startNonVolatileEleSim();
        ///// vcd collector
        void startVcdDecVarUser();
        void startVcdDecVarInternal();
        void startVcdColUser();
        void startVcdColInternal();
        ///// start flowblock CollectData
        void startPerfCol();




    };



}

#endif //PROXYEVENT_H
