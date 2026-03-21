//
// Created by tanawin on 19/2/2567.
//

#include "mem_block_agent.h"
#include "mem_block.h"
#include "model/hw_component/abstract/make_component.h"
#include "model/controller/controller.h"
#include "sim/model_sim_engine/hw_component/mem_blk/mem_agent_sim.h"

namespace kathryn{

    MemBlockEleHolder::MemBlockEleHolder(MemBlock *master, const Operable* indexer):
            LogicComp<MemBlockEleHolder>({0, master->get_width_size()},
                                         TYPE_MEM_BLOCK_INDEXER,
                                         new MemEleHolderSimEngine(this),
                                         false
                                          ),
            read_mode(true),
            _master(master),
            _indexer(const_cast<Operable*>(indexer)){
        assert(_master  != nullptr);
        assert(_indexer != nullptr);
        ////std::cout << _indexer->get_operable_slice().get_size() << "    actual " << get_exact_index_size() << std::endl;
        assert(_indexer->get_operable_slice().get_size()
               == get_exact_index_size());
        AssignOpr::set_master(this);
        AssignCallbackFromAgent::set_master(this);
    }


    MemBlockEleHolder::MemBlockEleHolder(MemBlock * master, const int idx):
            LogicComp<MemBlockEleHolder>({0, master->get_width_size()},
                                         TYPE_MEM_BLOCK_INDEXER,
                                         new MemEleHolderSimEngine(this),
                                         false
            ),
            read_mode(true),
            _master(master),
            _indexer(nullptr)
    {
        make_val(mem_indexer, get_exact_index_size(), idx);
        _indexer = (Operable*)(&mem_indexer);
        assert(_master != nullptr);
        assert(_indexer!= nullptr);

        AssignOpr::set_master(this);
        AssignCallbackFromAgent::set_master(this);
    }

    int
    MemBlockEleHolder::get_exact_index_size(){
        assert(_master != nullptr);
        return log2Ceil(_master->get_depth_size());
    }


    /**
     * standard assignment
     *
     * ***/


    void MemBlockEleHolder::do_block_asm(Operable &src_opr, Slice des_slice) {
        do_global_asm(src_opr, des_slice, ASM_DIRECT);
    }

    void MemBlockEleHolder::do_non_block_asm(Operable& src_opr, Slice des_slice) {
        do_global_asm(src_opr, des_slice, ASM_EQ_DEPNODE);
    }

    void MemBlockEleHolder::do_global_asm(Operable& src_opr, Slice des_slice, ASM_TYPE asm_type){
        mf_assert(get_assign_mode() == AM_MOD, "must be Model mode only");
        mf_assert(is_read_mode(), "duplicate write operation");
        mf_assert(get_slice().get_size() == src_opr.get_operable_slice().get_size(),
                 "invalid write size");
        set_write_mode();
        assert(src_opr.get_operable_slice().get_size() == _master->get_width_size());
        ctrl->on_memBlkEleHolder_update(
                generate_basic_node(src_opr,
                                  {0, _master->get_width_size()},
                                          asm_type),
                this);
    }

    /**
     * special group assignment
     *
     * ***/

    void MemBlockEleHolder::do_block_asm(Operable& src_opr,
                                       std::vector<AssignMeta*>& result_meta_collector,
                                       Slice  abs_src_slice,
                                       Slice  abs_des_slice){
        /** for assigning in memblock must have same size */
        mf_assert(is_read_mode(), "duplicate write operation");
        mf_assert(get_slice().get_size() == abs_src_slice.get_size(),
                 "invalid write size");
        set_write_mode();
        Assignable::do_global_asm(src_opr, result_meta_collector,
                                   abs_src_slice, abs_des_slice,
                                   ASM_DIRECT);
    }

    void MemBlockEleHolder::do_non_block_asm(Operable& src_opr,
                                          std::vector<AssignMeta*>& result_meta_collector,
                                          Slice  abs_src_slice,
                                          Slice  abs_des_slice){
        /** for assigning in memblock must have same size */
        mf_assert(is_read_mode(), "duplicate write operation");
        mf_assert(get_slice().get_size() == abs_src_slice.get_size(),
                 "invalid write size");
        set_write_mode();
        Assignable::do_global_asm(src_opr, result_meta_collector,
                                   abs_src_slice, abs_des_slice,
                                   ASM_EQ_DEPNODE);
    }



    /*** override Operable*/
    Identifiable* MemBlockEleHolder::cast_to_ident() {return this;}

    /***slicable*/

    SliceAgent<MemBlockEleHolder>&
    MemBlockEleHolder::operator()(int start, int stop) {
        mf_assert(is_read_mode(), "mem holder is used for write mode");
        set_read_mode();
        auto ret = new SliceAgent<MemBlockEleHolder>(this,
                                                     get_abs_sub_slice(start, stop,get_slice())
                                                     );
        return *ret;
    }

    SliceAgent<MemBlockEleHolder> &MemBlockEleHolder::operator()(int idx) {
        return operator() (idx, idx+1);
    }

    SliceAgent<MemBlockEleHolder> &MemBlockEleHolder::operator()(Slice sl) {
        return operator() (sl.start, sl.stop);
    }

    Operable* MemBlockEleHolder::do_slice(Slice sl){
        auto& x = operator() (sl.start, sl.stop);
        return x.cast_to_operable();
    }

    void MemBlockEleHolder::create_logic_gen(){
        _genEngine = new MemEleholderGen(
            _parent->get_module_gen_ptr(),
            this
        );
    }


}