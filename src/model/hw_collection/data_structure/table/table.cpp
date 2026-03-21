//
// Created by tanawin on 18/9/25.
//

#include "table.h"

#include "utility"

#include "gen/controller/gen_controller.h"
#include "model/controller/controller.h"

namespace kathryn{

    Table::Table(const Table& rhs){
        operator=(rhs);
    }

    Table::Table(const SlotMeta&  meta, const std::vector<RegSlot*>& rows):
        _meta(std::move(meta)),
        _rows(rows),
        _isMasterTable(false){
            mf_assert(!rows.empty(), "rows cannot be empty");
            for (RegSlot* row: rows){
                mf_assert(row != nullptr, "row cannot be nullptr");
            }
        }

    Table::Table(const SlotMeta&  slot_meta, int amt_row, const std::string& prefix_name):
        _meta(std::move(slot_meta)),
        _isMasterTable(true){
            mf_assert(amt_row > 0, "amt_row must be greater than 0");
            build_rows(_meta, amt_row, prefix_name);
        }

    Table::Table(const std::vector<std::string>& field_names,
              const std::vector<int>&         field_sizes,
              int amt_row,
              const std::string& prefix_name):
        _meta(field_names, field_sizes),
        _isMasterTable(true){
            mf_assert(amt_row > 0, "amt_row must be greater than 0");
            build_rows(_meta, amt_row, prefix_name);
        }

    Table::~Table(){
            if (_isMasterTable){
                for (RegSlot* row: _rows){
                    delete row;
                }
            }
        }


    Table::ReducNode Table::create_mux(ReducNode& lhs, ReducNode& rhs, Operable& select_left, int debug_idx, bool required_idx){

        Operable& select_right = ~select_left;

        ////// the cus_logic may augment lhs we have to debug it
        WireSlot* des_slot = new WireSlot(lhs.slot->get_meta());

        std::vector<AssignMeta*> left_ass_metas  = des_slot->gen_assign_meta_for_all(*lhs.slot, ASM_DIRECT);
        std::vector<AssignMeta*> right_ass_metas = des_slot->gen_assign_meta_for_all(*rhs.slot, ASM_DIRECT);

        //////// generate muxed as Assign meta data
        assert(left_ass_metas.size() == right_ass_metas.size());
        std::vector<AssignMeta*> muxed_assign_meta;
        for (int idx = 0; idx < left_ass_metas.size(); idx++){
            AssignMeta* left_ass_meta  = left_ass_metas[idx];
            AssignMeta* right_ass_meta = right_ass_metas[idx];
            AssignMeta* new_ass_meta = left_ass_meta->mux(right_ass_meta, &select_left);
            delete left_ass_meta;
            delete right_ass_meta;
            muxed_assign_meta.push_back(new_ass_meta);
        }


        auto* slot_asm_node = new AsmNode(muxed_assign_meta);
        slot_asm_node->dry_assign();
        delete slot_asm_node;

        Wire* selected_idx = nullptr;
        if (required_idx){
            assert(lhs.idx != nullptr);
            assert(rhs.idx != nullptr);
            int des_slice = lhs.idx->get_operable_slice().get_size();
            selected_idx = &make_opr_wire("reduc_opr" + std::to_string(debug_idx), lhs.idx->get_operable_slice().get_size());
            AssignMeta* left_asm_idx_meta  = selected_idx->generate_assign_meta(*lhs.idx, {0,  des_slice}, ASM_DIRECT, CM_CLK_FREE);
            AssignMeta* right_asm_idx_meta = selected_idx->generate_assign_meta(*rhs.idx, {0,  des_slice}, ASM_DIRECT, CM_CLK_FREE);
            AssignMeta* new_ass_meta      = left_asm_idx_meta->mux(right_asm_idx_meta, &select_left);
            delete left_asm_idx_meta;
            delete right_asm_idx_meta;

            auto* idx_asm_node = new AsmNode(new_ass_meta);
            idx_asm_node->dry_assign();
            delete idx_asm_node;

        }

        return {des_slot, selected_idx};

    }

    SlotMeta Table::get_meta() const{
        return _meta;
    }

    RegSlot& Table::get_ref_row(int idx){
        assert(is_valid_idx(idx));
        return *_rows[idx];
    }

