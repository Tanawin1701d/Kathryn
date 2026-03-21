//
// Created by tanawin on 1/4/2567.
//

#ifndef KATHRYN_TXUART_H
#define KATHRYN_TXUART_H
#include "kathryn.h"

namespace kathryn{

    class TxUart: public Module{
    public:
        const int _buadCycle;
        Pipe& tx_pipe = make_pipe();
        make_reg(sig   , 1);
        make_reg(i     , 4);
        make_reg(result,10);

        explicit TxUart(int buad_cycle):Module(), _buadCycle(buad_cycle){};

        void flow() override{
            seq{
                sig <<= 1;
                cwhile(true) {
                    pip_wait_send(tx_pipe)
                    cwhile(i < 8) {
                        par {
                            sig    <<= result(0);
                            result <<= (result >> 1);
                            i      <<= i + 1;
                        }
                        sy_wait(_buadCycle-1)
                    }
                }
            }
        }
    };
}

#endif //KATHRYN_TXUART_H
