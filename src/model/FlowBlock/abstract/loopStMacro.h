//
// Created by tanawin on 4/12/2566.
//

#ifndef KATHRYN_LOOPSTMACRO_H
#define KATHRYN_LOOPSTMACRO_H

namespace kathryn{


    class LoopStMacro{

    private:
        bool isStartStage = true;

    public:

        void step(){
            /** this is used when change start stage to finish State*/
            isStartStage = false;
        }

        virtual void doPreFunction() = 0;
        virtual void doPostFunction() = 0;

        bool doPrePostFunction(){
            if (isStartStage){
                doPreFunction();
                return true;
            }else{
                doPostFunction();
                return false;
            }
        }

    };


}

#endif //KATHRYN_LOOPSTMACRO_H
