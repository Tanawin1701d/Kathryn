//
// Created by tanawin on 2/4/2025.
//

#include "dataType.h"

namespace kathryn{


    RowMeta instrMeta(
            {"instr1" , "instr2", "prCond", "invl1", "invl2",
             "bhr", "pc", "npc"},
            {INSTR_LEN, INSTR_LEN, 1, 1, 1,
             GSH_BHR_LEN, ADDR_LEN, ADDR_LEN}
             //////// the npc is the next instruction from the last instruction that get predict from the btb or test system
             ////////
            );

    RowMeta intDecMeta(
            {"illInstr", "aluOp"   , "rsEntry",          ////// microop
             "immType" , "srcASel" , "srcBSel",  ////// operand meta data
             "regWr"   , "useRs1"  , "useRs2"          ////// operand enabler
            },
            {1            , ALU_OP_WIDTH   , RS_ENT_SEL,
             IMM_TYPE_WIDTH, SRC_A_SEL_WIDTH, SRC_A_SEL_WIDTH,
             1             , 1              , 1});

    RowMeta rrfMeta(
            {"rs1Idx", "rs2Idx" , "rdIdx"},
            {  ARF_SEL,   ARF_SEL, ARF_SEL});

    RowMeta memMeta(
            {"dmemSize" , "dmemType"},
            {MEM_TYPE_WIDTH, MEM_TYPE_WIDTH});

    RowMeta mdMeta(
            {"mdOp", "isMd1Sig", "isMd2Sig", "mdOutSel"},
            {MD_OP_WIDTH, 1, 1, MD_OUT_SEL_WIDTH });

    RowMeta decMeta(
            {"invl", "instr"  , "isBranch", "prCond", "pAddr"  , "specTag"   }, ///// prCond if branch is predict (taken or not taken)
            {1     , INSTR_LEN, 1         , 1       , INSTR_LEN, SPEC_TAG_LEN});

    RowMeta specGenMeta(
        {"res1", "res2"},
        {SPEC_TAG_LEN, SPEC_TAG_LEN});

    //// the base class of out of order reservation station entry
    RowMeta OORsvEntry({"busy"}, {1});
    //////// the occupy and ready signal will be assigned in the base table
    RowMeta intRsvEntry(
        {"specTag", "pc", "imm", "rrfTag", "aluOp",
            "srcASel",  "valid1", "src1",
            "srcBSel",  "valid2", "src2"},
        {SPEC_TAG_LEN, 1, INSTR_LEN, DATA_LEN, RRF_SEL, ALU_OP_WIDTH,
            SRC_A_SEL_WIDTH, 1, DATA_LEN,
            SRC_A_SEL_WIDTH, 1, DATA_LEN ////// the address is embbed in src1 and src2
        }
    );


    RowMeta joinDecMeta(decMeta + intDecMeta + rrfMeta + memMeta + mdMeta);


