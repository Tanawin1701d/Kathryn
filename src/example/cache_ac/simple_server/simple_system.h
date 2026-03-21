//
// Created by tanawin on 15/9/2024.
//

#ifndef SIMPLESYSTEM_H
#define SIMPLESYSTEM_H
#include "simple_outgress.h"
#include "example/cache_ac/system.h"
#include "example/cache_ac/simple_server/simple_bank.h"
#include "example/cache_ac/simple_server/simple_ingress.h"

namespace kathryn::cache_server{

    class SimpleServer: public ServerBase{
    public:
        const int SUFFIX_BIT = 0;

        explicit SimpleServer(SERVER_PARAM& sv_param):
        ServerBase(sv_param),
        SUFFIX_BIT(_svParam.kv_param.KEY_SIZE - _svParam.prefix_bit){
            assert(SUFFIX_BIT > 0);
            init_server();
        }

        CacheBankBase* gen_bank(int idx) override{
            m_mod(sm_bank, SimpleBank, _svParam.kv_param, SUFFIX_BIT, idx);
            return &sm_bank;
        }

        IngressBase* gen_ingress() override{
            m_mod(ingr, SimpleIngress, _svParam, _bankInputItfs);
            return &ingr;
        }

        OutgressBase* gen_outgress() override{
            m_mod(outr, SimpleOutgress, _svParam, _bankOutputItfs);
            return &outr;
        }

    };

}

#endif //SIMPLESYSTEM_H
