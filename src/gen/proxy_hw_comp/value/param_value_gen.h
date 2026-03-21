//
// Created by tanawin on 15/1/2025.
//

#ifndef KATHRYN_SRC_GEN_PROXYHWCOMP_VALUE_PARAMVALUEGEN_H
#define KATHRYN_SRC_GEN_PROXYHWCOMP_VALUE_PARAMVALUEGEN_H

#include "gen/proxy_hw_comp/abstract/logic_gen_base.h"

namespace kathryn{

    class PmVal;
    class ParamValGen: public LogicGenBase{
        PmVal* _master = nullptr;

    public:
        explicit ParamValGen(ModuleGen* md_gen_master,
                             PmVal*     master);

        void route_dep() override{};

        std::string dec_param_val() override;

        std::string dec_op() override;

    };

}

#endif //KATHRYN_SRC_GEN_PROXYHWCOMP_VALUE_PARAMVALUEGEN_H
