//
// Created by tanawin on 1/12/2566.
//

#ifndef KATHRYN_CONTROLLERITF_H
#define KATHRYN_CONTROLLERITF_H

#include<memory>
#include <cassert>

namespace kathryn {

    class ModelController;
    /**
     * This is used for hardware element to interface with comtroller
     * */
    class HwCompControllerItf {
    protected:
        ModelController* ctrl;
    public:
        explicit HwCompControllerItf(bool requiredAllocCheck = true);

        /** use while constructor is invoke*/
        virtual void com_init() { assert(false); }
        /** use when constructor is finish*/
        virtual void com_final(){ assert(false); /**todo */ }
    };


}

#endif //KATHRYN_CONTROLLERITF_H
