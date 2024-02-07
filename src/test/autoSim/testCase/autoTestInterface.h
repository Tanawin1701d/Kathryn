//
// Created by tanawin on 7/2/2567.
//

#ifndef KATHRYN_AUTOTESTINTERFACE_H
#define KATHRYN_AUTOTESTINTERFACE_H


namespace kathryn{


    class AutoTestEle{
    private:
        int _simId = -1;
    public:
        explicit AutoTestEle(int simId);
        virtual void start() = 0;
        int getSimId() const {return _simId;}

    };



}

#endif //KATHRYN_AUTOTESTINTERFACE_H
