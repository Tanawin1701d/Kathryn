// //
// // Created by tanawin on 26/4/2567.
// //
//
// #include "riscv_sim_sort.h"
// #include "util/term_color/term_color.h"
//
// #include "utility"
//
// namespace kathryn{
//
//     namespace riscv{
//
//
//         RiscvSimInterfaceSort::RiscvSimInterfaceSort(CYCLE       limit_cycle,
//                                              std::string prefix,
//                                              std::string test_type,
//                                              Riscv& core):
//                 SimInterface(limit_cycle,
//                              std::move(prefix + test_type + "/owave.vcd"),
//                              std::move(prefix + test_type + "/oprofile.prof")),
//                 _core(core),
//                 slot_writer({"fetch", "decode", "execute", "wb", "mem"},
//                            25,
//                            std::move(prefix + test_type + "/oslot.sl")),
//                 _prefixFolder(prefix),
//                 _testType(test_type){}
//
//         void RiscvSimInterfaceSort::describe() {
//             read_assembly (_prefixFolder + _testType + "/asm.out");
//             //read_assert_val(_prefixFolder + _testType + "/ast.out");
//
//             /** write data for sorting*/
//             for (int i = 0; i < _numSize; i++){
//                 _core.mem_blk._myMem.at((_startNumIdx0 + 4*i)/4).set_var(_numSize - i + 10);
//             }
//         }
//
//         void RiscvSimInterfaceSort::describe_con() {
//
//             for (int i = 0; i < _limitCycle - 2; i++){
//                 //std::cout<< i << std::endl;
//                 if (i %10000 == 0){
//                     std::cout << i << " pass" << std::endl;
//                 }
//                 con_end_cycle();
//                 record_slot();
//                 con_next_cycle(1);
//             }
// //            bool pass = true;
// //            for (int i = 0;  i < AMT_REG; i++){
// //                if (_regTestVal[i] != _core.reg_file.v(i)){
// //                    pass = false;
// //                    test_and_print("fail reg" + std::to_string(i),
// //                                 _core.reg_file.v(i), _regTestVal[i]);
// //                }
// //
// //            }
// //            if (pass){
// //                std::cout << TC_GREEN << "register val test pass" << TC_DEF << std::endl;
// //            }else{
// //                std::cout << TC_RED << "register val test fail" << TC_DEF << std::endl;
// //            }
//
//         }
//
//         void RiscvSimInterfaceSort::record_slot() {
//
//             /** please bare in mind that this recorder work correctly when
//              *  it is the end of the cycle
//              * */
//              auto& pip_stages = _core.pip_probe->get_pip_stage();
//
//             /*** record fetch */
//             FlowBlockPipeBase* fetch = pip_stages[RISC_FETCH_SORT];
//             write_fetch_slot(fetch);
//             /*** decode*/
//             FlowBlockPipeBase* decode = pip_stages[RISC_DECODE_SORT];
//             write_decode_slot(decode);
//             /*** execute*/
//             FlowBlockPipeBase* execute = pip_stages[RISC_EXECUTE_SORT];
//             write_execute_slot(execute);
//             /*** write back*/
//             FlowBlockPipeBase* write_back = pip_stages[RISC_WB_SORT];
//             write_wb_slot(write_back);
//             /*** memory writer*/
//             write_mem();
//
//             slot_writer.iterate_cycle();
//         }
//
//         bool RiscvSimInterfaceSort::write_slot_if_stall(PIPE_STAGE_SORT stage_idx,
//                                                  FlowBlockPipeBase* pipfb) {
//
//             ///////// if it is running in con thread type it will be run after model sim but before exit event of all type
//             assert(pipfb != nullptr);
//             /** check recv block*/
//             FlowBlockPipeCom* recv_pip_com = pipfb->get_recv_fb_pip_com();
//             bool recv_running = recv_pip_com->get_sim_engine_ptr()->is_block_running();
//             if (recv_running)
//                 slot_writer.add_slot_val(stage_idx, "WAIT_RECV");
//
//             /** check send block*/
//             FlowBlockPipeCom* send_pip_com = pipfb->get_send_fb_pip_com();
//             bool send_running =  send_pip_com->get_sim_engine_ptr()->is_block_running();
//             if (send_running)
//                 slot_writer.add_slot_val(stage_idx, "WAIT_SEND");
//
//             return recv_running | send_running;
//         }
//
//         void RiscvSimInterfaceSort::write_fetch_slot(FlowBlockPipeBase* pipblock) {
//             assert(pipblock != nullptr);
//             if (write_slot_if_stall(RISC_FETCH_SORT, pipblock)){return;}
//
//
//             if (_core.fetch.fetch_block->get_sim_engine_ptr()->is_block_running()) {
//
//                 if (ull(_core.fetch.read_en)) {
//                     if (ull(_core.fetch.read_fin)) {
//                         slot_writer.add_slot_val(RISC_FETCH_SORT, "READ ADDR");
//                         slot_writer.add_slot_val(RISC_FETCH_SORT,
//                                               cvt_num2_hex_str(ull(_core.fetch._reqPc)));
//                     } else {
//                         slot_writer.add_slot_val(RISC_FETCH_SORT, "FETCHING WAIT4MEM");
//                     }
//                 } else {
//                     slot_writer.add_slot_val(RISC_FETCH_SORT, "FETCHING  WAIT4REQ");
//                 }
//
//             }else{
//                 slot_writer.add_slot_val(RISC_FETCH_SORT, "Unknown State");
//             }
//
//             slot_writer.add_slot_val(RISC_FETCH_SORT, "is_fin " + std::to_string(ull(_core.fetch.read_fin)));
//             slot_writer.add_slot_val(RISC_FETCH_SORT, "is_en " + std::to_string(ull(_core.fetch.read_en)));
//
//
//         }
//
//         void RiscvSimInterfaceSort::write_decode_slot(FlowBlockPipeBase* pipblock) {
//             assert(pipblock != nullptr);
//
//             if (write_slot_if_stall(RISC_DECODE_SORT, pipblock)){return;}
//
//             //////////// now decoding
//
//             ull raw_intr  = ull(_core.fetch_data.fetch_instr);
//             ull pc       = ull(_core.fetch_data.fetch_pc);
//             ull next_pc  = ull(_core.fetch_data.fetch_nextpc);
//
//
//             ull op_mask_bit = (1 << 7) - 1;
//             ull op = raw_intr & op_mask_bit;
//
//             std::map<ull, std::string> dec_map = {
//                 {0b00'000'11, "LOAD"},
//                 {0b01'000'11, "STORE"},
//                 {0b00'011'11, "MISCMEM"},
//                 {0b11'000'11, "BRANCH"},
//                 {0b11'001'11, "JALR"},
//                 {0b11'011'11, "JAL"},
//                 {0b00'100'11, "OP_IMM"},
//                 {0b01'100'11, "OP"},
//                 {0b00'101'11, "AUIPC"},
//                 {0b01'101'11, "LUI"},
//                 {0b11'100'11, "SYSTEM"},
//             };
//
//             if (_core.decode.decode_blk->get_sim_engine_ptr()->is_block_running()) {
//
//                 std::string dec_str = (dec_map.find(op) != dec_map.end()) ? dec_map[op] : "UNKNOWN";
//                 slot_writer.add_slot_val(RISC_DECODE_SORT, dec_str);
//                 slot_writer.add_slot_val(RISC_DECODE_SORT, cvt_num2_hex_str(pc));
//                 slot_writer.add_slot_val(RISC_DECODE_SORT, cvt_num2_hex_str(next_pc));
//             }else{
//                 slot_writer.add_slot_val(RISC_DECODE_SORT, "UNKNOWN STATE");
//             }
//
//         }
//
//         void RiscvSimInterfaceSort::write_execute_slot(FlowBlockPipeBase* pipblock) {
//             assert(pipblock != nullptr);
//
//             if (write_slot_if_stall(RISC_EXECUTE_SORT, pipblock)){return;}
//
//             UOp& decoded_uop = _core.execute._decodedUop;
//
//             /** decode
//              *
//              * uop
//              *
//              * */
//             if (ull(decoded_uop.op_ls.is_uop_use)){
//                 int size = 8 * (1 << ull(decoded_uop.op_ls.size));
//                 slot_writer.add_slot_val(RISC_EXECUTE_SORT,
//                                       (ull(decoded_uop.op_ls.is_mem_load) ? "LOAD": "STORE") +
//                                       std::to_string(size));
//             }else if (ull(decoded_uop.op_alu.is_uop_use)){
//                 slot_writer.add_slot_val(RISC_EXECUTE_SORT,
//                                       ull(decoded_uop.op_alu.is_add)        ? "Add":
//                                       ull(decoded_uop.op_alu.is_sub)              ? "Sub":
//                                       ull(decoded_uop.op_alu.is_xor)              ? "Xor":
//                                       ull(decoded_uop.op_alu.is_or)               ? "Or":
//                                       ull(decoded_uop.op_alu.is_and)              ? "And":
//                                       ull(decoded_uop.op_alu.is_cmp_less_than_sign)  ? "CmpLessThanSign":
//                                       ull(decoded_uop.op_alu.is_cmp_less_than_u_sign) ? "CmpLessThanUSign":
//                                       ull(decoded_uop.op_alu.is_shift_left_logical) ? "ShiftLeftLogical":
//                                       ull(decoded_uop.op_alu.is_shift_right_logical)? "ShiftRightLogical":
//                                       ull(decoded_uop.op_alu.is_shift_right_arith)  ? "ShiftRightArith":
//                                       "UNKNOW_alu"
//                 );
//
//             }else if (ull(decoded_uop.op_ctrl_flow.is_uop_use)){
//                 slot_writer.add_slot_val(RISC_EXECUTE_SORT,
//                                       ull(decoded_uop.op_ctrl_flow.is_jal)  ? "JAL":
//                                       ull(decoded_uop.op_ctrl_flow.is_jal_r) ? "JALR":
//                                       ull(decoded_uop.op_ctrl_flow.is_eq)   ? "JALEQ":
//                                       ull(decoded_uop.op_ctrl_flow.is_n_eq)  ? "BRA_NEQ":
//                                       ull(decoded_uop.op_ctrl_flow.is_lt)   ? "BRA_LT":
//                                       ull(decoded_uop.op_ctrl_flow.is_ge)   ? "BRA_GE":
//                                       "UNKNOW_BRANCH"
//                 );
//             }else if (ull(decoded_uop.op_ld_pc.is_uop_use)){
//                 slot_writer.add_slot_val(RISC_EXECUTE_SORT,ull(decoded_uop.op_ld_pc.need_pc) ? "LDPC": "LDPC_PC");
//             }else{
//                 slot_writer.add_slot_val(RISC_EXECUTE_SORT, "UNKNOWN_UOP");
//             }
//
//             if (_core.execute.reg_access_block
//                 ->get_sim_engine_ptr()->is_block_running()){
//                 slot_writer.add_slot_val(RISC_EXECUTE_SORT, "REG_ACCESS");
//             }else if (_core.execute.alu_block
//                     ->get_sim_engine_ptr()->is_block_running()){
//                 slot_writer.add_slot_val(RISC_EXECUTE_SORT, "SIMPLE_ALU");
//             }else if (_core.execute.alu_block
//                     ->get_sim_engine_ptr()->is_block_running()){
//                 slot_writer.add_slot_val(RISC_EXECUTE_SORT, "COMPLEX_ALU");
//             }else{
//                 slot_writer.add_slot_val(RISC_EXECUTE_SORT, "unknown_state");
//             }
//
//             /** register write */
//
//             write_reg("r1", RISC_EXECUTE_SORT, decoded_uop.reg_data[RS_1  ]);
//             write_reg("r2", RISC_EXECUTE_SORT, decoded_uop.reg_data[RS_2  ]);
//             write_reg("r3", RISC_EXECUTE_SORT, decoded_uop.reg_data[RS_3  ]);
//             write_reg("rd", RISC_EXECUTE_SORT, decoded_uop.reg_data[RS_des]);
//             slot_writer.add_slot_val(RISC_EXECUTE_SORT, "read_fin " + std::to_string(ull(_core.execute.read_fn)));
//             slot_writer.add_slot_val(RISC_EXECUTE_SORT, "read_en " + std::to_string(ull(_core.execute.read_en)));
//             slot_writer.add_slot_val(RISC_EXECUTE_SORT, "fin_ls " + std::to_string(ull(_core.execute.test_exit)));
//             slot_writer.add_slot_val(RISC_EXECUTE_SORT, "read_addr " + std::to_string(ull(_core.execute.read_addr)));
//             slot_writer.add_slot_val(RISC_EXECUTE_SORT, "m16 " + std::to_string(ull(_core.execute.m16)));
//             slot_writer.add_slot_val(RISC_EXECUTE_SORT, "read1020 " + std::to_string(ull(_core.mem_blk._myMem.at(1020 >> 2))));
//             slot_writer.add_slot_val(RISC_EXECUTE_SORT, "read1024 " + std::to_string(ull(_core.mem_blk._myMem.at(1024 >> 2))));
//             slot_writer.add_slot_val(RISC_EXECUTE_SORT, "read1028 " + std::to_string(ull(_core.mem_blk._myMem.at(1028 >> 2))));
//             slot_writer.add_slot_val(RISC_EXECUTE_SORT, "reset_signal " + std::to_string(ull(_core.mis_predic)));
//
//
//         }
//
//         void RiscvSimInterfaceSort::write_wb_slot(FlowBlockPipeBase* pipblock) {
//             assert(pipblock != nullptr);
//             if (write_slot_if_stall(RISC_WB_SORT, pipblock)){return;}
//
//             RegEle& wb_reg = _core.wb_data;
//
//             write_reg("rwb", RISC_WB_SORT, wb_reg);
//             slot_writer.add_slot_val
//             (RISC_WB_SORT, std::to_string(ull(wb_reg.val)));
//         }
//
//         void RiscvSimInterfaceSort::write_reg(const std::string& prefix,
//                                          PIPE_STAGE_SORT         pipe_stage,
//                                          RegEle&            reg_ele){
//
//             slot_writer.add_slot_val(pipe_stage, prefix + " id " +
//                                                     std::to_string(ull(reg_ele.idx)) + " v" +
//                                                     std::to_string(ull(reg_ele.valid)) + " val " +
//                                                     std::to_string(ull(reg_ele.val))
//                                                     );
//         }
//
//         void RiscvSimInterfaceSort::write_mem(){
//             for (int i = 0; i < _numSize; i++){
//                 slot_writer.add_slot_val(RISC_MEM_SORT, "idx" + std::to_string(i) + " " +
//                                         std::to_string(ull(_core.mem_blk._myMem.at((_startNumIdx0 + 4*i)/4))) + " addr " + std::to_string((_startNumIdx0 + 4*i)/4));
//             }
//             slot_writer.add_slot_val(RISC_MEM_SORT, "------------------------------");
//             for (int i = 0; i < _numSize; i++){
//                 slot_writer.add_slot_val(RISC_MEM_SORT, "idx" + std::to_string(i) + " " +
//                                                 std::to_string((ull)(_core.mem_blk._myMem.at((_startNumIdx1 + 4*i)/4 )))
//                                                 + " addr " + std::to_string((_startNumIdx1 + 4*i)/4)
//                                                 );
//             }
//
//         }
//
//
//         void RiscvSimInterfaceSort::read_assembly(const std::string& file_path){
//
//             ///////// initialize file
//             std::ifstream asm_file(file_path, std::ios::binary);
//             if (!asm_file.is_open()){assert(false);}
//             asm_file.seekg(0, std::ios::end);
//             std::streampos file_size = asm_file.tellg();
//             assert((file_size % 4) == 0);
//             asm_file.seekg(0, std::ios::beg);
//
//
//             /** read instruction from file and write it to memory block*/
//             uint32_t write_addr = 0;
//             uint32_t instr;
//             while(asm_file.read(reinterpret_cast<char*>(&instr), sizeof instr)){
//                 assert((instr & 0b11) == 0b11); ////// check instruction
//                 _core.mem_blk._myMem.at(write_addr).set_var(instr);
//                 //////////////std::cout << instr << std::endl;
//                 write_addr++;
//             }
//             asm_file.close();
//
//
// //            for (;
// //                 write_addr < 400;////(1 << MEM_ADDR_IDX_ACTUAL_AL32);
// //                 write_addr++){
// //                _core.mem_blk._myMem.s(write_addr, 0b0010011);
// //            }
//             std::cout << TC_GREEN << "initialize mem finish" << TC_DEF << std::endl;
//
//
//
//         }
//
//
//         void RiscvSimInterfaceSort::read_assert_val(const std::string& file_path){
//
//             std::vector<std::string> raw_vals;
//
//             FileReaderBase reader(file_path);
//             raw_vals = reader.read_lines();
//
//             assert(raw_vals.size() == AMT_REG);
//
//             for (int reg_idx = 0; reg_idx < AMT_REG; reg_idx++){
//                 ///std::cout << reg_idx << " reg val  "<< stoul(raw_vals[reg_idx]) << std::endl;
//                 _regTestVal[reg_idx] = stoul(raw_vals[reg_idx]);
//                 ///std::cout << reg_idx << " reg val  "<< _regTestVal[reg_idx] << std::endl;
//             }
//         }
//
//
//     }
//
// }