    RegSlot Table::get_cloned_row(int idx) const{
        assert(is_valid_idx(idx));
        return *_rows[idx];
    }

    int Table::get_num_row() const{
        return static_cast<int>(_rows.size());
    }

    int Table::get_max_cell_width() const{
        assert(!_rows.empty());
        return _rows[0]->get_max_bit_width();
    }

    void Table::build_rows(SlotMeta& slot_meta, int amt_row, std::string prefix_name){
        for (int row_idx = 0; row_idx < amt_row; row_idx++){
            _rows.push_back(new RegSlot(slot_meta, prefix_name + "_" + std::to_string(row_idx)));
        }
    }

    bool Table::is_sufficient_bin_idx(Operable& required_idx) const{
        int input_size = required_idx.get_operable_slice().get_size();
        return (1 << input_size) >= get_num_row();
    }

    bool Table::is_sufficient_oh_idx(Operable& required_idx) const{
        return required_idx.get_operable_slice().get_size() == get_num_row();
    }

    bool Table::is_sufficient_idx(Operable& required_idx, bool is_oh) const{
        if (is_oh){
            return is_sufficient_oh_idx(required_idx);
        }
        return is_sufficient_bin_idx(required_idx);
    }

    int Table::get_sufficient_idx_size(bool is_oh) const{
        if (is_oh){
            return get_num_row();
        }
        return log2Ceil(get_num_row());
    }

    bool Table::is_valid_idx(int idx) const{
        return idx >= 0 && idx < _rows.size();
    }

    bool Table::check_valid_range(int start, int stop) const{
        return ( (start >= 0    ) && (start <  _rows.size()) ) &&
               ( (stop   >  start) && (stop   <= _rows.size()) );
    }

    Operable& Table::create_idx_match_cond(Operable& required_idx, int row_idx, bool is_oh){
        if (is_oh){
            return *required_idx.do_slice({row_idx, row_idx+1});
        }
        return (required_idx == row_idx);
    }

    /**
     * gen assign meta
     *
     */

    WireSlot Table::gen_dyn_wire_slotBase(Operable& required_idx, bool is_one_hot_idx){

        ////// return node
        WireSlot result_wire_slot(get_meta());
        //return result_wire_slot;
        ////// all metadata
        std::vector<AssignMeta*> all_row_collector;
        std::vector<Operable*> all_row_pre_cond;
        ////// generate all assign meta to all node
        int amt_row = get_num_row();
        for(int row_idx = 0; row_idx < get_num_row(); row_idx++){
            std::vector<AssignMeta*> each_row_collector;
            std::vector<Operable*> each_row_pre_cond;
            ////// generate the assign condition
            Operable* row_idx_check_cond = &create_idx_match_cond(required_idx, row_idx, is_one_hot_idx);
            //////// generate each row's assign meta
            each_row_collector = result_wire_slot.gen_assign_meta_for_all(*_rows[row_idx], ASM_DIRECT);
            for (int col_idx = 0; col_idx < result_wire_slot.get_num_field_ptr(); col_idx++){
                each_row_pre_cond.push_back(row_idx_check_cond);
            }
            /////// push it to pool system
            all_row_collector.insert(all_row_collector.end(),
            each_row_collector.begin(), each_row_collector.end());
            all_row_pre_cond.insert(all_row_pre_cond.end(),
            each_row_pre_cond.begin(), each_row_pre_cond.end());
        }
        ////// generate assignment Node
        AsmNode* asm_node = WireSlot::gen_grp_asm_node(all_row_collector, all_row_pre_cond);
        //// we have to do dry assign
        asm_node->dry_assign();
        delete asm_node;

        ///result_wire_slot.do_glob_asm(asm_node);
        return result_wire_slot;

    }

    WireSlot Table::gen_dyn_wire_slotBiIdx(Operable& required_idx){
        mf_assert(is_sufficient_bin_idx(required_idx), "required_idx is not sufficient to get all system");
        return gen_dyn_wire_slotBase(required_idx, false);
    }

    WireSlot Table::gen_dyn_wire_slotOHIdx(Operable& required_idx){
        mf_assert(is_sufficient_oh_idx(required_idx), "required_idx is not sufficient to get all system");
        return gen_dyn_wire_slotBase(required_idx, true);
    }

