//
// Created by tanawin on 19/7/2024.
//

#include "code_base_writer.h"

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

    void CbBase::add_st(std::string stm, bool ln){ //// ln = new line
        _codeSt.push_back({
            std::move(stm), last_order, ln, false
        });
        last_order++;
    }

    void CbBase::add_cm(const std::string& comment){
        _codeSt.push_back({
            std::move(comment), last_order, true, true
        });
        last_order++;
    }

    void CbBase::append_sub_block(CbBase* sub_block){
        assert(sub_block != nullptr);
        _subBlocks.push_back(sub_block);
        _sbOrder.push_back(last_order);
        last_order++;
    }



}
