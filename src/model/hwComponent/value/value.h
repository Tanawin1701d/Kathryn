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

namespace kathryn{

    /** This class act as constant value */
    class Val: public Operable,
               public Identifiable,
               HwCompControllerItf{
    protected:
        int _size;
        void com_init() override;
    public:
        /** todo we will make value save the value and range more precisly*/
        explicit Val(int size, std::string v);
        /** todo we will deal with it later*/
        [[maybe_unused]]
        /** todo assign it  */
        expression& operator <<= (Operable& b){ assert(true);};
        Val& operator = (std::string& b){return *this;};
        Val& operator = (ull v){return *this;};

        /** override operable*/
        [[nodiscard]]
        Slice getOperableSlice() const override { return Slice{0, _size};}
        [[nodiscard]]
        Operable& getExactOperable() const override {return (Operable &) *this;}

        /** assign todo we will assign it later*/
        Val operator() (int start, int stop) {return Val(stop-start, "b000");}
        Val operator() (int idx) {return Val(1, "0b00");}

    };


}

#endif //KATHRYN_VALUE_H
