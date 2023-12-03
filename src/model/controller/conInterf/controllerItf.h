//
// Created by tanawin on 1/12/2566.
//

#ifndef KATHRYN_CONTROLLERITF_H
#define KATHRYN_CONTROLLERITF_H

#include<memory>
#include <cassert>

namespace kathryn {

    class Controller;
    typedef std::shared_ptr<Controller> ControllerPtr;

    /**
     * This is used for hardware element to interface with comtroller
     * */
    class HwCompControllerItf {
    protected:
        ControllerPtr ctrl;
    public:
        explicit HwCompControllerItf();

        /** use while constructor is invoke*/
        virtual void com_init() { assert(true); }
        /** use when constructor is finish*/
        virtual void com_final(){ assert(true); /**todo */ }
    };


}

#endif //KATHRYN_CONTROLLERITF_H
