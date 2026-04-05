//
// Created by tanawin on 12/1/2567.
//

#include <cassert>
#include "modelDebugger.h"
#include "util/logger/logger.h"
#include "model/controller/controller.h"
#include "util/termColor/termColor.h"


namespace kathryn{


    void ModelDebuggable::add_md_log(MdLogVal *md_log_val) {

        assert(md_log_val != nullptr);
        md_log_val->addVal("This model log is not implemented.");
        md_log_val->addVal("-------------------------------");

    }

    void mf_assert(bool valid, std::string msg){
        if (valid){
            return;
        }
        std::cout << msg << std::endl;
        std::cout << get_controller_ptr()->get_cur_model_stack_dbg();
        assert(false);
    }


    void mf_warn(bool valid, std::string msg){

        if (valid){
            return;
        }
        std::cout << TC_YELLOW << msg << TC_DEF <<  std::endl;
    }

}
