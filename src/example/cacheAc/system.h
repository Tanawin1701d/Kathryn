//
// Created by tanawin on 13/9/2024.
//

#ifndef KATHRYN_SYSTEM_H
#define KATHRYN_SYSTEM_H

#include "bank.h"
#include "ingress.h"
#include "outgress.h"
#include <vector>

namespace kathryn::cacheServer{

        class ServerBase: public Module{
        public:
            SERVER_PARAM                _svParam;
            IngressBase*                _ingress  = nullptr;
            OutgressBase*               _outgress = nullptr;
            std::vector<CacheBankBase*> _banks;
            ///////// constructor
            explicit ServerBase(SERVER_PARAM svParam):_svParam(svParam){

            }
            ~ServerBase() override{
                delete _ingress;
                delete _outgress;
                for (CacheBankBase* bank: _banks){ delete bank; }
            }
            ///////// start build the element
            void initServer(){
                int amtBank = 1 << _svParam.prefixBit;
                for (int bankId = 0; bankId < amtBank; bankId++) {
                    _banks.push_back(genBank(bankId));
                }
                _ingress  =  genIngress();
                _outgress =  genOutgress();
            }
            ///////// gen element that used to init server
            virtual CacheBankBase* genBank    (int idx) = 0;
            virtual IngressBase*   genIngress ()        = 0;
            virtual OutgressBase*  genOutgress()        = 0;

            ///////// getter
            IngressBase& getIngress() const{
                assert(_ingress != nullptr);
                return *_ingress;
            }

            OutgressBase& getOutgress() const{
                assert(_outgress != nullptr);
                return *_outgress;
            }

            std::vector<CacheBankBase*>& getRefBanks(){return _banks;}

        };

    }


#endif //KATHRYN_SYSTEM_H
