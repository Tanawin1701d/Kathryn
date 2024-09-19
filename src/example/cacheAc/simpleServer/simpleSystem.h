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

        explicit SimpleServer(SERVER_PARAM svParam):
        ServerBase(svParam),
        SUFFIX_BIT(_svParam.kvParam.KEY_SIZE - _svParam.prefixBit){
            assert(SUFFIX_BIT > 0);
        }

        ~SimpleServer() override = default;

        CacheBankBase* genBank(int idx) override{
            return new SimpleBank(_svParam.kvParam, SUFFIX_BIT);
        }

        IngressBase* genIngress() override{
            std::vector<BankInputInterface*> bankInItf;
            for (CacheBankBase* bank: _banks){
                assert(bank != nullptr);
                bankInItf.push_back(bank->getBankInputInterface());
            }
            mMod(ingr, SimpleIngress, _svParam, bankInItf);
            return &ingr;
        }

        OutgressBase* genOutgress() override{
            std::vector<BankOutputInterface*> bankOutItf;
            for (CacheBankBase* bank: _banks){
                assert(bank != nullptr);
                bankOutItf.push_back(bank->getBankOutputInterface());
            }
            mMod(outr, SimpleOutgress, _svParam, bankOutItf);
            return &outr;
        }

    };

}

#endif //SIMPLESYSTEM_H
