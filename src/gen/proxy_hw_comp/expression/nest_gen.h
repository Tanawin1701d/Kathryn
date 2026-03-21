//
// Created by tanawin on 20/6/2024.
//

#ifndef NESTGEN_H
#define NESTGEN_H

#include "gen/proxy_hw_comp/abstract/logic_gen_base.h"

namespace kathryn{

    class nest;
    class NestGen: public LogicGenBase{
    private:
        std::vector<Operable*> _routedNestList;
        nest* _master = nullptr;

    public:
        explicit NestGen(ModuleGen*    md_gen_master,
                         nest*         nest_master);

        ~NestGen() override = default;

        ////////// routing zone
        void route_dep() override;

        ///////// gen zone
        std::string dec_io()       override;
        std::string dec_variable() override;
        std::string dec_op()       override;


    };

}

#endif //NESTGEN_H
