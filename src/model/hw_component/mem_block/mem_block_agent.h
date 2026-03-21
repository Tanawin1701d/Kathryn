//
// Created by tanawin on 19/2/2567.
//

#ifndef KATHRYN_MEMBLOCKAGENT_H
#define KATHRYN_MEMBLOCKAGENT_H


#include "model/hw_component/abstract/operable.h"
#include "model/hw_component/abstract/assignable.h"
#include "model/hw_component/abstract/slicable.h"
#include "sim/model_sim_engine/hw_component/abstract/logic_sim_engine.h"
#include "model/hw_component/abstract/logic_comp.h"
#include "gen/proxy_hw_comp/mem_block/mem_agent_gen.h"

namespace kathryn{


    class MemBlock;

    /**this class is used to hold only one element in each memblock*/
    class MemBlockEleHolder: public LogicComp<MemBlockEleHolder>{
        friend class MemEleHolderSimEngine;
        friend class MemEleholderGen;
    private:
        ///bool set_mode_yet = false; /**the goal of set_mode is to prevent duplicate read write in the same index*/
        bool read_mode = true; /**Therefore, we should know that if it did not set mode it may be read mode*/

        MemBlock* _master  = nullptr;
        Operable* _indexer = nullptr;

    protected:
        void set_read_mode (){read_mode = true; }
        void set_write_mode(){read_mode = false;}

        ////[[nodiscard]] bool is_set_mode   () const{return set_mode_yet;}

    public:
        explicit MemBlockEleHolder(MemBlock* master, const Operable* indexer);
        explicit MemBlockEleHolder(MemBlock* master, int idx);

        void com_init () override{};
        void com_final() override{};

        bool is_read_mode () const{return read_mode;}
        bool is_write_mode() const{return !read_mode;}

        int     get_exact_index_size();

        /** override assignable (need to call controller)*/
        void do_block_asm (Operable& src_opr, Slice des_slice) override;
        void do_non_block_asm(Operable& src_opr, Slice des_slice) override;
        void do_global_asm(Operable& src_opr, Slice des_slice, ASM_TYPE asm_type) override;

        void do_block_asm(Operable& src_opr,
                        std::vector<AssignMeta*>& result_meta_collector,
                        Slice  abs_src_slice,
                        Slice  abs_des_slice) override;

        void do_non_block_asm(Operable& src_opr,
                           std::vector<AssignMeta*>& result_meta_collector,
                           Slice  abs_src_slice,
                           Slice  abs_des_slice) override;

        CLOCK_MODE get_cur_assign_clk_mode() override {return GET_CLOCK_MODE();}

        MemBlockEleHolder& operator = (Operable& b)         { operator_eq(b);                                return *this;}
        MemBlockEleHolder& operator = (ull b)               { operator_eq(b);                                   return *this;}
        MemBlockEleHolder& operator = (MemBlockEleHolder& b){ if (this == &b){return *this;} operator_eq(b); return *this;}

        /** Operable*/
        Identifiable*   cast_to_ident         () override;

        /** Slicable*/
        SliceAgent<MemBlockEleHolder>& operator() (int start, int stop) override;
        SliceAgent<MemBlockEleHolder>& operator() (int idx) override;
        SliceAgent<MemBlockEleHolder>& operator() (Slice sl) override;
        Operable* do_slice(Slice sl) override;
        /** check short circuit*/
        Operable* check_short_circuit   () override{return nullptr;}

        /** debug method to do will will make debug string more delightful*/
        std::string get_md_describe() override {return Identifiable::get_ident_debug_value();}
        std::string get_md_ident_val() override {return Identifiable::get_ident_debug_value();}

        Operable*get_indexer_ptr(){ assert(_indexer != nullptr); return _indexer;}
        MemBlock*get_master_mem_blk_ptr(){assert(_master != nullptr); return _master;}

        /**override logicc gen base*/
        void create_logic_gen() override;

    };






}

#endif //KATHRYN_MEMBLOCKAGENT_H
