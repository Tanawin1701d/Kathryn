//
// Created by tanawin on 1/4/2567.
//
#ifndef KATHRYN_RXUART_H
#define KATHRYN_RXUART_H
#include "kathryn.h"

namespace kathryn{

    class RxUart: public Module{
    public:
        const int _buadCycle;
        Pipe& rx_pipe = make_pipe();
        make_wire(sig  , 1);
        make_reg(i     , 4);
        make_reg(result, 8);

        explicit RxUart(int buad_cycle):Module(), _buadCycle(buad_cycle){};

        void flow() override {
            seq
                cwhile(true) {
                    sc_wait(sig == 0);
                    i <<= 0;
                    sy_wait(_buadCycle+(_buadCycle/2))
                    cwhile(i < 8) {
                        par {
                            result <<= g(result, sig);
                            i      <<= i + 1;
                        }
                        sy_wait(_buadCycle);
                    }
                    pip_wait_send(rx_pipe);
                }
        }
    };
}
#endif //KATHRYN_RXUART_H