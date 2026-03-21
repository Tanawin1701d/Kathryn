//
// Created by tanawin on 5/4/2567.
//

#ifndef KATHRYN_DECODE_H
#define KATHRYN_DECODE_H

#include "kathryn.h"
#include "example/riscv/element.h"

namespace kathryn{

    namespace riscv{

        struct Decode {

            CORE_DATA& cd;
            ZyncSimProb zync_sim_prob;
            PipSimProbe pip_sim_probe;

            explicit Decode(CORE_DATA& core_data): cd(core_data){

                InstrRepo& repo = cd.dc.repo;
                repo.add_fix_prefix("<11>");
                repo.add_mop({"ldst", {"lsb", "lsh", "lsw", "usign", "isload"}});
                repo.add_mop({"br", {"beq","bge","bne","bltu","blt","bgeu"}});
                repo.add_mop({"jal", {"needpc"}});
                repo.add_mop({"ldpc", {"needpc"}});
                repo.add_mop({"op",{"add","sll","sltu","sr", "sra","and",
                    "sub","slt","xor","or"}});

                /////////////////////////////////////////////////////////////////////////////////////
                ////// load/store/and////////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////////////////////////////////////
                /// load
                repo.add_dec_rule("ldst", "<12-i2-0-12><5-rs0><1-ufb><2-ufa><5-rd0><0000011>").
                ad("<ufa>",{{"<00>", "lsb"}, {"<01>", "lsh"}, {"<10>", "lsw"}}).
                ad("<ufb>", {{"<1>", "usign"}}).
                adm({"isload"});
                /// store
                repo.add_dec_rule("ldst", "<7-i2-5-12><5-rs1><5-rs0><3-ua><5-i2-0-5><0100011>").
                ad("<ua>", {{"<000>", "lsb"},{"<001>", "lsh"},{"<010>", "lsw"}});
                /// branch
                repo.add_dec_rule("br", "<1-i2-12-13><6-i2-5-11><5-rs1>"
                                "<5-rs0><3-ua><4-i2-1-5><1-i2-11-12><1100011>").
                ad("<ua>", {{"<000>", "beq"},{"<001>", "bne" },{"<100>", "blt" },
                            {"<101>", "bge"},{"<110>", "bltu"},{"<111>", "bgeu"}});
                ////// 001 jalr
                repo.add_dec_rule("jal", "<12-i1-0-12><5-rs0><000><5-rd0><1100111>");
                ////// 011 jal
                repo.add_dec_rule("jal", "<1-i1-20-21><10-i1-1-11><1-i1-11-12><8-i1-12-20><5-rd0><1101111>")
                .adm({"needpc"});
                /////////////////////////////////////////////////////////////////////////////////////
                ////// op/op_imm ////////////////////////////////////////////////////////////////////
                ////////////////////////////////////////////////////////////////////////////////////
                repo.add_dec_rule("op", "<0><1-ub><00000><5-rs1><5-rs0><3-ua><5-rd0><0110011>").
                ad("<ub><ua>", {{"<0><000>","add" },{"<1><000>","sub"},
                                {"<0><001>","sll" },{"<0><010>","slt"},
                                {"<0><011>","sltu"},{"<0><100>","xor"},
                                {"<0><101>","sr"  },{"<1><101>","sra"},
                                {"<0><110>","or"  },{"<0><111>","and"}});

                repo.add_dec_rule("op", "<12-i1-0-12><5-rs0><3-ua><5-rd0><0010011>").
                ad("<ua>", {{"<000>","add" }, {"<010>","slt"},{"<011>","sltu"},
                            {"<100>","xor" }, {"<110>","or" },{"<111>","and"},
                            {"<001>","sll"  }})
                .ad("<s-30-31><ua>", {{"<1><101>", "sra"}, {"<0><101>","sr"}});
                ////// 101 auipc/lui
                repo.add_dec_rule("ldpc", "<20-i1-12-32><5-rd0><0010111>").adm({"needpc"});
                repo.add_dec_rule("ldpc", "<20-i1-12-32><5-rd0><0110111>");


                repo.declare_hw();

            }

            void flow() {
                pip(cd.dc.sync){      init_probe(pip_sim_probe);
                    zync(cd.ex.sync){ init_probe(zync_sim_prob);
                        cd.dc.pc     <<= cd.ft.fetch_pc;
                        cd.dc.next_pc <<= cd.ft.fetch_nextpc;
                        cd.dc.repo.gen_decode_logic();

                        // m_wire(dbg_st_decode, 1);
                        // dbg_st_decode = 1;
                        // dbg_st_decode.as_output_glob("st_decode");

                    }
                }
            }
        };
    }

}
#endif //KATHRYN_DECODE_H