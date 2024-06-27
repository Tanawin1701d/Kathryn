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


    class Module;
    class ModuleGen{
    private:
        Module* _master           = nullptr;
        int depthFromGlobalModule = 0;
    public:
        LogicGenBaseVec   _regPool;
        LogicGenBaseVec   _wirePool;
        LogicGenBaseVec   _exprPool;
        LogicGenBaseVec   _nestPool;
        LogicGenBaseVec   _valPool;
        LogicGenBaseVec   _memBlockPool;
        LogicGenBaseVec   _memBlockElePool;

        std::vector<ModuleGen*>      _subModulePool;

        LogicGenBaseVec              _inputSubModulePool;
        LogicGenBaseVec              _outputSubModulePool;

        std::unordered_map<Operable*, int> _interWireMap;
        std::vector<WireIo*>                _interWires;

        std::unordered_map<Operable*, int> _autoInputWireMap;
        std::vector<WireIo*>               _autoInputWires;

        std::unordered_map<Operable*, int> _autoOutputWireMap;
        std::vector<WireIo*>               _autoOutputWires;

        std::vector<Wire*>                 _userDecInputWires;
        std::vector<Wire*>                 _userOutputWires;

        /////// only top module will have this
        std::vector<WireIo*>               _globalInputs;
        std::vector<WireIo*>               _globalOutputs;

        explicit ModuleGen(Module* master);

        template<typename T>
        void createLogicGenBase(std::vector<T*>& srcs);

        template<typename T>
        void recruitLogicGenBase(LogicGenBaseVec& des,
                                 std::vector<T*>& srcs
        );

        template<typename T>
        void createAndRecruitLogicGenBase(LogicGenBaseVec& des,
                                 std::vector<T*>& srcs);
        ///////////// main progress

        void startInitEle  ();
        void startRouteEle ();
        void startCmpModule(ModuleGen* rhsMdg);
        void startWriteFile(FileWriterBase* fileWriter);


        ////////////// io operation
        WireIo* addAutoWireBase  (Operable* opr,
                                    Operable* realSrc,
                                    std::vector<WireIo*>& ioVec,
                                    std::unordered_map<Operable*, int>& ioMap,
                                    const std::string& wireName,
                                    WIRE_IO_TYPE wireIoType,
                                    bool connectTheWire = true);

        //////////////////////////////////////////////////////////////////////
        WireIo* addAutoInputWire          (Operable* opr, Operable* realSrc);
        bool    checkIsThereAutoInputWire (Operable* realSrc);
        WireIo* getAutoInputWire          (Operable* realSrc);
        //////////////////////////////////////////////////////////////////////
        WireIo* addAutoOutputWire         (Operable* opr, Operable* realSrc);
        bool    checkIsThereAutoOutputWire(Operable* realSrc);
        WireIo* getAutoOutputWire         (Operable* realSrc);
        //////////////////////////////////////////////////////////////////
        WireIo* addAutoInterWire          (Operable* realSrc);
        bool    checkIsThereAutoInterWire (Operable* realSrc);
        WireIo* getAutoInterWire          (Operable* realSrc);
        //////////////////////////////////////////////////////////////////
        Operable* routeSrcOprToThisModule (Operable* exactRealSrc);
        int       getDept() const{return depthFromGlobalModule;}
        //////////////////////////////////////////////////////////////////
        /////////////// module genFileName
        std::vector<std::string> getIoDec(
            const LogicGenBaseVec& inputVec,
            const LogicGenBaseVec& outputVec,
            const LogicGenBaseVec& globInputVec,
            const LogicGenBaseVec& globOutputVec
        );
        LogicGenBaseVec recruiteGlobIoIfTop(bool isInput);

        std::string getSubModuleDec(ModuleGen* mdGen);
        std::string getOpr();
    };

    class ModuleGenInterface{
    public:
        virtual ~ModuleGenInterface() = default;
        virtual void       createModuleGen() = 0;
        virtual ModuleGen* getModuleGen() = 0;

    };

}

#endif //MODULEGEN_H