    void D_DECODE::decode(D_INSTR& instrMeta, int decIdx) {

        auto& fetchMeta     = instrMeta.fetchMeta;
        auto& instr = fetchMeta.get("instr" + std::to_string(decIdx+1));
        auto& dec = decs[decIdx];


        //////// assign normal meta
        auto& op     = instr(0, 7);
        auto& funct7 = instr(31,25);
        auto& funct12= instr(31,20);
        auto& funct3 = instr(14,12);

        auto intDec = dec(intDecMeta);
        auto mdDec  = dec(mdMeta);
        auto memDec = dec(memMeta);
        auto rrfDec = dec(rrfMeta);
        auto metaDec = dec(decMeta);

        ////// mem decoder zone
        memDec <<= {&funct3(0,2).extB(3), &funct3};
        ////// register decoder zone
        rrfDec <<= {&instr(15, 20), &instr(20, 25), &instr(7, 12)};

        ////// decode mete decoder zone
        if (!decIdx) { ////// id is 0
            metaDec({"invl", "instr", "isBranch", "prCond", "pAddr"}) <<=
                    {&fetchMeta.get("invl1"),
                     &instr,
                     &isBranches[decIdx],
                     &(isBranches[0] & fetchMeta.get("prCond")),
                     &fetchMeta.get("npc")};
            zif(isBranches[decIdx] & fetchMeta.get("prCond")){
                metaDec({"pAddr"}) <<= fetchMeta.get("npc");
            }zelse{
                metaDec({"pAddr"}) <<= fetchMeta.get("pc") + 4;
            }

        }else
            metaDec({"invl", "instr", "isBranch", "prCond", "pAddr"}) <<=
                {&(fetchMeta.get("invl2") | ((isBranches[0]) & fetchMeta.get("prCond"))), ///// invalid due to fetcher tell that it is invalid or (prev instruction is branch adn that branch is taken)
                 &instr,
                 &isBranches[decIdx],
                 &((isBranches[1]) & (~isBranches[0]) & fetchMeta.get("prCond")),
                 &fetchMeta.get("npc")};


        ////// INT DEC
        zif(op == RV32_LOAD)
            intDec({"rsEntry", "regWr"}) <<= {RS_ENT_LDST, 1};
        zif(op == RV32_STORE)
            intDec({"rsEntry", "immType", "useRs2"}) <<= {RS_ENT_LDST, IMM_S, 1};
        zif(op == RV32_BRANCH){
            mWire(sb, 1); sb = 1;
            isBranches[decIdx] = sb; ///// TODO beware this
            intDec({"rsEntry", "srcBSel", "useRs2"}) <<= {RS_ENT_BRANCH, SRC_B_RS2, 1};
            zif(funct3 == RV32_FUNCT3_BEQ  ) intDec({"aluOp"}) <<= ALU_OP_SEQ;
            zif(funct3 == RV32_FUNCT3_BNE  ) intDec({"aluOp"}) <<= ALU_OP_SNE;
            zif(funct3 == RV32_FUNCT3_BLT  ) intDec({"aluOp"}) <<= ALU_OP_SLT;
            zif(funct3 == RV32_FUNCT3_BLTU ) intDec({"aluOp"}) <<= ALU_OP_SLTU;
            zif(funct3 == RV32_FUNCT3_BGE  ) intDec({"aluOp"}) <<= ALU_OP_SGE;
            zif(funct3 == RV32_FUNCT3_BGEU ) intDec({"aluOp"}) <<= ALU_OP_SGEU;
            intDec({"illInstr"}) <<= ALU_OP_SGEU;
        }
        zif(op == RV32_JAL)
            intDec({"rsEntry", "srcASel" , "srcBSel", "regWr", "useRs1"}) <<=
                {RS_ENT_JAL, SRC_A_PC, SRC_B_FOUR, 1, 0};

        zif(op == RV32_JALR) {
            intDec({"rsEntry", "srcASel", "srcBSel", "regWr"}) <<=
                    {RS_ENT_JAL, SRC_A_PC, SRC_B_FOUR, 1};
            intDec({"illInstr"}) <<= (funct3 != 0);
        }

        zif( (op == RV32_OP_IMM) || (op == RV32_OP)){
            intDec({"regWr"}) <<= 1;
            zif(op == RV32_OP){
                intDec({"srcBSel", "useRs2"}) <<= {SRC_B_RS2, 1};
                zif(funct7 == RV32_FUNCT7_MUL_DIV){
                    zif ((funct3 == RV32_FUNCT3_MUL) ||
                         (funct3 == RV32_FUNCT3_MULH) ||
                         (funct3 == RV32_FUNCT3_MULHSU) ||
                         (funct3 == RV32_FUNCT3_MULHU)) {
                        intDec({"rsEntry"}) <<= RS_ENT_MUL;
                    }zelse{intDec({"rsEntry"}) <<= RS_ENT_DIV;}
                }
            }

            zif((op == RV32_OP) && (funct3 == RV32_FUNCT3_ADD_SUB) &&funct7(5)){
                intDec({"aluOp"}) <<= ALU_OP_SUB;
            }zelse{intDec({"aluOp"}) <<= ALU_OP_ADD;}

            zif((funct3 == RV32_FUNCT3_SRA_SRL) && funct7(5)){
                intDec({"aluOp"}) <<= ALU_OP_SRA;
            }zelse{intDec({"aluOp"}) <<= ALU_OP_SRL;};

            zif(funct3 == RV32_FUNCT3_SLL    ) intDec({"aluOp"}) <<= ALU_OP_SLL;
            zif(funct3 == RV32_FUNCT3_SLT    ) intDec({"aluOp"}) <<= ALU_OP_SLT;
            zif(funct3 == RV32_FUNCT3_SLTU   ) intDec({"aluOp"}) <<= ALU_OP_SLTU;
            zif(funct3 == RV32_FUNCT3_XOR    ) intDec({"aluOp"}) <<= ALU_OP_XOR;
            zif(funct3 == RV32_FUNCT3_OR     ) intDec({"aluOp"}) <<= ALU_OP_OR;
            zif(funct3 == RV32_FUNCT3_AND    ) intDec({"aluOp"}) <<= ALU_OP_AND;

        }
        intDec <<= {0    , ALU_OP_ADD, RS_ENT_ALU,
                    IMM_I, SRC_A_RS1 , SRC_B_IMM,
                    0    , 1         , 0};


        //////////// mul and div decoder zone

        ////// mul no need to be asisgn any Further
        zif(funct3 == RV32_FUNCT3_MULH   ) mdDec <<= {MD_OP_MUL,1,1,MD_OUT_HI};
        zif(funct3 == RV32_FUNCT3_MULHSU ) mdDec <<= {MD_OP_MUL,1,0,MD_OUT_HI};
        zif(funct3 == RV32_FUNCT3_MULHU  ) mdDec <<= {MD_OP_MUL,0,0,MD_OUT_HI};

        zif(funct3 == RV32_FUNCT3_DIV  ) mdDec <<= {MD_OP_DIV, 1, 1, MD_OUT_LO};
        zif(funct3 == RV32_FUNCT3_DIVU ) mdDec({"mdOp"}) <<= MD_OP_DIV;
        zif(funct3 == RV32_FUNCT3_REM  ) mdDec <<= {MD_OP_REM, 1, 1, MD_OUT_REM};
        zif(funct3 == RV32_FUNCT3_REMU ) mdDec <<= {MD_OP_REM, 0, 0, MD_OUT_REM};

        std::vector<uint64_t> defMulVals = {MD_OP_MUL, 0, 0, MD_OUT_LO};
        mdDec <<= defMulVals;


    }

}