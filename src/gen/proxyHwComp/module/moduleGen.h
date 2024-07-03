//
// Created by tanawin on 20/6/2024.
//

#ifndef MODULEGEN_H
#define MODULEGEN_H
#include "unordered_map"
#include "gen/proxyHwComp/expression/exprGen.h"
#include "gen/proxyHwComp/expression/nestGen.h"
#include "gen/proxyHwComp/register/regGen.h"
#include "gen/proxyHwComp/value/valueGen.h"
#include "gen/proxyHwComp/wire/wireGen.h"
#include "model/hwComponent/memBlock/MemBlock.h"
#include "model/hwComponent/register/register.h"
#include "model/hwComponent/wire/wire.h"
#include "model/hwComponent/expression/expression.h"
#include "model/hwComponent/value/value.h"
#include "model/hwComponent/expression/nest.h"
#include "model/flowBlock/abstract/spReg/waitReg.h"
#include "model/hwComponent/wire/wireIo.h"


namespace kathryn{


    enum MODULE_GEN_PROGRESS{
        MGP_UNINIT,
        MGP_INITED_ELE,
        MGP_ROUTED,
        MGP_MARK_PRE_WRITE,
        MGP_MARK_PRE_REDUNDANT
    };


    class Module;
    class GenStructure;
    class ModuleGen{
    protected:
        Module*        _master           = nullptr;
        int            depthFromGlobalModule = 0;
        moduleLocalCef _cerf;
        moduleGlobalCef _globCerf;
        MODULE_GEN_PROGRESS _mgp = MGP_UNINIT;

    public:
        LogicGenBaseVec   _regPool;
        LogicGenBaseVec   _wirePool;
        LogicGenBaseVec   _exprPool;
        LogicGenBaseVec   _nestPool;
        LogicGenBaseVec   _valPool;
        LogicGenBaseVec   _memBlockPool;
        LogicGenBaseVec   _memBlockElePool;
        ///// ioPool it can be use after finalize Route Ele is used
        LogicGenBaseVec   _interWirePool;
        LogicGenBaseVec   _autoInputWirePool;
        LogicGenBaseVec   _autoOutputWirePool;
        LogicGenBaseVec   _globalInputPool;
        LogicGenBaseVec   _globalOutputPool;


        std::unordered_map<Operable*, int>  _interWireMap;
        std::vector<WireIo*>                _interWires;


        std::unordered_map<Operable*, int> _autoInputWireMap;
        std::vector<WireIo*>               _autoInputWires;

        std::unordered_map<Operable*, int> _autoOutputWireMap;
        std::vector<WireIo*>               _autoOutputWires;

        //////// now we didn't use it
        std::vector<Wire*>                 _userDecInputWires;
        std::vector<Wire*>                 _userOutputWires;
        /////// only top module will have this
        std::vector<WireIo*>               _globalInputs;
        std::vector<WireIo*>               _globalOutputs;

        std::vector<ModuleGen*>            _subModulePool;


        explicit ModuleGen(Module* master);

        template<typename T>
        void createLogicGenBase(std::vector<T*>& srcs);

        template<typename T>
        void recruitLogicGenBase(LogicGenBaseVec& des,
                                 std::vector<T*>& srcs){
            for(T* src: srcs){
                LogicGenBase* logicGenBase = src->getLogicGen();
                assert(logicGenBase != nullptr);
                des.push_back(logicGenBase);
            }
        }

        template<typename T>
        LogicGenBaseVec recruitLogicGenBase(std::vector<T*>& srcs){
            LogicGenBaseVec result;
            recruitLogicGenBase(result, srcs);
            return result;
        }

        template<typename T>
        void createAndRecruitLogicGenBase(LogicGenBaseVec& des,
                                 std::vector<T*>& srcs);

        MODULE_GEN_PROGRESS getGenProgress()const {return _mgp;}

        /*
         * main progress
         */

        void startInitEle  ();
        void startRouteEle ();
        void finalizeRouteEle();
        void genCerfAll    (int idx); /// idx that it is submodule

        bool startCmpModule(ModuleGen* rhsMdg, GenStructure* genStructure);
        void startPutToGenSystem(GenStructure* genStructure);
        void startWriteFile(FileWriterBase* fileWriter, GenStructure* genStructure);

        /*
         * routing operation
         */
        WireIo* addAutoWireBase  (Operable* opr,
                                    Operable* realSrc,
                                    std::vector<WireIo*>& ioVec,
                                    std::unordered_map<Operable*, int>& ioMap,
                                    const std::string& wireName,
                                    WIRE_IO_TYPE wireIoType,
                                    bool connectTheWire = true);
        //// input wire
        WireIo* addAutoInputWire          (Operable* opr, Operable* realSrc);
        bool    checkIsThereAutoInputWire (Operable* realSrc);
        WireIo* getAutoInputWire          (Operable* realSrc);
        //// output wire
        WireIo* addAutoOutputWire         (Operable* opr, Operable* realSrc);
        bool    checkIsThereAutoOutputWire(Operable* realSrc);
        WireIo* getAutoOutputWire         (Operable* realSrc);
        //// inter wire
        WireIo* addAutoInterWire          (Operable* realSrc);
        bool    checkIsThereAutoInterWire (Operable* realSrc);
        WireIo* getAutoInterWire          (Operable* realSrc);
        //// route opr
        Operable* routeSrcOprToThisModule (Operable* exactRealSrc);
        int       getDept() const{return depthFromGlobalModule;}
        //// global io
        std::vector<WireIo*>& getGlobalInputs (){return _globalInputs; }
        std::vector<WireIo*>& getGlobalOutputs(){return _globalOutputs;}
        /**
         * file generator
         */
        std::vector<std::string> getIoDec(
            const LogicGenBaseVec& inputVec,
            const LogicGenBaseVec& outputVec,
            const LogicGenBaseVec& globInputVec,
            const LogicGenBaseVec& globOutputVec
        );
        std::string getSubModuleDec(ModuleGen* mdGen, GenStructure* genStructure);
        std::string getOpr();
        /**
         *  cmp function
         */
        void genCerfToEachElement();
        void genCerfToThisModule(int idx);
        bool cmpCerfEqLocally(const ModuleGen& rhs) const;
        moduleLocalCef getCerf() const{return _cerf;}
        moduleGlobalCef getGlobCerf() const{return _globCerf;}
    };

    class ModuleGenInterface{
    public:
        virtual ~ModuleGenInterface() = default;
        virtual void       createModuleGen() = 0;
        virtual ModuleGen* getModuleGen() = 0;

    };

}

#endif //MODULEGEN_H