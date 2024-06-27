//
// Created by tanawin on 20/6/2024.
//


#ifndef KATHRYN_GEN_PROXYHWCOMP_ABSTRCT
#define KATHRYN_GEN_PROXYHWCOMP_ABSTRCT

#include <utility>
#include "util/fileWriter/fileWriterBase.h"
#include "model/hwComponent/abstract/assignable.h"
#include "logicCerf.h"
#include "model/hwComponent/abstract/globIo.h"

namespace kathryn{

    class ModuleGen;
    class LogicGenBase{
    protected:
        ModuleGen*     _mdGenMaster = nullptr;
        logicLocalCef  _cerf{};
        Assignable*    _asb = nullptr;
        Identifiable*  _ident = nullptr;

    public:
        explicit LogicGenBase(ModuleGen*    mdGenMaster,
                              logicLocalCef cerf,
                              Assignable*   asb,
                              Identifiable* ident
        );
        virtual ~LogicGenBase() = default;
        std::string getOprStrFromOpr(Operable* opr);
        ///////// routing zone
        virtual void routeDep() {assert(false);} ///// do routing
        ///////// get zone
        virtual std::string getOpr();
        virtual std::string getOpr(Slice sl);
        //////// gen zone
        virtual std::string  decGlobIo()   {assert(false);}
        virtual std::string  decGlobIoAsm(){assert(false);}
        virtual std::string  decIo()       {assert(false);}
        virtual std::string  decVariable() {assert(false);}
        virtual std::string  decOp()       {assert(false);}
        /////// glob io check
        virtual GLOB_IO_TYPE getGlobIoStatus(){return GLOB_IO_TYPE::GLOB_IO_NOT_BOTH;}


        ///////// getter
        [[nodiscard]] ModuleGen* getModuleGen() const{
            assert(_mdGenMaster != nullptr);
            return _mdGenMaster;
        }
        [[nodiscard]] logicLocalCef getLogicCef() const{
            return _cerf;
        }

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
    };

    inline void writeGenVec(const std::vector<std::string>& src,
                     FileWriterBase* fw,
                     const std::string& sep){
        assert(fw != nullptr);
        bool isFirst = true;
        for (const std::string& x: src){
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
