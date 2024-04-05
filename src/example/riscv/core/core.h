//
// Created by tanawin on 1/4/2567.
//

#ifndef KATHRYN_CORE_H
#define KATHRYN_CORE_H

#include"kathryn.h"

namespace kathryn{

    namespace riscv {

        class Riscv : public Module {

            const int XLEN = 32;

            makeReg(pc, XLEN);
            makeWire(predFault, 1);
            makeWire(correctPc, XLEN);

            void flow() override {

                cwhile(true) {
                    zif (predFault) {
                        pc <<= correctPc;
                    }
                    zelse {
                        pc <<= pc + 1;
                    }
                }


            }

        };

    }

}

#endif //KATHRYN_CORE_H
