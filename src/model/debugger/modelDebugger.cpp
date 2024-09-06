//
// Created by tanawin on 12/1/2567.
//

#include <cassert>
#include "modelDebugger.h"
#include "util/logger/logger.h"
#include "model/controller/controller.h"


namespace kathryn{


    void ModelDebuggable::addMdLog(MdLogVal* mdLogVal) {

        assert(mdLogVal != nullptr);
        mdLogVal->addVal("This model log is not implemented.");
        mdLogVal->addVal("-------------------------------");

    }

    void mfAssert(bool valid, std::string msg){
        if (valid){
            return;
        }
        std::cout << msg << std::endl;
        std::cout << getControllerPtr()->getCurModelStack();
        assert(false);
    }

}