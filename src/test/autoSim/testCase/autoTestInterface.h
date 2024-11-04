//
// Created by tanawin on 7/2/2567.
//

#ifndef KATHRYN_AUTOTESTINTERFACE_H
#define KATHRYN_AUTOTESTINTERFACE_H

#include <sim/modelSimEngine/base/proxyBuildMng.h>

#include "string"


namespace kathryn{


    class AutoTestEle{
    protected:
        int _simId = -1;
    public:
        explicit AutoTestEle(int simId);
        virtual void start(std::string prefix, SimProxyBuildMode simProxyBuildMode) = 0;
        int getSimId() const {return _simId;}

    };



}

#endif //KATHRYN_AUTOTESTINTERFACE_H
