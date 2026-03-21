//
// Created by tanawin on 15/9/25.
//

#ifndef KATHRYN_SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLE_H
#define KATHRYN_SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLE_H

#include "model/hw_collection/data_structure/slot/reg_slot.h"
#include "model/hw_collection/data_structure/slot/slot_meta.h"
#include "model/hw_collection/data_structure/slot/wire_slot.h"
#include "model/hw_collection/data_structure/indexing/index.h"
#include "table_slice_agent.h"

namespace kathryn{



    class Table{

    static constexpr char ORDERED_USER_VALID_KW[] = "user_valid_compare";
    static constexpr char ORDERED_SYSTEM_SEQ_OLD_KW[] = "system_in_oldest_sec";

    protected:
        SlotMeta _meta;
        std::vector<RegSlot*> _rows;
        bool     _isMasterTable = true; //// does it is the table that produce reg_slot

        struct ReducNode{
            WireSlot* slot = nullptr; Operable* idx{};
            void destroy() const{
                delete slot;
                //// operable is managed by module
            }
        };

        ReducNode create_mux(ReducNode& lhs, ReducNode& rhs, Operable& select_left, int debug_idx, bool required_idx);


    public:

        /**
         * this is used to initialize from slice
         */
        Table(const Table& rhs);
        Table(const SlotMeta&  meta, const std::vector<RegSlot*>& rows);

        Table(const SlotMeta&  slot_meta, int amt_row, const std::string& prefix_name = "table");

        Table(const std::vector<std::string>& field_names,
              const std::vector<int>&         field_sizes,
              int amt_row,
              const std::string& prefix_name);

        ~Table();


        /**
         * get the static data
         */
        [[nodiscard]] SlotMeta get_meta() const;
                      RegSlot& get_ref_row(int idx);
        [[nodiscard]] RegSlot  get_cloned_row(int idx) const;
        [[nodiscard]] int      get_num_row() const;
                      int      get_max_cell_width() const;

        /**
         *  build the reg slot for each row
         */
        void build_rows(SlotMeta& slot_meta, int amt_row, std::string prefix_name);

        /**
         * check indexing size
         */
        bool is_sufficient_bin_idx(Operable& required_idx) const;
        bool is_sufficient_oh_idx(Operable& required_idx) const;
        bool is_sufficient_idx(Operable& required_idx, bool is_oh) const;
        int  get_sufficient_idx_size(bool is_oh) const;

        bool is_valid_idx(int idx) const;
        bool check_valid_range(int start, int stop) const;

        Operable& create_idx_match_cond(Operable& required_idx, int row_idx,bool is_oh);



        /**
         * gen assign meta
         *
         */
        WireSlot gen_dyn_wire_slotBase(Operable& required_idx, bool is_one_hot_idx); //// one_hot_idx will determine the the row_id by bit Idx in the required_idx
        WireSlot gen_dyn_wire_slotBiIdx(Operable& bin_idx);
        WireSlot gen_dyn_wire_slotOHIdx(Operable& oh_idx);

        ////// this will asssign the slot
        void   do_glob_asm   (Slot& src_slot   , Operable& required_idx, ASM_TYPE asm_type, bool is_one_hot_idx);
        void   do_glob_asm   (Operable& src_opr, Operable& row_idx, Operable& col_idx, ASM_TYPE asm_type, bool is_one_hot_idx);
        void   do_glob_asm   (ull       src_val, Operable& row_idx, Operable& col_idx, ASM_TYPE asm_type, bool is_one_hot_idx);
        ////// asm_type is lock to
        Table& do_glob_col_asm(int   col_idx              , ull assign_val = 0);
        Table& do_glob_col_asm(const std::string& col_name, ull assign_val = 0);


        void do_cus_logic(std::function<void(RegSlot&, int row_idx)>  cus_logic);

        ////// make reset_event will do when glo
        Table& make_reset_event    (ull   reset_val                              , CLOCK_MODE cm = CM_POSEDGE);
        Table& make_col_reset_event (int   col_idx              , ull reset_val = 0, CLOCK_MODE cm = CM_POSEDGE);
        Table& make_col_reset_event (const std::string& col_name, ull reset_val = 0, CLOCK_MODE cm = CM_POSEDGE);


        /////// reduction operation

        ReducNode do_reduce_base(const std::vector<ReducNode>& init_reduc_nodes,
                               const std::function<Operable&(WireSlot& lhs, Operable* lidx,
                                                       WireSlot& rhs, Operable* ridx)>& cus_logic,
                               bool required_idx);
        WireSlot do_reduc_no_idx(const std::function<Operable&(WireSlot& lhs, Operable* lidx,
                                                      WireSlot& rhs, Operable* ridx)>& cus_logic);
        std::pair<WireSlot, Operable&> do_reduc_bin_idx(const std::function<Operable&(WireSlot& lhs, Operable* lidx,
                                                                             WireSlot& rhs, Operable* ridx)>& cus_logic);
        std::pair<WireSlot, OH> do_reduc_oh_idx(const std::function<Operable&(WireSlot& lhs, Operable* lidx,
                                                                     WireSlot& rhs, Operable* ridx)>& cus_logic);

        ReducNode find_matched_ordered(bool is_newest,
                                    const std::vector<ReducNode>& init_reduc_nodes,
                                    bool required_idx);

        WireSlot* augment_for_ordered_search(int row_idx,
                                         Operable& OldestStartIndex,
                                         const std::function<Operable&(RegSlot& src)>& user_valid_func);

        std::pair<WireSlot, Operable&>
        findMBO_BIDX(
            bool is_newest, ///// otherwise it is oldest
            Operable& oldest_start_index,
            const std::function<Operable&(RegSlot& src)>& user_valid_func);

        std::pair<WireSlot, OH>
        findMBO_OHIDX(
            bool is_newest,
            Operable& oldest_start_index,
            const std::function<Operable&(RegSlot& src)>& user_valid_func);

        /**
         * static slicing
         */
        RegSlot& operator () (int idx);

        Table operator() (int start, int end);

        Table& operator = (const Table& rhs);

        Table slice_by_col(int start, int end);  //// treated as base col slice function
        Table slice_by_col(const std::string& start_field, const std::string& end_field);
        Table slice_by_col(const std::vector<int>& field_idxs); //// treated as base col slice function
        Table slice_by_col(const std::vector<std::string>& field_names);

        /**
         * dynamic
         */

        TableSliceAgent operator[] (Operable& required_idx);
        TableSliceAgent operator[] (OH oh_idx);

        /**
         *  table join
         *
         */

        Table join_table_by_row(const Table& rhs);
        Table join_table_by_row_interleave(const Table& rhs);

        Table join_table_by_col(const Table& rhs);

        Table join(const Table& rhs,  int axis);

    };

}

#endif //KATHRYN_SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLE_H
