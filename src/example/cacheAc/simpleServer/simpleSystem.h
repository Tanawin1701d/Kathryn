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

        explicit SimpleServer(const SERVER_PARAM& svParam):
        ServerBase(svParam),
        SUFFIX_BIT(_svParam.kvParam.KEY_SIZE - _svParam.prefixBit){
            assert(SUFFIX_BIT > 0);
            initServer();
        }

        ~SimpleServer() override = default;

        CacheBankBase* genBank(int idx) override{
            mMod(smBank, SimpleBank, _svParam.kvParam, SUFFIX_BIT);
            return &smBank;
        }

        IngressBase* genIngress() override{
            std::vector<BankInputInterface*> bankInItf;
            assert(!_banks.empty());
            for (CacheBankBase* bank: _banks){
                assert(bank != nullptr);
                bankInItf.push_back(bank->getBankInputInterface());
            }
            mMod(ingr, SimpleIngress, _svParam, bankInItf);
            _ingress = &ingr;
            return &ingr;
        }

        OutgressBase* genOutgress() override{
            std::vector<BankOutputInterface*> bankOutItf;
            assert(!_banks.empty());
            for (CacheBankBase* bank: _banks){
                assert(bank != nullptr);
                bankOutItf.push_back(bank->getBankOutputInterface());
            }
            mMod(outr, SimpleOutgress, _svParam, bankOutItf);
            _outgress = &outr;
            return &outr;
        }

    };

}

#endif //SIMPLESYSTEM_H