    ////// this will asssign the slot
    void Table::do_glob_asm(Slot& src_slot, Operable& required_idx, ASM_TYPE asm_type, bool is_one_hot_idx){
        mf_assert(is_sufficient_idx(required_idx, is_one_hot_idx), "required_idx is not sufficient to get all system");
        std::vector<AssignMeta*> all_row_collector;
        std::vector<Operable*>   all_row_pre_cond;

        /////// generate assign meta by row
        for (int des_idx = 0; des_idx < get_num_row(); des_idx++){
            /////// get related meta data
            std::vector<AssignMeta*> each_row_collector;
            std::vector<Operable*>   each_row_pre_cond;
            RegSlot& des_slot = get_ref_row(des_idx);
            SlotMeta src_meta = src_slot.get_meta();
            ////// seach for match assign column and generate assign Metadata
            auto  [src_matchidxs, des_match_idxs] = get_meta().match_by_name(src_meta);
            each_row_collector = des_slot.gen_assign_meta_for_all(src_slot, src_matchidxs, des_match_idxs, {}, asm_type);
            ////// generate condition for each row
            Operable* row_idx_check_cond = &(create_idx_match_cond(required_idx, des_idx, is_one_hot_idx));
            for (int col_idx = 0; col_idx < each_row_collector.size(); col_idx++){
                each_row_pre_cond.push_back(row_idx_check_cond);
            }
            /////// push it to pool system
            all_row_collector.insert(all_row_collector.end(),
                                   each_row_collector.begin(), each_row_collector.end());
            all_row_pre_cond.insert(all_row_pre_cond.end(),
                                 each_row_pre_cond.begin(), each_row_pre_cond.end());
        }

        /////// we have to create own asm node and push it directly to the system
        auto* asm_node = new AsmNode(all_row_collector);
        for (int idx = 0; idx < all_row_pre_cond.size(); idx++){
            asm_node->add_specific_pre_condition(all_row_pre_cond[idx], idx);
        }
        /////// we have to add it to controller by ourself
        ModelController* ctrl = get_controller_ptr();
        assert(ctrl != nullptr);
        ctrl->on_reg_update(asm_node, nullptr);

    }

    void Table::do_glob_asm(Operable& src_opr, Operable& row_idx, Operable& col_idx, ASM_TYPE asm_type, bool is_oh_row){
        mf_assert(is_sufficient_idx(row_idx, is_oh_row), "required_idx is not sufficient to get all system");
        mf_assert(_rows[0]->is_sufficient_idx(col_idx.get_operable_slice().get_size()), "column is not sufficient to get all column");

        std::vector<AssignMeta*> all_row_collector;
        std::vector<Operable*>   all_row_pre_cond;

        for (int des_row_idx = 0; des_row_idx < get_num_row(); des_row_idx++){
            /////// get related meta data
            std::vector<AssignMeta*> each_row_collector;
            std::vector<Operable*>   each_row_pre_cond;
            RegSlot& row_slot = get_ref_row(des_row_idx);

            ///// gen row assign
            each_row_collector = row_slot.gen_assign_meta_for_all(src_opr, asm_type);
            ///// gen col assign
            Operable* des_row_idx_check_cond = &(create_idx_match_cond(col_idx, des_row_idx, is_oh_row));
            for (int des_col_idx = 0; des_col_idx < each_row_collector.size(); des_col_idx++){
                each_row_pre_cond.push_back(
                    &( (*des_row_idx_check_cond) && (col_idx == des_col_idx)));
            }
            ///// add to main pool
            all_row_collector.insert(all_row_collector.end(),
                                   each_row_collector.begin(), each_row_collector.end());
            all_row_pre_cond.insert(all_row_pre_cond.end(),
            each_row_pre_cond.begin(), each_row_pre_cond.end());
        }

        ///// gen assign Node
        auto* asm_node = new AsmNode(all_row_collector);
        for (int idx = 0; idx < all_row_pre_cond.size(); idx++){
            asm_node->add_specific_pre_condition(all_row_pre_cond[idx], idx);
        }
        ///// add it to the whole main controller
        ModelController* ctrl = get_controller_ptr();
        assert(ctrl != nullptr);
        ctrl->on_reg_update(asm_node, nullptr);

    }

