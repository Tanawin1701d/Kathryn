//
// Created by tanawin on 15/9/2024.
//

#ifndef SIMPLESYSTEM_H
#define SIMPLESYSTEM_H
#include "simpleOutgress.h"
#include "example/cacheAc/system.h"
#include "example/cacheAc/simpleServer/simpleBank.h"
#include "example/cacheAc/simpleServer/simpleIngress.h"

namespace kathryn::cacheServer{

    class SimpleServer: public ServerBase{
    public:
        const int SUFFIX_BIT = 0;

        explicit SimpleServer(SERVER_PARAM& svParam):
        ServerBase(svParam),
        SUFFIX_BIT(_svParam.kvParam.KEY_SIZE - _svParam.prefixBit){
            assert(SUFFIX_BIT > 0);
            initServer();
        }

        CacheBankBase* genBank(int idx) override{
            mMod(smBank, SimpleBank, _svParam.kvParam, SUFFIX_BIT, idx);
            return &smBank;
        }

        IngressBase* genIngress() override{
            mMod(ingr, SimpleIngress, _svParam, _bankInputItfs);
            return &ingr;
        }

        OutgressBase* genOutgress() override{
            mMod(outr, SimpleOutgress, _svParam, _bankOutputItfs);
            return &outr;
        }

    };

}

#endif //SIMPLESYSTEM_H
