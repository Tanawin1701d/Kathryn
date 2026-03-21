//
// Created by tanawin on 9/12/25.
//

#ifndef MODEL_HWCOLLECTION_DATASTRUCTURE_MUX_MUX_H
#define MODEL_HWCOLLECTION_DATASTRUCTURE_MUX_MUX_H
#include "model/flow_block/abstract/nodes/asm_node.h"
#include "model/hw_component/abstract/operable.h"
#include "model/hw_component/wire/wire.h"

namespace kathryn{


    inline Operable& mux(Operable& sel, Operable& x0, Operable& x1){

        assert(sel.get_operable_slice().get_size() == 1);
        assert(x0.get_operable_slice().get_size() == x1.get_operable_slice().get_size());
        int destinated_width = x0.get_operable_slice().get_size();
        assert(destinated_width > 0);

        Wire& result = m_opr_wire("mux2", destinated_width);

        AssignMeta* sel0 =
        result.generate_assign_meta(x0, result.get_operable_slice(), ASM_DIRECT, CM_CLK_FREE);
        AssignMeta* sel1 =
        result.generate_assign_meta(x1, result.get_operable_slice(), ASM_DIRECT, CM_CLK_FREE);

        AsmNode* ass_node = new AsmNode({sel0, sel1});
        ass_node->add_specific_pre_condition(&(sel == 1), 0); ///// for x0
        ass_node->add_specific_pre_condition(&(sel == 0), 1); ///// for x1
        ass_node->dry_assign();
        delete ass_node;

        return result;


    }

    inline Operable& mux(Operable& sel, const std::vector<Operable*>& srcs){

        int index_size   = sel.get_operable_slice().get_size();
        int capable_size = 1 << index_size;
        assert(index_size >= 1);
        mf_assert(capable_size == srcs.size(), "the input amount should be 2^(sel)");

        //// initialize queue
        std::queue<Operable*> mux_queue;
        for (int idx = 0; idx < capable_size; idx++){
            assert(srcs[idx] != nullptr);
            mux_queue.push(srcs[idx]);
        }

        //// generate mux
        int bit_idx = 0;
        while (mux_queue.size() > 1){
            int size = mux_queue.size();
            for (int piece = 0; piece < size; piece+=2){
                Operable* first   = mux_queue.front(); mux_queue.pop();
                Operable* second  = mux_queue.front(); mux_queue.pop();
                Operable& next_opr = mux(*sel.do_slice({bit_idx, bit_idx+1}), *second, *first);
                mux_queue.push(&next_opr);
            }
            bit_idx +=1;
        }

        assert(mux_queue.size() == 1);
        return *mux_queue.front();
    }



}

#endif //MODEL_HWCOLLECTION_DATASTRUCTURE_MUX_MUX_H