    void Table::do_glob_asm(ull rhs_val, Operable& row_idx, Operable& col_idx, ASM_TYPE asm_type, bool is_oh_row){
        Operable& my_src_opr = get_match_assign_operable(rhs_val, get_max_cell_width());
        do_glob_asm(my_src_opr, row_idx, col_idx, asm_type, is_oh_row);
    }

    Table& Table::do_glob_col_asm(int col_idx, ull assign_val){

        mf_assert(col_idx < get_meta().get_num_field_ptr(), "col_idx is out of range");
        ////// gen the value
        int required_size = get_meta().get_copy_field(col_idx)._size;
        m_val(av, required_size, assign_val);
        ////// asm_node meta
        std::vector<AssignMeta*> all_row_collector;

        ////// pool the node
        for (int des_row_idx = 0; des_row_idx < get_num_row(); des_row_idx++){
            RegSlot&    row_slot = get_ref_row(des_row_idx);
            AssignMeta* asm_meta = row_slot.gen_assign_meta(av, col_idx, ASM_DIRECT);
            all_row_collector.push_back(asm_meta);
        }
        ////// create asm_node
        auto* asm_node = new AsmNode(all_row_collector);
        ////// put it to the controller
        ModelController* ctrl = get_controller_ptr();
        assert(ctrl != nullptr);
        ctrl->on_reg_update(asm_node, nullptr);

        return *this;
    }

    Table& Table::do_glob_col_asm(const std::string& col_name, ull assign_val){
        int target_col_idx = _meta.get_idx_ptr(col_name);
        do_glob_col_asm(target_col_idx, assign_val);
        return *this;
    }


    void Table::do_cus_logic(std::function<void(RegSlot&, int row_idx)>  cus_logic){
        for (int row_idx = 0; row_idx < get_num_row(); row_idx++){
            cus_logic(*_rows[row_idx], row_idx);
        }
    }

    /////// reset event

    Table& Table::make_reset_event(ull reset_val, CLOCK_MODE cm){
        for (RegSlot* row : _rows){
            assert(row != nullptr);
            row->make_reset_event(reset_val, cm);
        }
        return *this;
    }

    Table& Table::make_col_reset_event(int col_idx, ull reset_val, CLOCK_MODE cm){
        for (RegSlot* row : _rows){
            assert(row != nullptr);
            row->make_reset_event(col_idx, reset_val, cm);
        }
        return *this;
    }

    Table& Table::make_col_reset_event(const std::string& col_name, ull reset_val,
                                    CLOCK_MODE cm){
        for (RegSlot* row : _rows){
            assert(row != nullptr);
            row->make_reset_event(col_name, reset_val, cm);
        }
        return *this;
    }

    Table::ReducNode Table::do_reduce_base(const std::vector<ReducNode>& init_reduc_nodes,
                                         const std::function<Operable&(WireSlot& lhs, Operable* lidx,
                                                                       WireSlot& rhs, Operable* ridx)>& cus_logic,
                                         bool required_idx){

        int debug_idx = 0;

        

        std::queue<ReducNode> reduc_queue_a;
        std::queue<ReducNode> reduc_queue_b;

        for (const ReducNode& node : init_reduc_nodes){
            reduc_queue_a.push(node);
        }

        std::queue<ReducNode>* src_reduc_queue = &reduc_queue_a;
        std::queue<ReducNode>* des_reduc_queue = &reduc_queue_b;

        while (src_reduc_queue->size() != 1){

            while(!src_reduc_queue->empty()){
                ////  if there is only one element by pass it
                if (src_reduc_queue->size() == 1){
                    des_reduc_queue->push(src_reduc_queue->front());
                    src_reduc_queue->pop();
                }

                //////get two node
                ReducNode src_node_left  = src_reduc_queue->front();
                src_reduc_queue->pop();
                ReducNode src_node_right = src_reduc_queue->front();
                src_reduc_queue->pop();

                //// get condition node
                Operable& select_left = cus_logic(*src_node_left.slot,  src_node_left.idx,
                                                *src_node_right.slot, src_node_right.idx);
                mf_assert(select_left.get_operable_slice().get_size() == 1, "select_left is not a single bit");
                ReducNode bined_reduc_node = create_mux(src_node_left, src_node_right, select_left, debug_idx++, required_idx);
                des_reduc_queue->push(bined_reduc_node);

                ///// free memory of the reducnode
                src_node_left.destroy();
                src_node_right.destroy();
            }
            std::swap(src_reduc_queue,des_reduc_queue);
        }

        assert(!src_reduc_queue->empty());

        return src_reduc_queue->front();

    }

