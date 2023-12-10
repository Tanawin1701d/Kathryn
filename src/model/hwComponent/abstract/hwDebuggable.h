//
// Created by tanawin on 10/12/2566.
//

#ifndef KATHRYN_HWDEBUGGABLE_H
#define KATHRYN_HWDEBUGGABLE_H

#include <string>


namespace kathryn{

    class HwCompDebuggable{

    public:
        virtual std::vector<std::string> getDebugAssignmentValue() = 0;

    };


}

#endif //KATHRYN_HWDEBUGGABLE_H
