//
// Created by tanawin on 12/1/2567.
//

#ifndef KATHRYN_MODELDEBUGGER_H
#define KATHRYN_MODELDEBUGGER_H

#include <string>
#include "util/logger/logger.h"

namespace kathryn{

    /**
     * The base class of the model of the hardware to recursively debugable
     *
     * */
    class ModelDebuggable{

    public:
        /** this is legacy method*/
        virtual std::string getMdDescribe(){
            return "[this component has no description]";
        };
        virtual std::string getMdIdentVal() = 0;

        virtual void addMdLog(MdLogVal* mdLogVal);

    };

    /////// the assertation to make sure that the system is running correctly
    void mfAssert(bool valid, std::string msg);



}

#endif //KATHRYN_MODELDEBUGGER_H
