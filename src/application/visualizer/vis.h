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
     *  meta data for wire and register (non state register)
     *  the value and depend cond is not recufrsively get
     * */
    struct compMeta{
        struct AsMeta{
            std::string value;
            std::string dependCond;
        };
        std::string regName;
        std::vector<AsMeta> asMetas;
    };

    /**
     * metadata for expression not recursively get value
     * */
    struct NonMemorizeComp{
        std::string compName;
        std::string value;
    };

    /** Vis is used for print out state and
     * condition that occure in each module
     * */
    class Vis{

    private:
        Module* _sampleModel;
        std::vector<StateMeta> stateMetas;
        std::vector<compMeta> regMetas;
        std::vector<compMeta> wireMetas;
        std::vector<NonMemorizeComp> expression;

    protected:
        void retrieveStateReg();
        void retrieveReg();
        void retrieveWire();
        void retrieveExpr();





    public:
        explicit Vis(Module* sampleModel);

        void execute();

        void print();

    };



}

#endif //KATHRYN_VIS_H
