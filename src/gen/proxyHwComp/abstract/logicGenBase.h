//
// Created by tanawin on 20/6/2024.
//


#ifndef KATHRYN_GEN_PROXYHWCOMP_ABSTRCT
#define KATHRYN_GEN_PROXYHWCOMP_ABSTRCT

#include <utility>
#include "util/fileWriter/fileWriterBase.h"
#include "model/hwComponent/abstract/assignable.h"
#include "cerf.h"
#include "model/hwComponent/abstract/WireMarker.h"

namespace kathryn{

    enum OUT_SEARCH_POL{
        SUBMOD,
        MASTERMOD
    };

    /** compare Stategy
     * CP_LC_ONLY
     * CP_
     */
    class ModuleGen;
    class LogicGenBase{
    protected:
        ModuleGen*     _mdGenMaster = nullptr;
        logicLocalCef  _cerf{};
        Assignable*    _asb = nullptr;
        Identifiable*  _ident = nullptr;

    public:
        explicit LogicGenBase(ModuleGen*    mdGenMaster,
                              Assignable*   asb,
                              Identifiable* ident
        );
        virtual ~LogicGenBase() = default;
        std::string getOprStrFromOpr(Operable* opr);
        ///////// routing zone
        virtual void routeDep() {assert(false);} ///// do routing
        ///////// generate cerificate
        virtual void genCerf(MODULE_GEN_GRP mgg, int grpIdx, int idx);
        ///////// start compare
        virtual bool compare(LogicGenBase* lgb) = 0;
        ///////// get zone
        virtual std::string getOpr();
        virtual std::string getOpr(Slice sl);
        //////// gen zone
        virtual std::string  decGlobIo()   {assert(false);}
        virtual std::string  decGlobIoAsm(){assert(false);}
        virtual std::string  decParamVal() {assert(false);}
        virtual std::string  decIo()       {assert(false);}
        virtual std::string  decVariable() {assert(false);}
        virtual std::string  decOp()       {assert(false);}
        /////// glob io check
        virtual WIRE_MARKER_TYPE  getGlobIoStatus(){return WIRE_MARKER_TYPE::WMT_NONE;}

        bool checkCerfEqLocally(const LogicGenBase& rhsGenBase);
        bool cmpEachOpr(Operable*  exactSrcA, Operable*  exactSrcB,
                        ModuleGen* srcMdA,    ModuleGen* srcMdB,
                        OUT_SEARCH_POL searchPol);

        ///////// getter
        [[nodiscard]] ModuleGen* getModuleGen() const{
            assert(_mdGenMaster != nullptr);
            return _mdGenMaster;
        }
        [[nodiscard]] logicLocalCef getLogicCef() const{return _cerf;}
        [[nodiscard]] Identifiable* getIdent()    const {return _ident;}

        virtual void addDirectUpdateEvent(UpdateEvent* updateEvent){assert(false);}

    };


    class LogicGenBaseVec: public std::vector<LogicGenBase*>{
    public:
        void routeDepAll(){
            for (auto& x : *this){
                x->routeDep();
            }
        }

        std::vector<std::string> getOprs(){
            std::vector<std::string> result;
            for (auto& x : *this){
                result.push_back(x->getOpr());
            }
            return result;
        }

        std::vector<std::string> getDecIos(){
            std::vector<std::string> result;
            for (auto& x : *this){
                result.push_back(x->decIo());
            }
            return result;
        }

        std::vector<std::string> getDecVars(){
            std::vector<std::string> result;
            for (auto& x : *this){
                result.push_back(x->decVariable());
            }
            return result;
        }

        std::vector<std::string> getDecOps(){
            std::vector<std::string> result;
            for (auto& x : *this){
                result.push_back(x->decOp());
            }
            return result;
        }

        void genCerf(MODULE_GEN_GRP mgg, int grp){
            int idx = 0;
            for (auto& x: *this){
                x->genCerf(mgg, grp, idx);
                idx++;
            }
        }

        bool compare(LogicGenBaseVec& lgbVec){
            if (size() != lgbVec.size()){
                return false;
            }
            bool cmpRes = true;
            for (int idx = 0; idx < size(); idx++){
                cmpRes &= (*this)[idx]->compare(lgbVec[idx]);
            }
            return cmpRes;
        }

        bool compareCefOnly(LogicGenBaseVec& lgbVec){
            if (size() != lgbVec.size()){
                return false;
            }
            bool cmpRes = true;
            for (int idx = 0; idx < size(); idx++){
                cmpRes &= (*this)[idx]->checkCerfEqLocally(*lgbVec[idx]);
            }
            return cmpRes;
        }
    };

    inline void writeGenVec(const std::vector<std::string>& src,
                     FileWriterBase* fw,
                     const std::string& sep){
        assert(fw != nullptr);
        bool isFirst = true;
        for (const std::string& x: src){
            if (x.empty()){
                continue;
            }
            if(!isFirst){
                fw->addData(sep);
            }
            fw->addData(x);
            isFirst = false;
        }
    }

    class LogicGenInterface{
    public:
        virtual               ~LogicGenInterface() = default;
        virtual void          createLogicGen()     = 0;
        virtual LogicGenBase* getLogicGen()        = 0;
    };

}


#endif
