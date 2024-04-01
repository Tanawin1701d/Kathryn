//
// Created by tanawin on 1/4/2567.
//
#ifndef KATHRYN_RXUART_H
#define KATHRYN_RXUART_H
#include"kathryn.h"

namespace kathryn{

    class RxUart: public Module{
    public:
        const int _buadCycle;
        Pipe& rxPipe = makePipe();
        makeWire(sig  , 1);
        makeReg(i     , 4);
        makeReg(result, 8);

        explicit RxUart(int buadCycle):Module(), _buadCycle(buadCycle){};

        void flow() override {
            seq
                cwhile(true) {
                    scWait(sig == 0);
                    i <<= 0;
                    syWait(_buadCycle+(_buadCycle/2))
                    cwhile(i < 8) {
                        par {
                            result <<= g(result, sig);
                            i      <<= i + 1;
                        }
                        syWait(_buadCycle);
                    }
                    pipWaitSend(rxPipe);
                }
        }
    };
}
#endif //KATHRYN_RXUART_H