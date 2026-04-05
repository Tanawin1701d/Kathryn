//
// Created by tanawin on 10/12/2566.
//

#ifndef KATHRYN_LOGICCOMP_H
#define KATHRYN_LOGICCOMP_H

#include "model/hwComponent/abstract/modelMode.h"
#include "model/hwComponent/abstract/slicable.h"
#include "model/hwComponent/abstract/operation.h"
#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/abstract/operable.h"
#include "model/hwComponent/abstract/identifiable.h"
#include "model/controller/conInterf/controllerItf.h"
#include "model/debugger/modelDebugger.h"
#include "sim/modelSimEngine/hwComponent/abstract/logicSimEngine.h"
#include "util/numberic/numConvert.h"

#include "gen/proxyHwComp/abstract/logicGenBase.h"
// #include "gen/proxyHwComp/expression/exprGen.h"
// #include "gen/proxyHwComp/expression/nestGen.h"
// #include "gen/proxyHwComp/register/regGen.h"
// #include "gen/proxyHwComp/value/valueGen.h"
// #include "gen/proxyHwComp/wire/wireGen.h"
// #include "gen/proxyHwComp/memBlock/memGen.h"
// #include "gen/proxyHwComp/memBlock/memAgentGen.h"


namespace kathryn{

    template<typename TYPE_COMP>
    class LogicComp : public AssignOpr<TYPE_COMP>,
                      public Assignable,
                      public Operable,
                      public Slicable<TYPE_COMP>,
                      public AssignCallbackFromAgent<TYPE_COMP>,
                      public Identifiable,
                      public HwCompControllerItf,
                      public ModelDebuggable,
                      public LogicSimEngineInterface,
                      public LogicGenInterface{
    protected:
        LogicSimEngine* _simEngine =  nullptr;
        LogicGenBase*   _genEngine =  nullptr;

    public:

        explicit LogicComp(Slice slc,
                           HW_COMPONENT_TYPE hwType,
                           LogicSimEngine* simEngine,
                           bool requiredAllocCheck):
                AssignOpr<TYPE_COMP>(),
                Assignable(),
                Operable(),
                Slicable<TYPE_COMP>(slc),
                Identifiable(hwType),
                HwCompControllerItf(requiredAllocCheck),
                ////LogicSimEngine(slc.getSize(), sigType, simForNext),
                ModelDebuggable(),
                _simEngine(simEngine)
                            {}

        virtual ~LogicComp() {
            delete _simEngine;
            delete _genEngine;
        }


        /** simulation engine override*/

        LogicSimEngine* getSimEnginePtr() override{
            return _simEngine;
        }

        void markSv(const std::string& key) override{
            _simEngine->markSv(key);
        }


        ///////////////////////////////////////////

        LogicGenBase* getLogicGenPtr() override{
            return _genEngine;
        }

        LogicGenBase* get_logic_gen_ptrBase() override{
            return _genEngine;
        }


        /** iterable override*/

        std::string getMdIdentVal() override{
            return getIdentDebugValue();
        }

        Operable* castToOpr(){
            return staticCast<Operable*>(this);
        }

        LogicSimEngine*getLogicSimEngineFromOprPtr() override{
            return _simEngine;
        }

        /** operable override*/
        Slice getOperableSlice() const override{
            return Slicable<TYPE_COMP>::getSlice();
        }

        Operable& getExactOperable() const override{
            return *(Operable*)this;
        }

        Identifiable* castToIdent() override{
            return staticCast<Identifiable*>(this);
        }


        /**
         *
         * override assignable
         *
         * */
        // void assignSimValue(ull b) override{
        //     mfAssert(getAssignMode() == AM_SIM, "cannot assign in model mode");
        //     assert(_simEngine != nullptr);
        //     //////// TODO assign the value
        //     getSimEnginePtr()->getProxyRep().setVar(b);
        // }

        LogicComp<TYPE_COMP>& s(ull value){
            assert(isCacheRepInit); ///// check from operable
            cachedRep.setVar(value);
            return *this;
        }

        LogicComp<TYPE_COMP>& s(ValRepBase value){
            assert(isCacheRepInit);
            cachedRep.setVar(value);
            return *this;
        }

        void assignSimValue(ull value) override{
            s(value);
        }

        void assignSimValue(ValRepBase value) override{
            s(value);
        }

        Slice getAssignSlice() override{
            return Slicable<TYPE_COMP>::getSlice();
        }

        virtual void makeDefEvent(ull defVal) {assert(false);}

        void startCheckShortCircuit(){
            Operable* matchOpr = checkShortCircuit();
            if (matchOpr != nullptr){
                std::cout << getMdIdentVal() << "  match with " << matchOpr->castToIdent()->getIdentDebugValue() << std::endl;
                mfAssert(false, "get short circuit");
            }
        }

        Assignable*getAssignableFromAssignOprPtr() override{
            return this;
        }

        Assignable*getAssignableFromAssignCallbackerPtr() override{
            return this;
        }

    };

}

#endif //KATHRYN_LOGICCOMP_H
