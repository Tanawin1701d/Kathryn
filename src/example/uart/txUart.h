//
// Created by tanawin on 1/4/2567.
//

#ifndef KATHRYN_TXUART_H
#define KATHRYN_TXUART_H
#include"kathryn.h"

namespace kathryn{

    class TxUart: public Module{
    public:
        const int _buadCycle;
        Pipe& txPipe = makePipe();
        makeReg(sig   , 1);
        makeReg(i     , 4);
        makeReg(result,10);

        explicit TxUart(int buadCycle):Module(), _buadCycle(buadCycle){};

        void flow() override{
            seq{
                sig <<= 1;
                cwhile(true) {
                    pipWaitSend(txPipe)
                    cwhile(i < 8) {
                        par {
                            sig    <<= result(0);
                            result <<= (result >> 1);
                            i      <<= i + 1;
                        }
                        syWait(_buadCycle-1)
                    }
                }
            }
        }
    };
}

#endif //KATHRYN_TXUART_H
