//
// Created by tanawin on 25/2/2567.
//

#include "nest.h"
#include "model/controller/controller.h"
#include "sim/model_sim_engine/hw_component/expression/nest_sim.h"

namespace kathryn{


    /** pre function declaration*/

    std::vector<NestMeta> get_nest_vec(){
        return {};
    }
    std::vector<Operable*> get_nest_vec_read_only(){
        return {};
    }

    nest& make_nest_man(bool is_user, const std::vector<NestMeta>& grouped_meta){
        int nest_size = 0;
        for (NestMeta nest_meta: grouped_meta){
            nest_size += nest_meta.opr1->get_operable_slice().get_size();
        }
        return _make<nest>("uncatagorized_yet", "nest", is_user, nest_size, grouped_meta);
    }

    nest& make_nest_man_read_only(bool is_user,
        const std::vector<Operable*>& nest_list_read_only){
        int nest_size = 0;
        for(Operable* opr1: nest_list_read_only){
            assert(opr1 != nullptr);
            nest_size += opr1->get_operable_slice().get_size();
        }
        return _make<nest>("uncatagorized_yet", "nest", is_user, nest_size, nest_list_read_only);
    }

    /** nest class*/

    void nest::com_init() {
        ctrl->on_nest_init(this);
    }

    nest::nest(int size, std::vector<NestMeta> nest_list):
    LogicComp<nest>({0, size}, TYPE_NEST,
                    new NestSimEngine(this, VST_WIRE),
                    false),
    read_only(false),
    _nestList(std::move(nest_list))
    {
        int test_size = 0;
        for (NestMeta meta: _nestList){
            assert(meta.opr1 != nullptr);
            test_size += meta.opr1->get_operable_slice().get_size();
        }
        assert(test_size == size);
        com_init();
        AssignOpr::set_master(this);
        AssignCallbackFromAgent::set_master(this);
    }

    //////////// for read only element

    nest::nest(int size, const std::vector<Operable*>& nest_list_read_only):
    LogicComp<nest>({0, size}, TYPE_NEST,
                    new NestSimEngine(this, VST_WIRE),
                    false),
    read_only(true)
    {
        int test_size = 0;
        for (Operable* opr1: nest_list_read_only){
            assert(opr1 != nullptr);
            test_size += opr1->get_operable_slice().get_size();
            _nestList.push_back({opr1, nullptr});
        }
        assert(test_size == size);
        com_init();
        AssignOpr::set_master(this);
        AssignCallbackFromAgent::set_master(this);
    }


    /**
     *
     * blocking assignment
     *
     * */


    void nest::do_block_asm(Operable& src_opr, Slice des_slice) {
        assert(des_slice.get_size() <= get_slice().get_size());
        Slice actual_slice = des_slice.get_match_size_sub_slice(src_opr.get_operable_slice());
        std::vector<AssignMeta*> result_collector;
        /** get assign meta to build node*/
        do_block_asm(src_opr,
                   result_collector,
                   src_opr.get_operable_slice(),
                   actual_slice);
        /** basic node building*/
        auto* asm_node = new AsmNode(result_collector);
        /** update node*/
        ctrl->on_nest_update(asm_node, this);
    }

    void nest::do_non_block_asm(Operable &src_opr, Slice des_slice) {
        assert(des_slice.get_size() <= get_slice().get_size());
        Slice actual_slice = des_slice.get_match_size_sub_slice(src_opr.get_operable_slice());
        std::vector<AssignMeta*> result_collector;
        /** get assign meta to build node*/
        do_non_block_asm(src_opr,
                   result_collector,
                   src_opr.get_operable_slice(),
                  actual_slice);
        /** basic node building*/
        auto* asm_node = new AsmNode(result_collector);
        /** update node*/
        ctrl->on_nest_update(asm_node, this);
    }

    /** assign enforcer*/


    void nest::do_block_asm(Operable &src_opr, std::vector<AssignMeta *> &result_meta_collector,
                          Slice abs_src_slice, Slice abs_des_slice) {
        do_nest_global_asm(src_opr, result_meta_collector, abs_src_slice, abs_des_slice, true);
    }