    WireSlot Table::do_reduc_no_idx(const std::function<Operable&(WireSlot& lhs, Operable* lidx,
                                                         WireSlot& rhs, Operable* ridx)>& cus_logic){
        std::vector<ReducNode> init_reduc_nodes;
        init_reduc_nodes.reserve(get_num_row());
        for (int row_idx = 0; row_idx < get_num_row(); row_idx++){
            init_reduc_nodes.push_back({new WireSlot(*static_cast<Slot*>(_rows[row_idx]), "init_reduc"), nullptr});
        }
        ReducNode final_node = do_reduce_base(init_reduc_nodes, cus_logic, false);
        WireSlot result(*final_node.slot);
        final_node.destroy();
        return result;
    }

    std::pair<WireSlot, Operable&> Table::do_reduc_bin_idx(const std::function<Operable&(WireSlot& lhs, Operable* lidx,
                                                                                      WireSlot& rhs, Operable* ridx)>& cus_logic){
        std::vector<ReducNode> init_reduc_nodes;
        for (int row_idx = 0; row_idx < get_num_row(); row_idx++){
            Val* idx_val = &make_opr_val("init_bin_idx_opr" + std::to_string(row_idx), get_sufficient_idx_size(false), row_idx);
            init_reduc_nodes.push_back({new WireSlot(*static_cast<Slot*>(_rows[row_idx]), "init_reduc_bin"), idx_val});
        }
        ReducNode final_node = do_reduce_base(init_reduc_nodes, cus_logic, true);
        WireSlot result(*final_node.slot);
        Operable& result_idx = *final_node.idx;
        final_node.destroy();
        return {result, result_idx};
    }

    std::pair<WireSlot, OH> Table::do_reduc_oh_idx(const std::function<Operable&(WireSlot& lhs, Operable* lidx,
                                                                              WireSlot& rhs, Operable* ridx)>& cus_logic){
        std::vector<ReducNode> init_reduc_nodes;
        for (int row_idx = 0; row_idx < get_num_row(); row_idx++){
            Val* idx_val = &make_opr_val("init_oh_idx_opr" + std::to_string(row_idx), get_sufficient_idx_size(true), ((ull) 1) << row_idx);
            init_reduc_nodes.push_back({new WireSlot(*static_cast<Slot*>(_rows[row_idx]), "init_reduc_oh"), idx_val});
        }
        ReducNode final_node = do_reduce_base(init_reduc_nodes, cus_logic, true);
        WireSlot result(*final_node.slot);
        Operable& result_idx = *final_node.idx;
        final_node.destroy();
        return {result, OH(result_idx)};
    }



    Table::ReducNode Table::find_matched_ordered(
        bool is_newest,
        const std::vector<ReducNode>& init_reduc_nodes,
        bool required_idx){

        //// key words to compare   user_valid_compare, system_in_old_seq
        ReducNode result =
            do_reduce_base(init_reduc_nodes,
                         [&](WireSlot& lhs, Operable* lidx, WireSlot& rhs, Operable* ridx)-> opr&{

                             /**
                              *  |
                              *  |------ newest
                              *  |------ start Ptr oldest
                              *  |------
                              *  |------
                              */

                             ////// newst
                             if (is_newest){
                                 return (lhs(ORDERED_USER_VALID_KW) &  (~rhs(ORDERED_USER_VALID_KW)))    || ///// ordinary case

                                       (  lhs(ORDERED_USER_VALID_KW)      & rhs(ORDERED_USER_VALID_KW) & /// if both equal select left only right is in old region and we in newer region
                                        (~lhs(ORDERED_SYSTEM_SEQ_OLD_KW)) & rhs(ORDERED_SYSTEM_SEQ_OLD_KW));
                             }
                             ////// oldest
                             return (lhs(ORDERED_USER_VALID_KW) &  (~rhs(ORDERED_USER_VALID_KW)))    || ///// ordinary case
                                    (
                                     lhs(ORDERED_USER_VALID_KW) & rhs(ORDERED_USER_VALID_KW) & /// if both equal select left only right is in old region
                                    (lhs(ORDERED_SYSTEM_SEQ_OLD_KW) == rhs(ORDERED_SYSTEM_SEQ_OLD_KW))
                                    ); //// left only when it is in the same region


                         },
                        required_idx);
        return result;

    }

