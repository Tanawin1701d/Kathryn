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

            repo.addMop({"rType", {"add", "sll", "sltu", "sub", "slt", "xor"}});

            repo.addDecRule("rType", "<7-ub><5-rs1><5-rs0><3-ua><5-rd0><0110011>").
            ad("<ub><ua>",{{"<0000000><000>","add" },{"<0100000><000>","sub"},
                           {"<0000000><001>","sll" },{"<0000000><010>","slt"},
                           {"<0000000><011>","sltu"},{"<0000000><100>","xor"}});

            repo.addDecRule("rType", "<12-i1-0-12><5-rs0><3-ua><5-rd0><0010011>").
            ad("<ua>"    ,{{"<000>", "add"},{"<100>", "xor"}});

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

            OP_HW& opHw =  _md->repo.getOp("rType");

            _md->instr.s(0b0000000'01000'00010'010'00100'0110011);
            conNextCycle(1);

            std::cout << "------------ test slt rType" << std::endl;
            testAndPrint("src0Valid",(ull)_md->repo.getSrcReg(0).valid, 0);
            testAndPrint("src0idx"  ,(ull)_md->repo.getSrcReg(0).idx  , 2);

            testAndPrint("src1Valid",(ull)_md->repo.getSrcReg(1).valid, 0);
            testAndPrint("src1idx"  ,(ull)_md->repo.getSrcReg(1).idx  , 8);

            testAndPrint("des1Valid",(ull)_md->repo.getDesReg(0).valid, 0);
            testAndPrint("des1idx"  ,(ull)_md->repo.getDesReg(0).idx  , 4);

            /////////// test

            testAndPrint("mopSet", (ull)opHw._set, 1);
            for (auto[uopName, idx]: opHw.uopMapIdx){
                if (uopName == "slt"){
                    testAndPrint("testUop", (ull)*opHw._uopSets[idx], 1);
                }else{
                    testAndPrint("testUop false", (ull)*opHw._uopSets[idx], 0);
                }
            }



            _md->instr.assignSimValue(0b100100000000'00101'100'01010'0010011);
            conNextCycle(1);
            std::cout << "------------ test xor iType" << std::endl;
            testAndPrint("src0Valid",(ull)_md->repo.getSrcReg(0).valid, 0);
            testAndPrint("src0idx"  ,(ull)_md->repo.getSrcReg(0).idx  , 5);

            testAndPrint("src1Valid",(ull)_md->repo.getSrcReg(1).valid, 1);
            testAndPrint("src1idx"  ,(ull)_md->repo.getSrcReg(1).idx  , 0);
            testAndPrint("src1data" ,(ull)_md->repo.getSrcReg(1).data , 4294965504);

            testAndPrint("des1Valid",(ull)_md->repo.getDesReg(0).valid, 0);
            testAndPrint("des1idx"  ,(ull)_md->repo.getDesReg(0).idx  , 10);

            testAndPrint("mopSet", (ull)opHw._set, 1);
            for (auto[uopName, idx]: opHw.uopMapIdx){
                if (uopName == "xor"){
                    testAndPrint("testUop", (ull)*opHw._uopSets[idx], 1);
                }else{
                    testAndPrint("testUop false", (ull)*opHw._uopSets[idx], 0);
                }
            }

            _md->instr.assignSimValue(0b0000000'01010'00111'000'10000'0110011);
            conNextCycle(1);

            std::cout << "------------ test add rType" << std::endl;
            testAndPrint("src0Valid",(ull)_md->repo.getSrcReg(0).valid, 0);
            testAndPrint("src0idx"  ,(ull)_md->repo.getSrcReg(0).idx  , 7);

            testAndPrint("src1Valid",(ull)_md->repo.getSrcReg(1).valid, 0);
            testAndPrint("src1idx"  ,(ull)_md->repo.getSrcReg(1).idx  , 10);

            testAndPrint("des1Valid",(ull)_md->repo.getDesReg(0).valid, 0);
            testAndPrint("des1idx"  ,(ull)_md->repo.getDesReg(0).idx  , 16);


            testAndPrint("mopSet", (ull)opHw._set, 1);
            for (auto[uopName, idx]: opHw.uopMapIdx){
                if (uopName == "add"){
                    testAndPrint("testUop", (ull)*opHw._uopSets[idx], 1);
                }else{
                    testAndPrint("testUop false", (ull)*opHw._uopSets[idx], 0);
                }
            }



        }
    };

    class Sim36TestEle: public AutoTestEle{
    public:
        explicit Sim36TestEle(int id): AutoTestEle(id){}
        void start(std::string prefix) override{
            mMod(d, testSimMod36, 1);
            startModelKathryn();
            sim36 simulator((testSimMod36*) &d, _simId, prefix);
            //std::cout << getGlobalModulePtr()->getUserWires()[0]->getVarName() << std::endl;
            //std::cout << getGlobalModulePtr()->getUserWires()[0]->getSimEngine()->getVarName() << std::endl;
            simulator.simStart();
        }

    };

    Sim36TestEle ele36(36);

}
