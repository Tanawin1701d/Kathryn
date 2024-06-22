//
// Created by tanawin on 20/6/2024.
//


#ifndef KATHRYN_GEN_PROXYHWCOMP_ABSTRCT
#define KATHRYN_GEN_PROXYHWCOMP_ABSTRCT

#include <utility>

#include "model/hwComponent/abstract/assignable.h"
#include "logicCerf.h"

namespace kathryn{

    class ModuleGen;
    class LogicGenBase{
    protected:
        ModuleGen*     _mdGenMaster = nullptr;
        logicLocalCef  _cerf{};
        Assignable*    _asb = nullptr;
        Identifiable*  _ident = nullptr;

    public:
        explicit LogicGenBase(ModuleGen*    mdGenMaster,
                              logicLocalCef cerf,
                              Assignable*   asb,
                              Identifiable* ident
        );
        virtual ~LogicGenBase() = default;

        std::string getOprStrFromOpr(Operable* opr);

        ///////// routing zone
        virtual void routeDep() = 0; ///// do routing
        ///// check zone

        ///////// get zone
        virtual std::string getOpr()        ;
        virtual std::string getOpr(Slice sl);
        //////// gen zone
        virtual std::string decIo()             = 0;
        virtual std::string decVariable()       = 0;
        virtual std::string decOp()             = 0;


        ///////// getter
        [[nodiscard]] ModuleGen* getModuleGen() const{
            assert(_mdGenMaster != nullptr);
            return _mdGenMaster;
        }
        [[nodiscard]] logicLocalCef getLogicCef() const{
            return _cerf;
        }

    };



    class LogicGenInterface{
    public:
        virtual ~LogicGenInterface() = default;
        virtual LogicGenBase* getLogicGen() = 0;
    };

}


#endif
