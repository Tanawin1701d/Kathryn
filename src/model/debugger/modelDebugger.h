//
// Created by tanawin on 12/1/2567.
//

#ifndef KATHRYN_MODELDEBUGGER_H
#define KATHRYN_MODELDEBUGGER_H

#include <string>
#include "util/logger/logger.h"

namespace kathryn{


    class ModelDebuggable{

    public:
        virtual std::string getMdDescribe(){
            return "[this component has no description]";
        };
        virtual std::string getMdIdentVal() = 0;

        virtual void addMdLog(MdLogVal* mdLogVal);

    };

}

#endif //KATHRYN_MODELDEBUGGER_H