    void nest::do_non_block_asm(Operable &src_opr, std::vector<AssignMeta *> &result_meta_collector,
                             Slice abs_src_slice, Slice abs_des_slice) {
        do_nest_global_asm(src_opr, result_meta_collector, abs_src_slice, abs_des_slice, false);
    }

    int nest::get_net_list_idx_that_match(int bit_idx) {
        assert(bit_idx > 0);
        int   start_acc = 0;
        for (int i = 0; i < _nestList.size(); i++){
            Operable* sub_opr = _nestList[i].opr1;
            assert(sub_opr != nullptr);
            /****/
            int sub_opr_size = sub_opr->get_operable_slice().get_size();
            if ( (start_acc + sub_opr_size) < bit_idx){
                start_acc += sub_opr_size;
            }else{
                return i;
            }
        }
        assert(false);
    }


    void
    nest::do_nest_global_asm(Operable& src_opr,
                          std::vector<AssignMeta *> &result_meta_collector,
                          Slice abs_src_slice,
                          Slice abs_des_slice,
                          bool isblocking_asm){
        mf_assert(!read_only, "this nest list is readonly mode");
        assert(abs_src_slice.get_size() >= abs_des_slice.get_size());
        assert(abs_src_slice.stop <= src_opr.get_operable_slice().get_size());
        assert(abs_des_slice.stop <= get_operable_slice().get_size());

        int des_iter_accum_bit = 0;
        int src_consummed_bit = 0;

        for (auto des_nest: _nestList){
            /** iterate to meet good point first*/
            auto des_opr = des_nest.opr1;
            auto des_asb= des_nest.asb;
            Slice cur_des_slice     = des_opr->get_operable_slice();
            int   cur_des_size      = cur_des_slice.get_size();
            /** if it is not desire destination continue next*/
            if (!abs_des_slice.is_intersec({des_iter_accum_bit, des_iter_accum_bit + cur_des_size})){
                des_iter_accum_bit += cur_des_size;
                continue;
            }

            /** get relative index of current destination*/
            int rel_des_start_bit = std::max(0         , abs_des_slice.start - des_iter_accum_bit);
            int rel_des_stop_bit  = std::min(cur_des_size, abs_des_slice.stop  - des_iter_accum_bit);
            /**convert to abs index of current destination*/
            Slice desire_des_slice = cur_des_slice.get_sub_slice({rel_des_start_bit, rel_des_stop_bit});

            /////// calculate src slice
                /**trim start bit*/
            Slice desire_src_slice = abs_src_slice.get_sub_slice(src_consummed_bit);
                /** trim stop bit*/
                  desire_src_slice = desire_src_slice.get_match_size_sub_slice(desire_des_slice);

            if (isblocking_asm){
                des_asb->do_block_asm(src_opr,
                                   result_meta_collector,
                                   desire_src_slice,
                                   desire_des_slice);
            }else{
                des_asb->do_non_block_asm(src_opr,
                                      result_meta_collector,
                                      desire_src_slice,
                                      desire_des_slice);
            }

            src_consummed_bit += desire_des_slice.get_size();
            des_iter_accum_bit += cur_des_size;

        }
    }
    /**********************************/

    SliceAgent<nest> &nest::operator()(int start, int stop) {
        auto ret =  new SliceAgent<nest>(this,get_abs_sub_slice(start, stop, get_slice()));
        return *ret;
    }

    SliceAgent<nest> &nest::operator()(int idx) {
        return operator()(idx, idx + 1);
    }

    SliceAgent<nest> &nest::operator()(Slice sl) {
        return operator()(sl.start, sl.stop);
    }

    Operable* nest::do_slice(Slice sl){
        auto& x = operator() (sl.start, sl.stop);
        return x.cast_to_operable();
    }

    Operable* nest::check_short_circuit() {
        if (is_in_check_path){
            return this;
        }

        is_in_check_path = true;

        for (NestMeta meta: _nestList){

            Operable* result = meta.opr1->check_short_circuit();
            if (result != nullptr){
                return result;
            }
        }

        is_in_check_path = false;
        return nullptr;
    }

    void nest::create_logic_gen(){
        _genEngine = new NestGen(
            _parent->get_module_gen_ptr(),
            this
        );
    }

}