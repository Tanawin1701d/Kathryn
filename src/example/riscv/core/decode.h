//
// Created by tanawin on 5/4/2567.
//

#ifndef KATHRYN_DECODE_H
#define KATHRYN_DECODE_H

#include "kathryn.h"
#include "example/riscv/element.h"

namespace kathryn{

    namespace riscv{

        class Decode {

        public:
            UOp& _uop;
            FlowBlockBase* decodeBlk = nullptr;

            explicit Decode(UOp& uop): _uop(uop){

                InstrRepo& repo = _uop.repo;
                repo.addFixPrefix("<11>");
                repo.addMop({"ldst", {"lsb", "lsh", "lsw", "usign", "isload"}});
                repo.addMop({"br", {"beq","bge","bne","bltu","blt","bgeu"}});
                repo.addMop({"jal", {"needpc"}});
                repo.addMop({"ldpc", {"needpc"}});
                repo.addMop({"op",{"add","sll","sltu","sr", "sra","and",
                    "sub","slt","xor","or"}});

                /////////////////////////////////////////////////////////////////////////////////////
                ////// load/store/and////////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////////////////////////////////////
                /// load
                repo.addDecRule("ldst", "<12-i2-0-12><5-rs0><1-ufb><2-ufa><5-rd0><0000011>").
                ad("<ufa>",{{"<00>", "lsb"}, {"<01>", "lsh"}, {"<10>", "lsw"}}).
                ad("<ufb>", {{"<1>", "usign"}}).
                adm({"isload"});
                /// store
                repo.addDecRule("ldst", "<7-i2-5-12><5-rs1><5-rs0><3-ua><5-i2-0-5><0100011>").
                ad("<ua>", {{"<000>", "lsb"},{"<001>", "lsh"},{"<010>", "lsw"}});
                /// branch
                repo.addDecRule("br", "<1-i2-12-13><6-i2-5-11><5-rs1>"
                                "<5-rs0><3-ua><4-i2-1-5><1-i2-11-12><1100011>").
                ad("<ua>", {{"<000>", "beq"},{"<001>", "bne" },{"<100>", "blt" },
                            {"<101>", "bge"},{"<110>", "bltu"},{"<111>", "bgeu"}});
                ////// 001 jalr
                repo.addDecRule("jal", "<12-i1-0-12><5-rs0><000><5-rd0><1100111>");
                ////// 011 jal
                repo.addDecRule("jal", "<1-i1-20-21><10-i1-1-11><1-i1-11-12><8-i1-12-20><5-rd0><1101111>")
                .adm({"needpc"});
                /////////////////////////////////////////////////////////////////////////////////////
                ////// op/opImm ////////////////////////////////////////////////////////////////////
                ////////////////////////////////////////////////////////////////////////////////////
                repo.addDecRule("op", "<0><1-ub><00000><5-rs1><5-rs0><3-ua><5-rd0><0110011>").
                ad("<ub><ua>", {{"<0><000>","add" },{"<1><000>","sub"},
                                {"<0><001>","sll" },{"<0><010>","slt"},
                                {"<0><011>","sltu"},{"<0><100>","xor"},
                                {"<0><101>","sr"  },{"<1><101>","sra"},
                                {"<0><110>","or"  },{"<0><111>","and"}});

                repo.addDecRule("op", "<12-i1-0-12><5-rs0><3-ua><5-rd0><0010011>").
                ad("<ua>", {{"<000>","add" }, {"<010>","slt"},{"<011>","sltu"},
                            {"<100>","xor" }, {"<110>","or" },{"<111>","and"},
                            {"<001>","sll"  }})
                .ad("<s-30-31><ua>", {{"<1><101>", "sra"}, {"<0><101>","sr"}});
                ////// 101 auipc/lui
                repo.addDecRule("ldpc", "<20-i1-12-32><5-rd0><0010111>").adm({"needpc"});
                repo.addDecRule("ldpc", "<20-i1-12-32><5-rd0><0110111>");


                repo.declareHw();

            }

            void flow(Operable& rst, FETCH_DATA& fetchData) {

                pipBlk { intrReset(rst); intrStart(rst); exposeBlk(decodeBlk)
                    cdowhile(!nextPipReadySig) {
                        par {
                            zif(nextPipReadySig){
                                /////// deal with pc
                                _uop.pc     <<= fetchData.fetch_pc;
                                _uop.nextPc <<= fetchData.fetch_nextpc;
                                _uop.repo.genDecodeLogic();
                            }
                        }
                    }
                }
            }
        };
    }

}
#endif //KATHRYN_DECODE_H