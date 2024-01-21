//
// Created by tanawin on 30/11/2566.
//

#ifndef KATHRYN_VALUE_H
#define KATHRYN_VALUE_H

#include<string>
#include<vector>

#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/abstract/operable.h"
#include "model/hwComponent/abstract/identifiable.h"
#include "model/controller/conInterf/controllerItf.h"
#include "model/hwComponent/abstract/slicable.h"
#include "model/hwComponent/abstract/logicComp.h"

namespace kathryn{

    /** This class act as constant value */
    class Val: public LogicComp<Val>{
    protected:
        std::string rawValue;
        int _size;
        /***the actual value will be assigned to val rep*/

        void com_init() override;
    public:
        /** todo we will make value save the value and range more precisly*/
        explicit Val(int size, std::string v);


        /**
         * override assignable
         * */
        Val& operator <<= (Operable& b) override { assert(false);}
        Val& operator =   (Operable& b) override { assert(false);}
        Val& operator = (std::string& b){return *this;};
        Val& operator = (ull v){return *this;};

        /** override operable*/
        [[nodiscard]]
        Slice getOperableSlice() const override { return Slice{0, _size};}
        [[nodiscard]]
        Operable& getExactOperable() const override {return (Operable &) *this;}

        /** assign todo we will assign it later*/
        SliceAgent<Val>& operator() (int start, int stop) override;
        SliceAgent<Val>& operator() (int idx) override;

        Val& callBackBlockAssignFromAgent(Operable& b, Slice absSlice) override {assert(false);};
        Val& callBackNonBlockAssignFromAgent(Operable& b, Slice absSlice) override{assert(false);};

        /** override simulator*/
        void simStartCurCycle() override;
        void simExitCurCycle() override;

    };

}

#endif //KATHRYN_VALUE_H
