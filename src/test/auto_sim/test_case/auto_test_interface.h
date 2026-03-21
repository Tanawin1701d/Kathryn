//
// Created by tanawin on 7/2/2567.
//

#ifndef KATHRYN_AUTOTESTINTERFACE_H
#define KATHRYN_AUTOTESTINTERFACE_H

#include "sim/model_sim_engine/base/proxy_build_mng.h"

#include "string"


namespace kathryn{


    class AutoTestEle{
    protected:
        int _simId = -1;
    public:
        explicit AutoTestEle(int sim_id);
        virtual void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) = 0;
        int get_sim_id() const {return _simId;}

    };



}

#endif //KATHRYN_AUTOTESTINTERFACE_H
