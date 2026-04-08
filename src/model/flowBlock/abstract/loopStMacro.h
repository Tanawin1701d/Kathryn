//
// Created by tanawin on 4/12/2566.
//

#ifndef KATHRYN_LOOPSTMACRO_H
#define KATHRYN_LOOPSTMACRO_H

namespace kathryn{

    class LoopStMacro{

    private:
        bool _is_start_stage = true;

    public:

        void step(){
            /** this is used when change start stage to finish State*/
            _is_start_stage = false;
        }

        virtual void do_pre_function() = 0;
        virtual void do_post_function() = 0;

        bool do_pre_post_function(){
            if (_is_start_stage){
                do_pre_function();
                return true;
            }else{
                do_post_function();
                return false;
            }
        }

    };

}

#endif //KATHRYN_LOOPSTMACRO_H
