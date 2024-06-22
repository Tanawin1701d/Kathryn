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

        std::vector<ModuleGen*>     _subModulePool;

        std::unordered_map<Operable*, int> _interWireMap;
        std::vector<Wire*>              _interWires;

        std::unordered_map<Operable*, int> _autoInputWireMap;
        std::vector<Wire*>                 _autoInputWires;
        std::unordered_map<Operable*, int> _autoOutputWireMap;
        std::vector<Wire*>                 _autoOutputWires;

        std::vector<Wire*>              _userDecInputWires;
        std::vector<Wire*>              _userOutputWires;

        explicit ModuleGen(Module* master);

        template<typename T>
        void recruitLogicGenBase(std::vector<LogicGenBase*>& des,
                                 std::vector<T*>& srcs
        );
        void doOpLogicGenVec(std::vector<LogicGenBase*>& des,
                             void (LogicGenBase::*func)());
        // std::vector<std::string>
        //     doOpLogicGenVec(std::vector<LogicGenBase*>& src,
        //                     std::string (LogicGenBase::*func)());
        //
        // void writeThisVector(std::vector<std::string>& writeData,
        //                      FileWriterBase* fileWriter);

        void doOpLogicGenAndWrite(std::vector<LogicGenBase*>& src,
                                 std::string (LogicGenBase::*func)(),
                                 FileWriterBase* fileWriter
        );


        ///////////// main progress

        void startInitEle  ();
        void startRouteEle ();
        void startCmpModule(ModuleGen* rhsMdg);
        void startWriteFile(FileWriterBase* fileWriter);

        ////////////// io operation
        Wire* addAutoWireBase  (Operable* opr,
                                    Operable* realSrc,
                                    std::vector<Wire*>& ioVec,
                                    std::unordered_map<Operable*, int>& ioMap,
                                    const std::string& wireName);

        //////////////////////////////////////////////////////////////////////
        Operable* addAutoInputWire (Operable* opr, Operable* realSrc);
        bool      checkIsThereAutoInputWire  (Operable* realSrc);
        Operable* getAutoInputWire(Operable* realSrc);
        //////////////////////////////////////////////////////////////////////
        Operable* addAutoOutputWire(Operable* opr, Operable* realSrc);
        bool      checkIsThereAutoOutputWire(Operable* realSrc);
        Operable* getAutoOutputWire(Operable* realSrc);
        //////////////////////////////////////////////////////////////////
        Wire* addAutoIterWire(Operable* realSrc);
        bool  checkIsThereAutoInterWire(Operable* realSrc);
        Wire* getAutoInterWire(Operable* realSrc);
        //////////////////////////////////////////////////////////////////
        Operable* routeSrcOprToThisModule(Operable* realSrc);
        int getDept() const{return depthFromGlobalModule;}
        //////////////////////////////////////////////////////////////////
        /////////////// module genFileName
        std::string getSubModuleDec(ModuleGen* mdGen);
    };

    class ModuleGenInterface{
    public:
        virtual ~ModuleGenInterface() = default;
        virtual ModuleGen* getModuleGen() = 0;

    };



}

#endif //MODULEGEN_H
