//
// Created by tanawin on 20/6/2024.
//

#ifndef EXPRESSION_H
#define EXPRESSION_H
#include "gen/proxyHwComp/abstract/logicGenBase.h"

namespace kathryn{

    class expression;
    class ExprGen: public LogicGenBase{

    protected:
        expression* _master = nullptr;

        Operable*   _routedOprA = nullptr;
        Operable*   _routedOprB = nullptr;


    public:
        explicit ExprGen(ModuleGen*    mdGenMaster,
                         expression*   master);

        void routeDep() override;

        std::string decIo() override;
        std::string decVariable() override;
        std::string decOp() override;
    };

}

#endif //EXPRESSION_H
