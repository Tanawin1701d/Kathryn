//
// Created by tanawin on 28/12/2024.
//

#include "checker.h"

namespace kathryn{
    namespace carolyne{
        void crlAss(const bool& cond, const std::string& errorValue){
            if (!cond){
                std::cout << TC_RED << "[CAROLYNE] "<< errorValue << TC_DEF << std::endl;
                assert(false);
            }
        }

        void crlVb(const std::string& vbStr){
            std::cout << TC_BLUE << "[CAROLYNE] " << vbStr << TC_DEF << std::endl;
        }
    }
}
