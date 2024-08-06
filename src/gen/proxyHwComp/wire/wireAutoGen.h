//
// Created by tanawin on 20/6/2024.
//

#ifndef WIREAUTOGEN_H
#define WIREAUTOGEN_H
#include "model/hwComponent/wire/wireAuto.h"
#include "gen/proxyHwComp/abstract/AssignGen.h"

namespace kathryn{

    class Wire;
    /** Wire which is NOT exposed to model it is auto generate wire
     * cause of auto generate is
     * auto input/output routing
     * global input/output wire representation
     */
    class WireAutoGen: public AssignGenBase{
    protected:
        WIRE_AUTO_GEN_TYPE _autoWireGenType; ///// active when isModelWire is false
        WireAuto*          _master = nullptr;
    public:

        WireAutoGen(ModuleGen*    mdGenMaster,
                    WireAuto*         wireMaster,
                    WIRE_AUTO_GEN_TYPE  ioType);

        std::string getOpr()     override;

        std::string decIo()       override;
        std::string decVariable() override; ///// in case output there is no need dec variable
        std::string decOp()       override;

        bool compare(LogicGenBase* lgb) override;
    };

}

#endif //WIREAUTOGEN_H
