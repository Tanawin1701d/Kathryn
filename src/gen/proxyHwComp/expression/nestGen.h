//
// Created by tanawin on 20/6/2024.
//

#ifndef NESTGEN_H
#define NESTGEN_H
#include "gen/proxyHwComp/abstract/logicGenBase.h"
#include "model/hwComponent/expression/nest.h"

namespace kathryn{

    class NestGen: public LogicGenBase{
    private:
        std::vector<Operable*> _routedNestList;
        nest* _master = nullptr;

    public:
        explicit NestGen(ModuleGen*    mdGenMaster,
                         logicLocalCef cerf,
                         nest*         nestMaster);
        ////////// routing zone
        void routeDep() override;

        ///////// gen zone
        std::string decIo()       override;
        std::string decVariable() override;
        std::string decOp()       override;



    };

}

#endif //NESTGEN_H
