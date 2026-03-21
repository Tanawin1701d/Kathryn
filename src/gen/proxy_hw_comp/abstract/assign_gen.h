//
// Created by tanawin on 22/6/2024.
//

#ifndef ASSIGNGEN_H
#define ASSIGNGEN_H

#include "logic_gen_base.h"

namespace kathryn{

    class AssignGenBase: public LogicGenBase{
    protected:

        UpdatePool translated_update_pool;


    public:
        explicit AssignGenBase(ModuleGen*    md_gen_master,
                              Assignable*   asb,
                              Identifiable* ident
        ):LogicGenBase(md_gen_master, asb, ident){}

        ~AssignGenBase() override;

        void route_dep() override;

        std::pair<Verilog_SEN_TYPE, std::string> get_clock_sen_info();

        /**
         *  assign system
         */
        //std::string assign_op_with_chain_condition(bool is_clock_sen);
        std::string assign_op_with_sole_condition();
        std::string assign_op_base();

        virtual std::string assignment_line(Slice des_slice,
                                           Operable* src_update_value,
                                           bool is_delayed_asm);


        void add_direct_update_event(UpdateEventBase* ueb) override{
            assert(ueb != nullptr);
            translated_update_pool.add_update_event(ueb);
        }

    };


}

#endif //ASSIGNGEN_H
