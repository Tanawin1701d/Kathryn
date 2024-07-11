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
        Reg& data;
        Reg& idx;
        Reg& valid;

        explicit OPR_HW(int archSize, int idxSize, int regNo);
        void reset();
        void setOnlyIndex(Operable* opr); ///// the value must get manual from regfile
        void setImm(Operable* opr); ///// the value will be added and valid
    };

    struct RULE;
    struct OP_HW{
        int _typeIdx = -1;
        std::string _typeName;
        Reg& _set;
        std::vector<Reg*> _uopSets;
        explicit OP_HW(int typeId,
                       const std::string& typeName,
                       std::vector<UopAsm>& uops);

        void set();
        void setUop(int idx, Operable* condition);
        void reset();
    };

    /***  rule meta data*/

    struct RULE{
        int         _typeIdx = -1;
        int         _uopIdx    = -1;
        std::string _name;
        std::string _rule;
        std::vector<token> _tokens;

        RULE    (int typeIdx, int uopIdx,
                 std::string  name,std::string  rule);
        void    startTokeniz(int instrBitSize);
        bool    operator < (const RULE& rhs) const{ return _typeIdx < rhs._typeIdx;}
    };



    class InstrRepo{
    protected:
        Operable* _instr = nullptr;
        bool decodeLock = false; //// if startTokenizing
        ///// we don't allow to add rule any more
        ///// meta data
        const int INSTR_SIZE    = -1;
        const int OPR_SIZE      = 32;
        const int _amtInstrType = -1;
        const int _amtSrcOpr   = -1;
        const int _amtDesOpr   = -1;
        std::vector<MOP>  mops;
        std::vector<RULE> rules;
        ////// hardware component
        std::vector<OPR_HW*> srcOprs;
        std::vector<OPR_HW*> desOprs;
        std::vector<OP_HW* > opcodes; //// index is type idx
        ////// index in uop is uopIdx as well
        std::pair<
            std::vector<UopAsm*>,
            std::vector<UopAsm*>
        >
            seperateUopByopcode(std::vector<UopAsm*>& uops, int bitIdx);
        void genDecInternal(std::vector<UopAsm*>& uops, int bitIdx);

    public:

        explicit      InstrRepo(int instrSize, int amtInstrType,
                                int amtSrcOpr, int amtDesOpr,
                                int oprSize,
                                Operable* instr);
                      ~InstrRepo();
        void          addRule(const std::string& rule,
                              const std::string& ruleName,
                              int typeIdx,int uopIdx);
        void          convertRuleToMop();
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
        int     getInstrSize () const{return INSTR_SIZE;}

        int     getOprSize()   const{return OPR_SIZE;}




        //// hardware operation helper



    };








}

#endif //INSTRREP_H
