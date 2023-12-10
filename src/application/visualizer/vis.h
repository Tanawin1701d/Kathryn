//
// Created by tanawin on 10/12/2566.
//

#ifndef KATHRYN_VIS_H
#define KATHRYN_VIS_H

#include <string>

#include "model/hwComponent/module/module.h"

namespace kathryn{

    /***
     * dependState recursively get until got state reg or register
     * */
    struct StateMeta{
        struct Trmeta{
            std::string dependState;
            std::string dependCondition;
        };
        std::string stateName;
        std::vector<Trmeta> trMetas;
    };

    /**
     * metadata for exprMetas , normal reg, normal wire not recursively get value
     * */
    struct CompDebugMessage{
        std::string compName;
        std::vector<std::string> values;
    };

    /** Vis is used for print out state and
     * condition that occure in each module
     * */
    class Vis{

    private:
        Module* _sampleModule;
        std::vector<StateMeta> stateMetas;
        std::vector<CompDebugMessage> regMetas;
        std::vector<CompDebugMessage> wireMetas;
        std::vector<CompDebugMessage> exprMetas;

    protected:
        template<typename T>
        void retrieveSimpleAsm(T& srcHwComp, std::vector<CompDebugMessage>& desStore){
            for (auto hwCompPtr: srcHwComp){
                CompDebugMessage dbgMsg;
                dbgMsg.compName = hwCompPtr->getGlobalName();
                dbgMsg.values = hwCompPtr->getDebugAssignmentValue();
                desStore.push_back(dbgMsg);
            }
        }

    public:
        explicit Vis(Module* sampleModel);

        void execute();

        void print();

    };



}

#endif //KATHRYN_VIS_H
