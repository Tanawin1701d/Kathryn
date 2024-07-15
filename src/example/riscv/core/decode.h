//
// Created by tanawin on 5/4/2567.
//

#ifndef KATHRYN_DECODE_H
#define KATHRYN_DECODE_H

#include "kathryn.h"
#include "example/riscv/element.h"
#include "example/numberic/extend.h"

namespace kathryn{

    namespace riscv{

        class Decode {

        public:
            UOp& _uop;
            FlowBlockBase* decodeBlk = nullptr;

            explicit Decode(UOp& uop): _uop(uop){

                InstrRepo& repo = _uop.repo;

                ////// 000 ld store and branch
                repo.addDecRule("<12-i1-0-12><5-rs0><3-u><5-rd0><0000011>", "ld").
                asr({
                    {"<000>", "add"},

                    }
                })
                repo.addDecRule("<7-i1-5-12><5-rs1><5-rs0><3-u><5-i1-0-5><0100011>", "st");
                repo.addDecRule("<1-i2-12-13><6-i2-5-11><5-rs1><5-rs0><3-u><4-i2-1-5><1-i2-11-12><1100011>", "br");
                ////// 001 jalr
                repo.addDecRule("<12-rs1-0-12><5-rs0><000><5-rd0><1100111>", "jalr");
                ////// 011 jal
                repo.addDecRule("<1-i1-20-21><10-i1-1-11><1-i1-11-12><8-i1-12-19><5-rd0><1101111>", "jal");
                ////// 100 op/opImm
                repo.addDecRule("<7-u><5-rs1><5-rs0><3-u><5-rd0><0110011>", "op");
                repo.addDecRule("<12-i1-0-12><5-rs0><3-u><5-rd0><0010011>", "opImm");
                ////// 101 auipc/lui
                repo.addDecRule("<20-i1-12-32><5-rd0><0010111>", "auipc");
                repo.addDecRule("<20-i1-12-32><5-rd0><0100011>", "lui");


                repo.addSubDecRule({{"<0000000><000>","add", "op"},{"<0100000><000>","sub", "op"},
                             {"<0000000><001>","sll", "op"},{"<0000000><010>","slt", "op"},
                             {"<0000000><011>","sltu","op"},{"<0000000><100>","xor", "op"},
                             {"<0000000><101>","srl" ,"op"},{"<0100000><101>","sra", "op"},
                             {"<0000000><110>","or"  ,"op"},{"<0000000><111>","and", "op"}});

                repo.addSubDecRule({{"<000>","addi" , "opImm"}, {"<010>","slti", "opImm"},
                             {"<011>","sltiu", "opImm"}, {"<100>","xori", "opImm"},
                             {"<110>","ori"  , "opImm"}, {"<111>","andi", "opImm"},
                             {"<001>","shl"  , "opImm"}, {"<101>","shr" , "opImm"}});

                repo.addSubDecRule({{"<000>", "sb", "st"},{"<001>", "sh", "st"},{"<010>", "sw", "st"}});

                repo.addSubDecRule({{"<000>", "lb", "ld"}, {"<001>", "lh", "ld"}, {"<010>", "lw", "ld"},
                             {"<100>", "lbu", "ld"},{"<101>", "lhu", "ld"}});

                repo.addSubDecRule({{"<000>", "beq", "br"},{"<001>", "bne" , "br"},{"<100>", "blt" , "br"},
                             {"<101>", "bge", "br"},{"<110>", "bltu", "br"},{"<111>", "bgeu", "br"}
                });

                repo.processToken();
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


//        enum RISCV_OP{
//            /*** mem memory related opcode */
//            RS_LOAD    = 0b00'000,
//            RS_STORE   = 0b01'000,
//            RS_MISCMEM = 0b00'011,
//            /***  branch instruction*/
//            RS_BRANCH  = 0b11'000,
//            RS_JALR    = 0b11'001,
//            RS_JAL     = 0b11'011,
//            /***  arithmthic instruction*/
//            RS_OP_IMM  = 0b00'100,
//            RS_OP      = 0b01'100,
//
//            RS_AUIPC   = 0b00'101,
//            RS_LUI     = 0b01'101,
//            /**  system instruction*/
//            RS_SYSTEM   = 0b11'100,
//        };

