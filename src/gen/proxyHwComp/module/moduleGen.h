//
// Created by tanawin on 20/6/2024.
//

#ifndef MODULEGEN_H
#define MODULEGEN_H
#include "unordered_map";
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
        Module* _master = nullptr;
        int depthFromGlobalModule = 0;
    public:
        std::vector<Reg*>        _regPool;
        std::vector<Wire*>       _wirePool;
        std::vector<expression*> _exprPool;
        std::vector<Val*>        _valPool;

        std::vector<MemBlock*>          _memBlockPool;
        std::vector<MemBlockEleHolder*> _memBlockElePool;

        std::vector<Wire*>              _interWires;

        std::unordered_map<Operable*, int> _autoInputWireMap;
        std::vector<Wire*>              _autoInputWires;
        std::unordered_map<Operable*, int> _autoOutputWireMap;
        std::vector<Wire*>              _autoOutputWires;

        std::vector<Wire*>              _userDecInputWires;
        std::vector<Wire*>              _userOutputWires;

        explicit ModuleGen(Module* master);

        void startInitEle();

        ////////////// io operation
        Operable* addAutoInputWire (Operable* opr);
        Operable* addAutoOutputWire(Operable* opr);

        bool checkIsThereAutoInputWire();
        bool chceckIsThereAutoOutputWire();

        Operable* routeSrcOprToThisModule(Operable* srcOpr);

        //////////// compareModule
        bool isIdent(ModuleGen* other);

        /////////// genFile
        void startWriteFile(FileWriterBase* fileWriter);


    };



}

#endif //MODULEGEN_H
