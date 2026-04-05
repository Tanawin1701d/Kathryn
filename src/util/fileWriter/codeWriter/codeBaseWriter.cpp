//
// Created by tanawin on 19/7/2024.
//

#include "codeBaseWriter.h"

#include "utility"

namespace kathryn{
    /**
     *
     * cb if base
     *
     */
    CbBase::~CbBase(){
        for (CbBase* cb: _subBlocks){
            delete cb;
        }
    }

    void CbBase::addSt(std::string stm, bool ln){ //// ln = new line
        _codeSt.pushBack({
            std::move(stm), lastOrder, ln, false
        });
        lastOrder++;
    }

    void CbBase::addCm(const std::string& comment){
        _codeSt.pushBack({
            std::move(comment), lastOrder, true, true
        });
        lastOrder++;
    }

    void CbBase::appendSubBlock(CbBase* subBlock){
        assert(subBlock != nullptr);
        _subBlocks.pushBack(subBlock);
        _sbOrder.pushBack(lastOrder);
        lastOrder++;
    }



}
