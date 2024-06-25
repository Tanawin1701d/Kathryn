//
// Created by tanawin on 20/6/2024.
//

#ifndef MODULEGEN_H
#define MODULEGEN_H
#include "unordered_map";
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
        std::vector<LogicGenBase*>   _regPool;
        std::vector<LogicGenBase*>   _wirePool;
        std::vector<LogicGenBase*>   _exprPool;
        std::vector<LogicGenBase*>   _nestPool;
        std::vector<LogicGenBase*>   _valPool;
        std::vector<LogicGenBase*>   _memBlockPool;
        std::vector<LogicGenBase*>   _memBlockElePool;

        std::vector<ModuleGen*>      _subModulePool;

        std::unordered_map<Operable*, int> _interWireMap;
        std::vector<WireIo*>                _interWires;

        std::unordered_map<Operable*, int> _autoInputWireMap;
        std::vector<WireIo*>               _autoInputWires;

        std::unordered_map<Operable*, int> _autoOutputWireMap;
        std::vector<WireIo*>               _autoOutputWires;

        std::vector<Wire*>                 _userDecInputWires;
        std::vector<Wire*>                 _userOutputWires;

        explicit ModuleGen(Module* master);

        template<typename T>
        void createLogicGenBase(std::vector<T*>& srcs);

        template<typename T>
        void recruitLogicGenBase(std::vector<LogicGenBase*>& des,
                                 std::vector<T*>& srcs
        );

        template<typename T>
        void createAndRecruitLogicGenBase(std::vector<LogicGenBase*>& des,
                                 std::vector<T*>& srcs);

        void doOpLogicGenVec(std::vector<LogicGenBase*>& src,
                             void (LogicGenBase::*func)());
        void
        doOpLogicGenVec(std::vector<std::string>&   result,
                        std::vector<LogicGenBase*>& src,
                        std::string (LogicGenBase::*func)());

        void doOpLogicGenAndWrite(std::vector<LogicGenBase*>& src,
                                 std::string (LogicGenBase::*func)(),
                                 FileWriterBase* fileWriter,
                                 const std::string& seperator = ""
        );


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