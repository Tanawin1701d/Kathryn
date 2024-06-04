//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_OPERABLE_H
#define KATHRYN_OPERABLE_H

#include<memory>
#include <model/debugger/modelDebugger.h>
#include <model/simIntf/hwComponent/logicSimEngine.h>

#include "Slice.h"
#include "operation.h"
#include "identifiable.h"
#include "makeComponent.h"
#include "model/hwComponent/abstract/modelMode.h"
#include "model/simIntf/base/modelSimEngine.h"

namespace kathryn {

    /**
     * Operable represents hardware component that can be drive the
     * logic value
     *
     * */
    template<typename T>
    class LogicComp;
    class expression;
    class LogicSimEngine;
    struct AssignMeta;
    class Operable{

    public:
        explicit Operable() = default;
        virtual ~Operable() = default;
        /** bitwise operators*/
        virtual expression& operator &  ( Operable& b);
        virtual expression& operator |  (Operable& b);
        virtual expression& operator ^  ( Operable& b);
        virtual expression& operator ~  ();
        virtual expression& operator << ( Operable& b);
        virtual expression& operator >> ( Operable& b);
        /** logical operators*/
        virtual expression& operator && ( Operable& b);
        virtual expression& operator || ( Operable& b);
        virtual expression& operator !  ();
        /** relational operator*/
        virtual expression& operator == ( Operable& b);
        virtual expression& operator != ( Operable& b);
        virtual expression& operator <  ( Operable& b);
        virtual expression& operator <= ( Operable& b);
        virtual expression& operator >  ( Operable& b);
        virtual expression& operator >= ( Operable& b);
        /** arithmetic operators*/
        virtual expression& operator +  ( Operable& b);
        virtual expression& operator -  ( Operable& b);
        virtual expression& operator *  ( Operable& b);
        virtual expression& operator /  ( Operable& b);
        virtual expression& operator %  ( Operable& b);
        /** todo for now self assign operation such as += is not permit */

        /**
         *
         * rhs is match ull size
         *
         * */
        virtual expression& operator &  (ull b);
        virtual expression& operator |  (ull b);
        virtual expression& operator ^  (ull b);
        virtual expression& operator << (ull b);
        virtual expression& operator >> (ull b);
        /** logical operators*/
        virtual expression& operator && (ull b);
        virtual expression& operator || (ull b);
        /** relational operator*/
        virtual expression& operator == (ull b);
        virtual expression& operator != (ull b);
        virtual expression& operator <  (ull b);
        virtual expression& operator <= (ull b);
        virtual expression& operator >  (ull b);
        virtual expression& operator >= (ull b);
        /** arithmetic operators*/
        virtual expression& operator +  (ull b);
        virtual expression& operator -  (ull b);
        virtual expression& operator *  (ull b);
        virtual expression& operator /  (ull b);
        virtual expression& operator %  (ull b);


        /** due to slice operable maybe change*/
        [[nodiscard]]
        virtual Slice           getOperableSlice() const = 0;
        [[nodiscard]]
        virtual Operable&       getExactOperable () const = 0;

        virtual Operable*       doSlice(Slice sl) = 0; //// sl is abs value
        /** please remind this is a copy not reference value*/
        virtual LogicSimEngine* getLogicSimEngineFromOpr() = 0;

        explicit operator ull(){
            LogicSimEngine* simEngine = getLogicSimEngineFromOpr();
            mfAssert(getAssignMode() == AM_SIM, "can't retrieve data in model building mode");
            mfAssert(simEngine != nullptr, "get value fail");
            return simEngine->getProxyRep()->getVal();
        }
        explicit operator ValRepBase(){
            LogicSimEngine* simEngine = getLogicSimEngineFromOpr();
            return *simEngine->getProxyRep();
        }

        /**downcasting*/
        virtual Identifiable*   castToIdent() = 0;

        Operable&               getMatchOperable(ull value) const;

        /** check logic section*/
        bool isInCheckPath = false;
        virtual Operable*       checkShortCircuit() = 0;

    };

}

#endif //KATHRYN_OPERABLE_H
