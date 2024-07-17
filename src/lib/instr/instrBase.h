//
// Created by tanawin on 9/7/2024.
//

#ifndef INSTRREP_H
#define INSTRREP_H
#include <vector>
#include <map>
#include <set>
#include <utility>


#include "model/hwComponent/register/register.h"
#include "model/hwComponent/abstract/Slice.h"
#include "instrEle.h"


namespace kathryn{
    // "<7-op3><5-r2><1-IMMZ><5-r1>0011100011"

    /***  the hardware stucture*/

    struct InstrRepoDebugMsg{
        std::string mopName;
        std::string uopName;
        std::string errorCause;
    };

    struct OPR_HW{
        bool _isSrc = true;
        Reg& valid;
        Reg& data;
        Reg& idx;

        explicit OPR_HW(int archSize, int idxSize, int regNo, bool isSrc);
        void reset();
        void setOnlyIndex(Operable* index); ///// the value must get manual from regfile
        void setImm(Operable* value); ///// the value will be added and valid

        OPR_HW& operator <<=(const OPR_HW& rhs){
            valid <<= rhs.valid;
            data  <<= rhs.data;
            idx   <<= rhs.idx;
            return *this;
        }
    };
    struct OP_HW{
        int _mopIdx = -1;
        std::string _mopName;
        Reg& _set;
        std::vector<Reg*> _uopSets;
        std::map<std::string, int> uopMapIdx;
        explicit OP_HW(int mopIdx,
                       const std::string& mopName,
                       std::vector<std::string>& uops);
        ////// operation on reg
        void set();
        void setUop(const std::string& idx, Operable* condition);
        void reset();
        ////// get value
        Reg& isSet();
        Reg& isUopSet(const std::string& uopName);

        bool isThereUop(const std::string& uopName);
    };
    struct MOP_META{
        std::string mopName;
        std::vector<std::string> uopNames;

        [[nodiscard]] bool checkValid() const;
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
        TOKEN_GRP _prefixCheck;
        std::vector<MasterRule> masterRules;
        std::vector<MOP_META>   mopMetas;
        ////// hardware component
        std::vector<OPR_HW*> srcOprs;
        std::vector<OPR_HW*> desOprs;
        std::vector<OP_HW* > opcodes; //// index is type idx
        std::map<std::string, int> opcodeMap;
        ////// index in uop is uopIdx as well
        std::pair<
            std::vector<MasterRule*>,
            std::vector<MasterRule*>
        >    seperateMopByopcode(std::vector<MasterRule*>& msrs, int bitIdx); ///// master rules
        void genDecInternal     (std::vector<MasterRule*>& msrs, int bitIdx);

    public:

        explicit      InstrRepo(int instrWidth, int amtSrcOpr,
                                int amtDesOpr , int oprWidth,
                                Operable* instr);
        virtual       ~InstrRepo();
        void          addMop(const MOP_META& mopMeta);
        //////// add rule
        MasterRule&   addDecRule(const std::string& workOnMopName,
                                 const std::string& rule);
        void          addFixPrefix(const std::string& preFixRule);
        //////// hardware declaration
        void          declareHw(); ////// define when addMop is added that ok;
        //////// gen the logic
        virtual  void genDecodeLogic(); ////// gen logic

        //// src reg
        OPR_HW& getSrcReg   (int idx);
        int     getAmtSrcReg() const{return _amtSrcOpr;}
        OPR_HW& getDesReg   (int idx);
        int     getAmtDesReg() const{return _amtDesOpr;}
        OP_HW&  getOp       (int typeId);
        OP_HW&  getOp       (const std::string& mopIdx);
        int     getAmtMop   ()     const{return (int)opcodes.size();}


        Operable* getInstrOpr() const{return _instr;}
        int     getInstrSize () const{return INSTR_WIDTH;}
        int     getOprSize()    const{return OPR_WIDTH;}

        ////////// for debug msg
        ///
        InstrRepoDebugMsg getGetDbgMsg();
        bool isThereOpDec(const std::string& opName);
        bool isThereUopDec(std::string opName, const std::string& uopName);

    };

}

#endif //INSTRREP_H
