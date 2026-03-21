//
// Created by tanawin on 20/6/2024.
//

#ifndef EXPRESSION_H
#define EXPRESSION_H
#include "gen/proxy_hw_comp/abstract/logic_gen_base.h"

namespace kathryn{

    class expression;
    class ExprGen: public LogicGenBase{

    protected:
        expression* _master = nullptr;

        Operable*   _routedOprA = nullptr;
        Operable*   _routedOprB = nullptr;


    public:
        explicit ExprGen(ModuleGen*    md_gen_master,
                         expression*   master);

        void route_dep() override;

        std::string dec_io() override;
        std::string dec_variable() override;
        std::string dec_op() override;

        static std::string cvt_to_sign_sig(std::string src_str);
    };

}

#endif //EXPRESSION_H
