//
// Created by tanawin on 7/8/2024.
//

#ifndef RISCV_GEN_H
#define RISCV_GEN_H
#include "front_end/cmd/param_reader.h"

namespace kathryn{

    namespace riscv{
        class RISCV_GEN_MNG{
        public:
            void start_gen(PARAM& params);
        };
    }

}

#endif //RISCV_GEN_H
