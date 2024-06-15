//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_EXPRESSION_H
#define KATHRYN_EXPRESSION_H


#include <memory>
#include <string>
#include <iostream>
#include <utility>

#include "model/controller/conInterf/controllerItf.h"
#include "model/hwComponent/abstract/logicComp.h"


/**
 * exprMetas is the class that represent the value from hardware
 * component such as register and wire
 *
 * */

namespace kathryn {




    class expression : public LogicComp<expression>{
    friend class expressionSimEngine;
    private:
        bool _valueAssinged = false;
        /** metas data that contain bi operation*/
        const LOGIC_OP _op;
        Operable* _a;
        Operable* _b;

    protected:
        void com_init() override;



    public:
        /** constructor auto get id of the system*/
        explicit expression(LOGIC_OP op,
                            const Operable* a,
                            const Operable* b,
                            int exp_size
                            );
        explicit expression(int exp_size);

        void com_final() override {};
        /** override assignable*/
        void doBlockAsm(Operable& srcOpr, Slice desSlice) override{
            mfAssert(false, "expr don't support doBlockAsm");
            assert(false);
        };
        void doNonBlockAsm(Operable& srcOpr, Slice desSlice) override;

        void doBlockAsm(Operable& srcOpr,
                        std::vector<AssignMeta*>& resultMetaCollector,
                        Slice  absSrcSlice,
                        Slice  absDesSlice) override{
            mfAssert(false, "expr don't support doBlockAsm"); assert(false);
        }
        void doNonBlockAsm(Operable& srcOpr,
                           std::vector<AssignMeta*>& resultMetaCollector,
                           Slice  absSrcSlice,
                           Slice  absDesSlice) override{
            mfAssert(absDesSlice == getSlice()                    ,
                     "des expression assign wrapper doesn't cover entire expression");
            mfAssert(absSrcSlice.getSize() >= getSlice().getSize(),
                     "src expression assign wrapper doesn't cover entire expression");
            doGlobalAsm(srcOpr, resultMetaCollector, absSrcSlice, absDesSlice, ASM_DIRECT);
        }

        expression& operator = (Operable& b)  { operatorEq(b);                                return *this;}
        expression& operator = (ull b)        { operatorEq(b);                                   return *this;}
        expression& operator = (expression& b){ if (this == &b){return *this;} operatorEq(b); return *this;}
        /**override operable*/




        /** override slicable*/
        SliceAgent<expression>& operator() (int start, int stop) override;
        SliceAgent<expression>& operator() (int idx) override;
        SliceAgent<expression>& operator() (Slice sl) override;
        Operable* doSlice(Slice sl) override;

        /** override debugg message*/
        //std::vector<std::string> getDebugAssignmentValue() override;

        /** get set method */
        LOGIC_OP getOp() const {return _op;};
        Operable* getOperandA() const {return _a;}
        Operable* getOperandB() const {return _b;}

        /**check short*/
        Operable* checkShortCircuit() override;

    };

    class expressionSimEngine: public LogicSimEngine{
        expression* _master = nullptr;
    public:
        expressionSimEngine(expression* master, VCD_SIG_TYPE sigType);
        void proxyBuildInit() override;
        /** override simulation engine */
        std::string createGlobalVariable() override;
        std::string createLocalVariable()  override;
        std::string createOp() override;
    };


}

#endif //KATHRYN_EXPRESSION_H
