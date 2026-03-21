//
// Created by tanawin on 20/6/2024.
//

#ifndef VALUEGEN_H
#define VALUEGEN_H
#include "gen/proxy_hw_comp/abstract/logic_gen_base.h"
#include "model/hw_component/value/value.h"

namespace kathryn{


    class Val;
    class ValueGen: public LogicGenBase{
        Val* _master = nullptr;

    public:

        explicit ValueGen(ModuleGen*        md_gen_master,
                              Val*          master
        );

        void route_dep() override{};

        std::string get_val_str() const;

        std::string dec_io()       override;
        std::string dec_variable() override;
        std::string dec_op()       override;

    };


}

#endif //VALUEGEN_H
