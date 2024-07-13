//
// Created by tanawin on 22/1/2567.
//

#include "kathryn.h"
#include "test/autoSim/simAutoInterface.h"
#include "autoTestInterface.h"
#include "lib/instr/instrBase.h"

namespace kathryn{

    class testSimMod36: public Module{
    public:
        mReg(instr, 32);
        InstrRepo repo;

        explicit testSimMod36(int x): Module(),
        repo(32,3, 1, 32, &instr){
            repo.addMop("<7-u><5-rs1><5-rs0><3-u><5-rd0><0110011>", "rType");
            repo.addMop("<12-i1-0-12><5-rs0><3-u><5-rd0><0010011>", "iType");

            repo.addUop({{"<0000000><000>","add", 0},{"<0100000><000>","sub", 0},
                         {"<0000000><001>","sll", 0},{"<0000000><010>","slt", 0},
                         {"<0000000><011>","sltu",0},{"<0000000><100>","xor", 0}});

            repo.addUop({{"<000>", "addi", 1}, {"<100>", "slti", 1}});
            repo.processToken();
            repo.declareHw();
        }

        void flow() override{
            cwhile(true){
                repo.genDecodeLogic();
            }
        }

    };

    class sim36 :public SimAutoInterface{
    public:

        testSimMod36* _md = nullptr;

        sim36(testSimMod36* md, int idx, const std::string& prefix):SimAutoInterface(idx,100,
                                                                                  prefix + "simAutoResult"+std::to_string(idx)+".vcd",
                                                                                  prefix + "simAutoResult"+std::to_string(idx)+".prof"),
        _md(md)
        {}
        void simAssert() override{}
        void simDriven() override{}

        void describeCon() override{

            _md->instr.assignSimValue(0b0000000'01000'00010'010'00100'0110011);
            conNextCycle(1);
            testAndPrint("src0Valid",(ull)_md->repo.getSrcReg(0).valid, 0);
            testAndPrint("src0idx"  ,(ull)_md->repo.getSrcReg(0).idx  , 2);

            testAndPrint("src1Valid",(ull)_md->repo.getSrcReg(1).valid, 0);
            testAndPrint("src1idx"  ,(ull)_md->repo.getSrcReg(1).idx  , 8);

            testAndPrint("des1Valid",(ull)_md->repo.getDesReg(0).valid, 0);
            testAndPrint("des1idx"  ,(ull)_md->repo.getDesReg(0).idx  , 4);

            testAndPrint("mopSet",(ull)_md->repo.getOp(0)._set, 1);
            for (int uop = 0; uop < 6; uop++){
                if (uop == 3){
                    testAndPrint("uopSet",(ull)*_md->repo.getOp(0)._uopSets[uop], 1);
                }else{
                    testAndPrint("uopSet",(ull)*_md->repo.getOp(0)._uopSets[uop], 0);
                }
            }
            testAndPrint("mopSet",(ull)_md->repo.getOp(1)._set, 0);



            _md->instr.assignSimValue(0b100100000000'00101'100'01010'0010011);
            conNextCycle(1);
            testAndPrint("src0Valid",(ull)_md->repo.getSrcReg(0).valid, 0);
            testAndPrint("src0idx"  ,(ull)_md->repo.getSrcReg(0).idx  , 5);

            testAndPrint("src1Valid",(ull)_md->repo.getSrcReg(1).valid, 1);
            testAndPrint("src1idx"  ,(ull)_md->repo.getSrcReg(1).idx  , 0);
            testAndPrint("src1data" ,(ull)_md->repo.getSrcReg(1).data , 4294965504);

            testAndPrint("des1Valid",(ull)_md->repo.getDesReg(0).valid, 0);
            testAndPrint("des1idx"  ,(ull)_md->repo.getDesReg(0).idx  , 10);

            testAndPrint("mopSet",(ull)_md->repo.getOp(1)._set, 1);
            for (int uop = 0; uop < 2; uop++){
                if (uop == 1){
                    testAndPrint("uopSet",(ull)*_md->repo.getOp(1)._uopSets[uop], 1);
                }else{
                    testAndPrint("uopSet",(ull)*_md->repo.getOp(1)._uopSets[uop], 0);
                }
            }
            testAndPrint("mopSet",(ull)_md->repo.getOp(0)._set, 0);
            _md->instr.assignSimValue(0b0000000'01010'00111'100'10000'0110011);
            conNextCycle(1);

            testAndPrint("src0Valid",(ull)_md->repo.getSrcReg(0).valid, 0);
            testAndPrint("src0idx"  ,(ull)_md->repo.getSrcReg(0).idx  , 7);

            testAndPrint("src1Valid",(ull)_md->repo.getSrcReg(1).valid, 0);
            testAndPrint("src1idx"  ,(ull)_md->repo.getSrcReg(1).idx  , 10);

            testAndPrint("des1Valid",(ull)_md->repo.getDesReg(0).valid, 0);
            testAndPrint("des1idx"  ,(ull)_md->repo.getDesReg(0).idx  , 16);

            testAndPrint("mopSet",(ull)_md->repo.getOp(0)._set, 1);
            for (int uop = 0; uop < 6; uop++){
                if (uop == 5){
                    testAndPrint("uopSet",(ull)*_md->repo.getOp(0)._uopSets[uop], 1);
                }else{
                    testAndPrint("uopSet",(ull)*_md->repo.getOp(0)._uopSets[uop], 0);
                }
            }
            testAndPrint("mopSet",(ull)_md->repo.getOp(1)._set, 0);

        }
    };

    class Sim36TestEle: public AutoTestEle{
    public:
        explicit Sim36TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix) override{
            mMod(d, testSimMod36, 1);
            startModelKathryn();
            sim36 simulator((testSimMod36*) &d, _simId, prefix);
            simulator.simStart();
        }

    };

    Sim36TestEle ele36(-1);

}
