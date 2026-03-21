//
// Created by tanawin on 25/2/2567.
//

#ifndef KATHRYN_NEST_H
#define KATHRYN_NEST_H



#include "memory"
#include "string"
#include "iostream"
#include "utility"
#include "model/controller/con_interf/controller_itf.h"
#include "model/hw_component/abstract/logic_comp.h"
#include "gen/proxy_hw_comp/expression/nest_gen.h"




namespace kathryn{


    struct NestMeta{
        Operable*   opr1;
        Assignable* asb;
    };

    /////////// for nest vector read and write are acceptable

    std::vector<NestMeta> get_nest_vec();

    template<typename OA, typename... T>
    std::vector<NestMeta> get_nest_vec(OA& oa, T&... args){
        std::vector<NestMeta> deep_get  = get_nest_vec(args...);
        deep_get.push_back({&oa, &oa});
        return deep_get;
    }

    class nest;

    template<typename OA, typename... T>
    nest& make_nest(bool is_user_dec,OA& oa, T&... args){
        auto nest_list = get_nest_vec(oa, args...);
        int nest_size = 0;
        for (NestMeta nest_meta: nest_list){
            assert(nest_meta.opr1 != nullptr);
            nest_size += nest_meta.opr1->get_operable_slice().get_size();
        }
        return _make<nest>("uncatagorized_yet", "nest", is_user_dec, nest_size, nest_list);
    }

    /////////// for nest vector read is only acceptable

    std::vector<Operable*> get_nest_vec_read_only();

    template<typename... T>
    std::vector<Operable*> get_nest_vec_read_only(Operable& first_operable, T&... args){
        std::vector<Operable*> deep_get = get_nest_vec_read_only(args...);
        deep_get.push_back(&first_operable);
        return deep_get;
    }

    template<typename S, typename... T>
    nest& make_nest_read_only(bool is_user_dec, S& first_operable, T&... args){
        auto nest_list = get_nest_vec_read_only(first_operable, args...);
        return make_nest_man_read_only(is_user_dec, nest_list);
    }

    //////////// for manual built
    nest& make_nest_man(bool is_user, const std::vector<NestMeta>& grouped_meta);
    nest& make_nest_man_read_only(bool is_user, const std::vector<Operable*>& nest_list_read_only);

    class nest : public LogicComp<nest>{
        friend class NestSimEngine;
        friend class NestGen;
        private:
            /** the higher bit is most significant bit*/
            bool read_only = false;
            std::vector<NestMeta> _nestList;

        protected:
            void com_init() override;

        public:

            explicit nest(int size, std::vector<NestMeta> nest_list);
            explicit nest(int size, const std::vector<Operable*>& nest_list);

            void com_final() override {};
            /** override assignable*/
            void do_block_asm    (Operable& src_opr, Slice des_slice) override;
            void do_non_block_asm (Operable& src_opr, Slice des_slice) override;

            void do_block_asm    (Operable& src_opr,
                                std::vector<AssignMeta*>& result_meta_collector,
                                Slice  abs_src_slice,
                                Slice  abs_des_slice) override;
            void do_non_block_asm (Operable& src_opr,
                                std::vector<AssignMeta*>& result_meta_collector,
                                Slice  abs_src_slice,
                                Slice  abs_des_slice) override;

            void do_global_asm   (Operable& src_opr,
                                std::vector<AssignMeta*>& result_meta_collector,
                                Slice  abs_src_slice,
                                Slice  abs_des_slice,
                                ASM_TYPE asm_type) override {assert(false);/**disable this function*/}

            CLOCK_MODE get_cur_assign_clk_mode() {assert(false); return CM_CLK_UNUSED;}

            nest& operator = (Operable& b){ operator_eq(b);                                return *this;}
            nest& operator = (ull b)      { operator_eq(b);                                   return *this;}
            nest& operator = (nest& b)    { if (this == &b){return *this;} operator_eq(b); return *this;}

        /**get which netlist element match that bit Idx */
            [[maybe_unused]]
            int get_net_list_idx_that_match(int bit_idx);

            void do_nest_global_asm(Operable& src_opr,
                                 std::vector<AssignMeta*>& result_meta_collector,
                                 Slice  abs_src_slice,
                                 Slice  abs_des_slice,
                                 bool isblocking_asm
                                       );

            /** override slicable*/
            SliceAgent<nest>& operator() (int start, int stop) override;
            SliceAgent<nest>& operator() (int idx) override;
            SliceAgent<nest>& operator() (Slice sl) override;
            Operable* do_slice(Slice sl) override;

            std::vector<NestMeta>& get_nest_list() {return _nestList;}

            /** check short circuit*/
            Operable* check_short_circuit() override;

            /**override logicc gen base*/
            void create_logic_gen() override;

    };

}

#endif //KATHRYN_NEST_H
