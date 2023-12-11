//
// Created by tanawin on 30/11/2566.
//

#include "value.h"
#include "model/controller/controller.h"


namespace kathryn{

    Val::Val(int size, std::string v):
                                    LogicComp({0, size}, TYPE_VAL, false),
                                    _size(size)
                                    {
                                        com_init();
    }

    void Val::com_init() {
        ctrl->on_value_init(this);
    }

    SliceAgent<Val>& Val::operator()(int start, int stop){

        auto ret = new SliceAgent<Val>(
                    this,
                    getNextSlice(start, stop, getSlice())
                );
        return *ret;
    }

    SliceAgent<Val>& Val::operator() (int idx){
        return operator() (idx, idx+1);
    }

    std::vector<std::string> Val::getDebugAssignmentValue() {
        return {"0"};
    }


}