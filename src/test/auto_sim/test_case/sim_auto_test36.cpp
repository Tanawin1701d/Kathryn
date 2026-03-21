//
// Created by tanawin on 22/1/2567.
//
//
// #include "kathryn.h"
// #include "test/auto_sim/sim_auto_interface.h"
// #include "auto_test_interface.h"
// #include "lib/instr/instr_base.h"
//
// namespace kathryn{
//
//     class test_sim_mod36: public Module{
//     public:
//         m_reg(instr, 32);
//         InstrRepo repo;
//
//         explicit test_sim_mod36(int x): Module(),
//         repo(32,3, 1, 32, &instr){
//
//             repo.add_mop({"r_type", {"add", "sll", "sltu", "sub", "slt", "xor"}});
//
//             repo.add_dec_rule("r_type", "<7-ub><5-rs1><5-rs0><3-ua><5-rd0><0110011>").
//             ad("<ub><ua>",{{"<0000000><000>","add" },{"<0100000><000>","sub"},
//                            {"<0000000><001>","sll" },{"<0000000><010>","slt"},
//                            {"<0000000><011>","sltu"},{"<0000000><100>","xor"}});
//
//             repo.add_dec_rule("r_type", "<12-i1-0-12><5-rs0><3-ua><5-rd0><0010011>").
//             ad("<ua>"    ,{{"<000>", "add"},{"<100>", "xor"}});
//
//             repo.declare_hw();
//         }
//
//         void flow() override{
//             cwhile(true){
//                 repo.gen_decode_logic();
//             }
//         }
//
//     };
//
//     class sim36 :public SimAutoInterface{
//     public:
//
//         test_sim_mod36* _md = nullptr;
//
//         sim36(test_sim_mod36* md, int idx, const std::string& prefix, SimProxyBuildMode sim_proxy_build_mode):SimAutoInterface(idx,100,
//                                                                                   prefix + "sim_auto_result"+std::to_string(idx)+".vcd",
//                                                                                   prefix + "sim_auto_result"+std::to_string(idx)+".prof", sim_proxy_build_mode),
//         _md(md)
//         {}
//         void sim_assert() override{}
//         void sim_driven() override{}
//
//         void describe_con() override{
//
//             OP_HW& op_hw =  _md->repo.get_op("r_type");
//
//             _md->instr.s(0b0000000'01000'00010'010'00100'0110011);
//             con_next_cycle(1);
//
//             std::cout << "------------ test slt r_type" << std::endl;
//             test_and_print("src0Valid",(ull)_md->repo.get_src_reg(0).valid, 0);
//             test_and_print("src0idx"  ,(ull)_md->repo.get_src_reg(0).idx  , 2);
//
//             test_and_print("src1Valid",(ull)_md->repo.get_src_reg(1).valid, 0);
//             test_and_print("src1idx"  ,(ull)_md->repo.get_src_reg(1).idx  , 8);
//
//             test_and_print("des1Valid",(ull)_md->repo.get_des_reg(0).valid, 0);
//             test_and_print("des1idx"  ,(ull)_md->repo.get_des_reg(0).idx  , 4);
//
//             /////////// test
//
//             test_and_print("mop_set", (ull)op_hw._set, 1);
//             for (auto[uop_name, idx]: op_hw.uop_map_idx){
//                 if (uop_name == "slt"){
//                     test_and_print("test_uop", (ull)*op_hw._uopSets[idx], 1);
//                 }else{
//                     test_and_print("test_uop false", (ull)*op_hw._uopSets[idx], 0);
//                 }
//             }
//
//
//
//             _md->instr.assign_sim_value(0b100100000000'00101'100'01010'0010011);
//             con_next_cycle(1);
//             std::cout << "------------ test xor i_type" << std::endl;
//             test_and_print("src0Valid",(ull)_md->repo.get_src_reg(0).valid, 0);
//             test_and_print("src0idx"  ,(ull)_md->repo.get_src_reg(0).idx  , 5);
//
//             test_and_print("src1Valid",(ull)_md->repo.get_src_reg(1).valid, 1);
//             test_and_print("src1idx"  ,(ull)_md->repo.get_src_reg(1).idx  , 0);
//             test_and_print("src1data" ,(ull)_md->repo.get_src_reg(1).data , 4294965504);
//
//             test_and_print("des1Valid",(ull)_md->repo.get_des_reg(0).valid, 0);
//             test_and_print("des1idx"  ,(ull)_md->repo.get_des_reg(0).idx  , 10);
//
//             test_and_print("mop_set", (ull)op_hw._set, 1);
//             for (auto[uop_name, idx]: op_hw.uop_map_idx){
//                 if (uop_name == "xor"){
//                     test_and_print("test_uop", (ull)*op_hw._uopSets[idx], 1);
//                 }else{
//                     test_and_print("test_uop false", (ull)*op_hw._uopSets[idx], 0);
//                 }
//             }
//
//             _md->instr.assign_sim_value(0b0000000'01010'00111'000'10000'0110011);
//             con_next_cycle(1);
//
//             std::cout << "------------ test add r_type" << std::endl;
//             test_and_print("src0Valid",(ull)_md->repo.get_src_reg(0).valid, 0);
//             test_and_print("src0idx"  ,(ull)_md->repo.get_src_reg(0).idx  , 7);
//
//             test_and_print("src1Valid",(ull)_md->repo.get_src_reg(1).valid, 0);
//             test_and_print("src1idx"  ,(ull)_md->repo.get_src_reg(1).idx  , 10);
//
//             test_and_print("des1Valid",(ull)_md->repo.get_des_reg(0).valid, 0);
//             test_and_print("des1idx"  ,(ull)_md->repo.get_des_reg(0).idx  , 16);
//
//
//             test_and_print("mop_set", (ull)op_hw._set, 1);
//             for (auto[uop_name, idx]: op_hw.uop_map_idx){
//                 if (uop_name == "add"){
//                     test_and_print("test_uop", (ull)*op_hw._uopSets[idx], 1);
//                 }else{
//                     test_and_print("test_uop false", (ull)*op_hw._uopSets[idx], 0);
//                 }
//             }
//
//
//
//         }
//     };
//
//     class Sim36TestEle: public AutoTestEle{
//     public:
//         explicit Sim36TestEle(int id): AutoTestEle(id){}
//         void start(std::string prefix, SimProxyBuildMode sim_proxy_build_mode) override{
//             m_mod(d, test_sim_mod36, 1);
//             start_model_kathryn();
//             sim36 simulator((test_sim_mod36*) &d, _simId, prefix, sim_proxy_build_mode);
//             //std::cout << get_global_module_ptr()->get_user_wires()[0]->get_var_name() << std::endl;
//             //std::cout << get_global_module_ptr()->get_user_wires()[0]->get_sim_engine_ptr()->get_var_name() << std::endl;
//             simulator.sim_start();
//         }
//
//     };
//
//     Sim36TestEle ele36(36);
//
// }
