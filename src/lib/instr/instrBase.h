//
// Created by tanawin on 9/7/2024.
//

#ifndef INSTRREP_H
#define INSTRREP_H
#include <vector>
#include "model/hwComponent/register/register.h"
#include "model/hwComponent/abstract/Slice.h"
#include "instrEle.h"


namespace kathryn{



    // "<7-op3><5-r2><1-IMMZ><5-r1>0011100011"

    struct OPR_META{
        Reg& data;
        Reg& idx;
        Reg& valid;

        explicit OPR_META(int archSize, int idxSize, int regNo);
    };

    struct OP_META{

    };

    struct

    class InstrRepo{
    protected:

        std::vector<OPR_META> srcOperands;
        std::vector<>

        bool decodeLock = false; //// if startTokenizing
        ///// we don't allow to add rule any more
        const int INSTR_SIZE = -1;
        std::vector<std::string> rules;
        std::vector<std::vector<token>> tokenizedRules;
    public:

        explicit      InstrRepo(int instrSize);
        void          addRule(const std::string& rule);
        void          startTokenize();
        void          startGenInstr();
        virtual  void genDecodeLogic();
    };








}

#endif //INSTRREP_H