    WireSlot* Table::augment_for_ordered_search(int row_idx,
                                            Operable& OldestStartIndex,
                                            const std::function<Operable&(RegSlot& src)>& user_valid_func){

        assert(row_idx < get_num_row());
        auto* result =
        new WireSlot(*static_cast<Slot*>(_rows[row_idx]), "augOldest_" + std::to_string(row_idx));
        ////// do augment wire
        result->add_wire(ORDERED_USER_VALID_KW, user_valid_func(*_rows[row_idx]));
        result->add_wire(ORDERED_SYSTEM_SEQ_OLD_KW,  OldestStartIndex <= row_idx);
        return result;

    }

    std::pair<WireSlot, Operable&>
        Table::findMBO_BIDX(bool is_newest,
                            Operable& oldest_start_index,
                            const std::function<Operable&(RegSlot& src)>& user_valid_func){

        assert(is_sufficient_bin_idx(oldest_start_index));

        std::vector<ReducNode> init_reduc_nodes;
        for (int row_idx = 0; row_idx < get_num_row(); row_idx++){
            Val*      idx_val  = &make_opr_val("init_bin_idx_opr" + std::to_string(row_idx), get_sufficient_idx_size(false), row_idx);
            WireSlot* aug_slot = augment_for_ordered_search(row_idx, oldest_start_index, user_valid_func);
            init_reduc_nodes.push_back({aug_slot, idx_val});
        }
        ReducNode final_node = find_matched_ordered(is_newest, init_reduc_nodes, true);
        WireSlot result(*final_node.slot);
        Operable& result_idx = *final_node.idx;
        final_node.destroy();
        ///// we have to delete the augmented filed
        return {result(0, get_meta().get_num_field_ptr()), result_idx};

    }


    std::pair<WireSlot, OH>
        Table::findMBO_OHIDX(bool is_newest,
                                       Operable& oldest_start_index,
                                       const std::function<Operable&(RegSlot& src)>& user_valid_func){

        assert(is_sufficient_bin_idx(oldest_start_index));

        std::vector<ReducNode> init_reduc_nodes;
        for (int row_idx = 0; row_idx < get_num_row(); row_idx++){
            Val*      idx_val  = &make_opr_val("init_oh_idx_opr" + std::to_string(row_idx), get_sufficient_idx_size(true), ((ull) 1) << row_idx);
            WireSlot* aug_slot = augment_for_ordered_search(row_idx, oldest_start_index, user_valid_func);
            init_reduc_nodes.push_back({aug_slot, idx_val});
        }
        ReducNode final_node = find_matched_ordered(is_newest, init_reduc_nodes, true);
        WireSlot result(*final_node.slot);
        Operable& result_idx = *final_node.idx;
        final_node.destroy();
        ///// we have to delete the augmented filed
        return {result(0, get_meta().get_num_field_ptr()-2), OH(result_idx)};

    }



    /**
     * static slicing
     */
    RegSlot& Table::operator () (int idx){
        mf_assert(is_valid_idx(idx), "index out of range to get " + std::to_string(idx));
        return *_rows[idx];
    }

    Table Table::operator() (int start, int end){

        mf_assert(check_valid_range(start, end), "invalid range to get");
        std::vector<RegSlot*> new_rows;
        for (int idx = start; idx < end; idx++){
            new_rows.push_back(_rows[idx]);
        }
        return Table(_meta, new_rows);

    }

    Table& Table::operator = (const Table& rhs){
        _meta = rhs._meta;
        _rows = rhs._rows;
        _isMasterTable = false;
        return *this;
    }

