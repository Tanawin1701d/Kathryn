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
        /** this is legacy method*/
        virtual std::string getMdDescribe(){
            return "[this component has no description]";
        };
        virtual std::string getMdIdentVal() = 0;

        virtual void addMdLog(MdLogVal* mdLogVal);

    };


    void mfAssert(bool valid, std::string msg);



}

#endif //KATHRYN_MODELDEBUGGER_H
