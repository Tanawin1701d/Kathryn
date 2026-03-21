//
// Created by tanawin on 12/1/2567.
//

#ifndef KATHRYN_MODELDEBUGGER_H
#define KATHRYN_MODELDEBUGGER_H

#include "string"
#include "util/logger/logger.h"

namespace kathryn{


    class ModelDebuggable{

    public:
        virtual ~ModelDebuggable() = default;
        /** this is legacy method*/
        virtual std::string get_md_describe(){
            return "[this component has no description]";
        };
        virtual std::string get_md_ident_val() = 0;

        virtual void add_md_log(MdLogVal* md_log_val);

    };


    void mf_assert(bool valid, std::string msg);
    void mf_warn  (bool valid, std::string msg);



}

#endif //KATHRYN_MODELDEBUGGER_H
