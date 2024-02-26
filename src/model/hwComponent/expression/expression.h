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
    friend class expressionLogicSim;
    private:

        /** metas data that contain bi operation*/
        LOGIC_OP _op;
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
        expression& operator <<= (Operable  & b) override {mfAssert(false, "expr don't support this <<= assigment"); assert(false);}
        expression& operator <<= (ull b)         override {mfAssert(false, "expr don't support this <<= assigment"); assert(false);}
        void generateAssMetaForBlocking(Operable& srcOpr,
                                        std::vector<AssignMeta*>& resultMetaCollector,
                                        Slice  absSrcSlice,
                                        Slice  absDesSlice) override{
            mfAssert(false, "expr don't support generateAssMetaForBlocking"); assert(false);
        }
        expression& operator =   (Operable  & b) override;
        expression& operator =   (expression& b);
        expression& operator = (ull b)         override {mfAssert(false, "expr don't support this = assigment with ull overload"); assert(false);}
        void generateAssMetaForNonBlocking(Operable& srcOpr,
                                           std::vector<AssignMeta*>& resultMetaCollector,
                                           Slice  absSrcSlice,
                                           Slice  absDesSlice) override{
            mfAssert(false, "expr don't support generateAssMetaForNonBlocking"); assert(false);
        }
        /**override operable*/
        [[nodiscard]]
        Slice getOperableSlice() const override  { return getSlice(); }
        [[nodiscard]]
        Operable& getExactOperable() const override { return *(Operable*)(this);};
        [[nodiscard]]



        /** override slicable*/
        SliceAgent<expression>& operator() (int start, int stop) override;
        SliceAgent<expression>& operator() (int idx) override;
        Operable* doSlice(Slice sl) override;
        /** call back assignable from client agent*/
        [[maybe_unused]]
        expression& callBackBlockAssignFromAgent(Operable& b, Slice absSlice) override;
        expression& callBackNonBlockAssignFromAgent(Operable& b, Slice absSlice) override;
        void        callBackBlockAssignFromAgent(Operable& srcOpr,
                                                 std::vector<AssignMeta*>& resultMetaCollector,
                                                 Slice  absSrcSlice,
                                                 Slice  absDesSlice) override{assert(false);}
        void        callBackNonBlockAssignFromAgent(Operable& srcOpr,
                                                    std::vector<AssignMeta*>& resultMetaCollector,
                                                    Slice  absSrcSlice,
                                                    Slice  absDesSlice) override{assert(false);}

        /** override debugg message*/
        //std::vector<std::string> getDebugAssignmentValue() override;

        /** get set method */
        LOGIC_OP getOp() const {return _op;};
        Operable* getOperandA() const {return _a;}
        Operable* getOperandB() const {return _b;}

        /**check short*/
        Operable* checkShortCircuit() override;

    };

    class expressionLogicSim: public LogicSimEngine{
        expression* _master = nullptr;
    public:
        expressionLogicSim(expression* master,int sz, VCD_SIG_TYPE sigType, bool simForNext);
        /** override simulation engine */
        void simStartCurCycle() override;

    };


}

#endif //KATHRYN_EXPRESSION_H
