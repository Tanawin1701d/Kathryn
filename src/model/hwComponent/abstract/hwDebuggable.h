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
        /** this is used to retrieve everything
         * that used to identify hardware component*/
        virtual std::string getDebugIdentValue() = 0;

    };


}

#endif //KATHRYN_HWDEBUGGABLE_H
