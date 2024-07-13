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

    /***  the hardware stucture*/

    struct OPR_HW{
        bool _isSrc = true;
        Reg& valid;
        Reg& data;
        Reg& idx;

        explicit OPR_HW(int archSize, int idxSize, int regNo, bool isSrc);
        void reset();
        void setOnlyIndex(Operable* index); ///// the value must get manual from regfile
        void setImm(Operable* value); ///// the value will be added and valid
    };

    struct RULE;
    struct OP_HW{
        int _mopIdx = -1;
        std::string _mopName;
        Reg& _set;
        std::vector<Reg*> _uopSets;
        explicit OP_HW(int mopIdx,
                       const std::string& mopName,
                       std::vector<UopAsm>& uops);

        void set();
        void setUop(int idx, Operable* condition);
        void reset();
    };

    /***  rule meta data*/

    struct RULE{
        int         _mopIdx = -1;
        std::string _name;
        std::string _rule;
        std::vector<token> _tokens;

        RULE    (int mopIdx, std::string  name,std::string  rule);
        void    startTokeniz();
    };

    struct UOP_INPUT_META{
        std::string rule;
        std::string ruleName;
        int mopIdx;
    };




    class InstrRepo{
    protected:
        Operable* _instr = nullptr;
        ///// we don't allow to add rule any more
        ///// meta data
        const int INSTR_WIDTH    = -1;
        const int OPR_WIDTH      = -1;
              int _amtMopType    = -1;
        const int _amtSrcOpr     = -1;
        const int _amtDesOpr     = -1;
        std::vector<MOP>    mops;
        std::vector<RULE>   mopRules;
        std::vector<RULE>   uopRules;
        ////// hardware component
        std::vector<OPR_HW*> srcOprs;
        std::vector<OPR_HW*> desOprs;
        std::vector<OP_HW* > opcodes; //// index is type idx
        ////// index in uop is uopIdx as well
        std::pair<
            std::vector<MOP*>,
            std::vector<MOP*>
        >    seperateMopByopcode(std::vector<MOP*>& mops, int bitIdx);
        void genDecInternal     (std::vector<MOP*>& uops, int bitIdx);

    public:

        explicit      InstrRepo(int instrWidth, int amtSrcOpr,
                                int amtDesOpr , int oprWidth,
                                Operable* instr);
        virtual       ~InstrRepo();
        void          addMop(const std::string& rule,
                             const std::string& ruleName);
        void          addUop(std::vector<UOP_INPUT_META> uopMetas);
        void          processToken();
        void          declareHw();
        virtual  void genDecodeLogic();

        //// src reg

        OPR_HW& getSrcReg(int idx);
        int     getAmtSrcReg() const{return _amtSrcOpr;}
        OPR_HW& getDesReg(int idx);
        int     getAmtDesReg() const{return _amtDesOpr;}
        OP_HW&  getOp    (int typeId);
        int     getAmtMop()     const{return (int)mops.size();}

        Operable* getInstrOpr() const{return _instr;}
        int     getInstrSize () const{return INSTR_WIDTH;}

        int     getOprSize()   const{return OPR_WIDTH;}

    };

}

#endif //INSTRREP_H
