//
// Created by tanawin on 19/7/2024.
//

#include "codeBaseWriter.h"

#include <utility>

namespace kathryn{
    /**
     *
     * cb if base
     *
     */
    CbBase::~CbBase(){
        std::cout << "deleting cbBase " << this << std::endl;
        for (CbBase* cb: _subBlocks){
            delete cb;
        }
    }

    void CbBase::addSt(std::string stm, bool ln){ //// ln = new line
        _codeSt.push_back({
            std::move(stm), lastOrder, ln, false
        });
        lastOrder++;
    }

    void CbBase::addCm(const std::string& comment){
        _codeSt.push_back({
            std::move(comment), lastOrder, true, true
        });
        lastOrder++;
    }

    void CbBase::appendSubBlock(CbBase* subBlock){
        assert(subBlock != nullptr);
        _subBlocks.push_back(subBlock);
        _sbOrder.push_back(lastOrder);
        lastOrder++;
    }



}