    Table Table::slice_by_col(int start, int end){
        SlotMeta new_slot_meta = _meta(start, end);
        std::vector<RegSlot*> new_rows;
        for (int row_idx = 0; row_idx < get_num_row(); row_idx++){
            RegSlot new_reg_slot = ((*_rows[row_idx])(start, end));
            new_rows.push_back(new RegSlot(new_reg_slot));
        }
        return {new_slot_meta, new_rows};
    }
        //// treated as base col slice function
    Table Table::slice_by_col(const std::string& start_field, const std::string& end_field){
        int start_idx = get_meta().get_idx_ptr(start_field);
        int end_idx   = get_meta().get_idx_ptr(end_field) + 1;
        mf_assert(get_meta().is_valid_idx(start_idx), "field name " + start_field + " not found");
        mf_assert(get_meta().is_valid_idx(end_idx), "field name " + end_field + " not found");
        return slice_by_col(start_idx, end_idx);
    }
    Table Table::slice_by_col(const std::vector<int>& field_idxs){
        SlotMeta new_slot_meta = _meta(field_idxs);
        std::vector<RegSlot*> new_rows;
        for (int row_idx = 0; row_idx < get_num_row(); row_idx++){
            RegSlot new_reg_slot = (*_rows[row_idx])(field_idxs);
            new_rows.push_back(new RegSlot(new_reg_slot));
        }
        return {new_slot_meta, new_rows};
    }
    Table Table::slice_by_col(const std::vector<std::string>& field_names){
        std::vector<int> field_idxs = get_meta().get_idx_ptrs(field_names);
        return slice_by_col(field_idxs);
    }



    /**
     * dynamic
     */

    TableSliceAgent Table::operator[] (Operable& required_idx){
        is_sufficient_bin_idx(required_idx);
        return TableSliceAgent(this, required_idx, false);
    }

    TableSliceAgent Table::operator[] (OH oh_idx){
        is_sufficient_oh_idx(oh_idx.get_idx_ptr());
        return TableSliceAgent(this, oh_idx.get_idx_ptr(), true);
    }

    /**
     *  table join
     *
     */

    Table Table::join_table_by_row(const Table& rhs){
        SlotMeta rhs_meta = rhs.get_meta();
        SlotMeta new_meta = get_meta();
        mf_assert(new_meta == rhs_meta, "slot meta is not match");

        ////// new row
        std::vector<RegSlot*> new_rows = _rows;
        new_rows.insert(new_rows.end(), rhs._rows.begin(), rhs._rows.end());

        return Table(new_meta, new_rows);

    }

    Table Table::join_table_by_row_interleave(const Table& rhs){
        //////// prequisite check
        SlotMeta rhs_meta = rhs.get_meta();
        SlotMeta new_meta = get_meta();
        mf_assert(new_meta == rhs_meta, "slot meta is not match");
        int cur_amt_row = get_num_row();
        int rhs_amt_row = rhs.get_num_row();
        mf_assert(cur_amt_row == rhs_amt_row, "row size is not match");

        ////// new row
        std::vector<RegSlot*> new_rows;
        for (int i = 0; i < get_num_row(); i++){
            new_rows.push_back(_rows[i]);
            new_rows.push_back(rhs._rows[i]);
        }
        return {new_meta, new_rows};

    }

    Table Table::join_table_by_col(const Table& rhs){
        mf_assert(get_num_row() == rhs.get_num_row(), "row size is not match");

        std::vector<RegSlot*> new_rows;

        for (int row_idx = 0; row_idx < get_num_row(); row_idx++){
            RegSlot  new_reg_slot = get_ref_row(row_idx) + rhs.get_cloned_row(row_idx);
            RegSlot* cloned_slot = new RegSlot(new_reg_slot);
            new_rows.push_back(cloned_slot);
        }

        return Table(get_meta(), new_rows);

    }


    Table Table::join(const Table& rhs,  int axis){
        mf_assert(axis >= 0 && axis <= 1, "axis must be 0 or 1");
        mf_assert(false, "not implemented yet");

        switch (axis){
            case 0:
                return join_table_by_row(rhs);
            case 1:
                return join_table_by_col(rhs);
            default:
                assert(false);
        }

    }

}