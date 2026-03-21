//
// Created by tanawin on 10/4/2567.
//

#include "box.h"
#include "model/controller/controller.h"

namespace kathryn{


    Box::Box(): HwCompControllerItf(true),
                Identifiable(TYPE_BOX)
    {
        com_init();
    }

    void Box::com_init() {
        ctrl->on_box_init(this);
    }

    void Box::com_final() {
        ctrl->on_box_end_init(this);
    }

    void Box::add_nest_meta(NestMeta nest_meta) {
        i++;
        if (i > 5){
            assert(false);
        }
        _nestMetas.push_back(nest_meta);
    }

    void Box::add_sub_box(Box* sub_box) {
       ///// add recurrent
       assert(sub_box != nullptr);
       assert(sub_box != this);
       _recurBoxs.push_back(sub_box);
    }

    std::vector<NestMeta>& Box::get_nest_metas(){
        return _nestMetas;
    }
    std::vector<Box*>& Box::get_sub_box(){
        return _recurBoxs;
    }

    void Box::collect_assign_meta(Box& rhs_box,
                                 bool is_block_asm,
                                 std::vector<AssignMeta*>& result_collector) {

        mf_assert(_nestMetas.size() == rhs_box.get_nest_metas().size(),
                 "box mismatch");

        mf_assert(_recurBoxs.size() == rhs_box.get_sub_box().size(),
                 "subblock mismatch"
                 );

        /** do element by element assignment*/

        for (size_t idx = 0; idx < _nestMetas.size(); idx++){
            Assignable* lhs_asb = _nestMetas[idx].asb;
            Operable*   rhs_opr = rhs_box.get_nest_metas()[idx].opr1;
            if (is_block_asm) {
                _nestMetas[idx].asb->do_block_asm(
                        *rhs_opr,
                        result_collector,
                        rhs_opr->get_operable_slice(),
                        lhs_asb->get_assign_slice()
                );
            }else{
                _nestMetas[idx].asb->do_non_block_asm(
                        *rhs_opr,
                        result_collector,
                        rhs_opr->get_operable_slice(),
                        lhs_asb->get_assign_slice()
                );
            }
        }

        /** do recursive element assigning*/
        for(size_t idx = 0; idx < _recurBoxs.size(); idx++){
            Box* sub_lhs    = _recurBoxs[idx];
            Box* sub_rhs_box = rhs_box.get_sub_box()[idx];
            sub_lhs->collect_assign_meta(*sub_rhs_box, is_block_asm, result_collector);
        }

    }

    void Box::build_asm_node(Box& rhs_box, bool is_block_asm){
        /**build vector to collect system*/
        std::vector<AssignMeta*> result_collector;
        /**collect assignmeta*/
        collect_assign_meta(rhs_box, is_block_asm, result_collector);
        /**build assignment node*/
        auto* asm_node = new AsmNode(result_collector);
        /**communication controller*/
        ctrl->on_box_update(asm_node, this);
    }

    Box& Box::operator=(Box& rhs) {
        build_asm_node(rhs, false);
        return *this;
    }

    Box& Box::operator<<=(Box& rhs) {
        build_asm_node(rhs, true);
        return *this;
    }

    std::string Box::get_md_ident_val(){
        return get_ident_debug_value();
    }

    void Box::add_md_log(MdLogVal* md_log_val){
        assert(md_log_val != nullptr);
        /** add sub element into log*/
        for (auto nest_meta: _nestMetas){
            std::string sub_ident = nest_meta.opr1->cast_to_ident()->get_ident_debug_value();
            md_log_val->add_val(sub_ident);
        }
        /**add recur box*/
        for (auto recur_box: _recurBoxs){
            MdLogVal* sub_log = md_log_val->make_new_sub_val();
            recur_box->add_md_log(sub_log);
        }
    }

}