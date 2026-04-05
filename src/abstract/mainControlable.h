//
// Created by tanawin on 23/3/2567.
//

#ifndef KATHRYN_MAINCONTROLABLE_H
#define KATHRYN_MAINCONTROLABLE_H


namespace kathryn{


    class MainControlable{
    public:
        virtual ~MainControlable() = default;
        /**to start do somthing*/
        virtual void start() = 0;
        /** to reset old value and tempolary data*/
        virtual void reset() = 0;
        /** to prepare system for exit main program*/
        virtual void clean() = 0;

    };


}

#endif //KATHRYN_MAINCONTROLABLE_H
